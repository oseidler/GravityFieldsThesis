#include "Game/Planetoids.hpp"
#include "Game/Player.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Model.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "ThirdParty/Squirrel/SmoothNoise.hpp"


//
//generic planetoid functions
//
void Planetoid::DebugRender() const
{
	if (m_field != nullptr)
	{
		m_field->DebugRender();
	}
}


Mat44 Planetoid::GetModelMatrix() const
{
	Mat44 modelMatrix = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();

	modelMatrix.SetTranslation3D(m_position);

	return modelMatrix;
}


//
//plane planetoid functions
//
PlanePLTD::PlanePLTD(Vec3 position, float halfLength, float halfWidth, EulerAngles orientation, bool includeField, float gravityHeight, float gravityForce, Rgba8 color)
	: Planetoid(position, orientation, color)
	, m_halfLength(halfLength)
	, m_halfWidth(halfWidth)
{
	if(includeField) m_field = new PlaneField(this, halfLength, halfWidth, gravityHeight, gravityForce);

	AddVertsForQuad3D(m_verts, Vec3(-m_halfLength, m_halfWidth, 0.0f), Vec3(-m_halfLength, -m_halfWidth, 0.0f), Vec3(m_halfLength, m_halfWidth, 0.0f), Vec3(m_halfLength, -m_halfWidth, 0.0f));
}


void PlanePLTD::Render() const
{
	g_theRenderer->BindShader(g_theGame->m_lightingShader);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(GetModelMatrix(), m_color);
	g_theRenderer->DrawVertexArray(m_verts);
}


void PlanePLTD::RenderPreview(Vec3 position, float halfLength, float halfWidth, EulerAngles orientation, float gravityHeight, Rgba8 color)
{
	std::vector<Vertex_PCU> verts;

	AddVertsForQuad3D(verts, Vec3(-halfLength, halfWidth, 0.0f), Vec3(-halfLength, -halfWidth, 0.0f), Vec3(halfLength, halfWidth, 0.0f), Vec3(halfLength, -halfWidth, 0.0f));
	AddVertsForQuad3D(verts, Vec3(-halfLength, -halfWidth, 0.0f), Vec3(-halfLength, halfWidth, 0.0f), Vec3(halfLength, -halfWidth, 0.0f), Vec3(halfLength, halfWidth, 0.0f));
	Mat44 modelMatrix = orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	modelMatrix.SetTranslation3D(position);
	Rgba8 previewColor = Rgba8(color.r, color.g, color.b, 127);

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(modelMatrix, previewColor);
	g_theRenderer->DrawVertexArray(verts);

	std::vector<Vertex_PCU> gravVerts;

	Vec3 fbl = Vec3(halfLength, halfWidth, 0.0f);
	Vec3 fbr = Vec3(halfLength, -halfWidth, 0.0f);
	Vec3 bbl = Vec3(-halfLength, halfWidth, 0.0f);
	Vec3 bbr = Vec3(-halfLength, -halfWidth, 0.0f);
	Vec3 ftl = Vec3(halfLength, halfWidth, gravityHeight);
	Vec3 ftr = Vec3(halfLength, -halfWidth, gravityHeight);
	Vec3 btl = Vec3(-halfLength, halfWidth, gravityHeight);
	Vec3 btr = Vec3(-halfLength, -halfWidth, gravityHeight);

	AddVertsForCube3D(gravVerts, fbl, fbr, ftl, ftr, bbl, bbr, btl, btr);

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(modelMatrix, g_previewGravFieldColor);
	g_theRenderer->DrawVertexArray(gravVerts);
}


bool PlanePLTD::CollideWithPlayer(Player* player)
{
	Vec3 playerInPltdSpace = GetModelMatrix().GetOrthonormalInverse().TransformPosition3D(player->m_position);

	playerInPltdSpace.z = 0.0f;
	playerInPltdSpace.x = GetClamped(playerInPltdSpace.x, -m_halfLength, m_halfLength);
	playerInPltdSpace.y = GetClamped(playerInPltdSpace.y, -m_halfWidth, m_halfWidth);

	Vec3 nearestPointOnPlane = GetModelMatrix().TransformPosition3D(playerInPltdSpace);

	bool pushed = PushSphereOutOfFixedPoint3D(player->m_position, player->m_collisionRadius, nearestPointOnPlane);

	if (pushed)
	{
		Vec3 pushDirection = nearestPointOnPlane - player->m_position;
		pushDirection.Normalize();
		//if surface is ground, player becomes grounded
		if (DotProduct3D(pushDirection, -player->m_orientation.GetKBasis3D()) > GROUNDED_THRESHOLD)
		{
			player->BecomeGrounded();
		}
		//if surface is wall, player begins wall slide
		else if (DotProduct3D(pushDirection, player->m_orientation.GetIBasis3D()) > WALL_THRESHOLD && !player->m_isGrounded)
		{
			Mat44 orientationMat = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
			player->StartWallSlide(orientationMat.GetKBasis3D());
		}
	}

	return pushed;
}


Vec3 PlanePLTD::GetNearestPointOnPlanetoid(Vec3 playerPos) const
{
	//INCOMPLETE
	return Vec3();
}


//
//sphere planetoid functions
//
SpherePLTD::SpherePLTD(Vec3 position, float radius, bool includeField, float gravityRadius, float gravityForce, Rgba8 color)
	: Planetoid(position, EulerAngles(), color)
	, m_radius(radius)
{
	if(includeField) m_field = new SphereField(this, gravityRadius, gravityForce);

	AddVertsForSphere3D(m_verts, Vec3(), m_radius, 64, 32);
}


void SpherePLTD::Render() const
{
	g_theRenderer->BindShader(g_theGame->m_lightingShader);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(GetModelMatrix(), m_color);
	g_theRenderer->DrawVertexArray(m_verts);
}


void SpherePLTD::RenderPreview(Vec3 position, float radius, float gravityRadius, Rgba8 color)
{
	std::vector<Vertex_PCU> verts;

	AddVertsForSphere3D(verts, Vec3(), radius, 32, 16);
	Mat44 modelMatrix = Mat44::CreateTranslation3D(position);
	Rgba8 previewColor = Rgba8(color.r, color.g, color.b, 127);

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(modelMatrix, previewColor);
	g_theRenderer->DrawVertexArray(verts);

	std::vector<Vertex_PCU> gravVerts;

	AddVertsForSphere3D(gravVerts, Vec3(), gravityRadius, 32, 16);

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(modelMatrix, g_previewGravFieldColor);
	g_theRenderer->DrawVertexArray(gravVerts);
}


bool SpherePLTD::CollideWithPlayer(Player* player)
{
	bool pushed = PushSphereOutOfFixedSphere3D(player->m_position, player->m_collisionRadius, m_position, m_radius);

	if (pushed)
	{
		Vec3 nearestPoint = GetNearestPointOnSphere3D(player->m_position, m_position, m_radius);
		Vec3 pushDirection = nearestPoint - player->m_position;
		pushDirection.Normalize();
		if (DotProduct3D(pushDirection, -player->m_orientation.GetKBasis3D()) > GROUNDED_THRESHOLD)
		{
			player->BecomeGrounded();
		}
	}
	
	return pushed;
}


Vec3 SpherePLTD::GetNearestPointOnPlanetoid(Vec3 playerPos) const
{
	return GetNearestPointOnSphere3D(playerPos, m_position, m_radius);
}


//
//capsule planetoid functions
//
CapsulePLTD::CapsulePLTD(Vec3 position, float radius, float boneLength, Vec3 boneDirection, bool includeField, float gravityRadius, float gravityForce, Rgba8 color)
	: Planetoid(position, EulerAngles(), color)
	, m_radius(radius)
	, m_boneLength(boneLength)
	, m_boneDirection(boneDirection)
{
	m_boneDirection.Normalize();
	m_boneEnd = m_position + (m_boneDirection * m_boneLength);
	if(includeField) m_field = new CapsuleField(this, gravityRadius, position, m_boneEnd, gravityForce);

	AddVertsForCapsule3D(m_verts, Vec3(), m_boneEnd - m_position, m_radius, 32, 16);
}


void CapsulePLTD::Render() const
{
	g_theRenderer->BindShader(g_theGame->m_lightingShader);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(GetModelMatrix(), m_color);
	g_theRenderer->DrawVertexArray(m_verts);
}


void CapsulePLTD::RenderPreview(Vec3 position, float radius, float boneLength, Vec3 boneDirection, float gravityRadius, Rgba8 color)
{
	std::vector<Vertex_PCU> verts;

	boneDirection.Normalize();
	Vec3 boneEnd = position + (boneDirection * boneLength);
	AddVertsForCapsule3D(verts, Vec3(), boneEnd - position, radius, 32, 16);
	Mat44 modelMatrix = Mat44::CreateTranslation3D(position);
	Rgba8 previewColor = Rgba8(color.r, color.g, color.b, 127);

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(modelMatrix, previewColor);
	g_theRenderer->DrawVertexArray(verts);

	std::vector<Vertex_PCU> gravVerts;

	AddVertsForCapsule3D(gravVerts, Vec3(), boneEnd - position, gravityRadius, 32, 16);

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(modelMatrix, g_previewGravFieldColor);
	g_theRenderer->DrawVertexArray(gravVerts);
}


bool CapsulePLTD::CollideWithPlayer(Player* player)
{
	bool pushed = PushSphereOutOfFixedCapsule3D(player->m_position, player->m_collisionRadius, m_position, m_boneEnd, m_radius);
	
	if (pushed)
	{
		Vec3 nearestPoint = GetNearestPointOnCapsule3D(player->m_position, m_position, m_boneEnd, m_radius);
		Vec3 pushDirection = nearestPoint - player->m_position;
		pushDirection.Normalize();
		if (DotProduct3D(pushDirection, -player->m_orientation.GetKBasis3D()) > GROUNDED_THRESHOLD)
		{
			player->BecomeGrounded();
		}
	}

	return pushed;
}


Vec3 CapsulePLTD::GetNearestPointOnPlanetoid(Vec3 playerPos) const
{
	return GetNearestPointOnCapsule3D(playerPos, m_position, m_boneEnd, m_radius);
}


//
//ellipsoid planetoid functions
//
EllipsoidPLTD::EllipsoidPLTD(Vec3 position, float xRadius, float yRadius, float zRadius, EulerAngles orientation, bool includeField, float gravityXRadius, float gravityYRadius, float gravityZRadius,
	float gravityForce, Rgba8 color)
	: Planetoid(position, orientation, color)
	, m_xRadius(xRadius)
	, m_yRadius(yRadius)
	, m_zRadius(zRadius)
{
	if(includeField) m_field = new EllipsoidField(this, gravityXRadius, gravityYRadius, gravityZRadius, gravityForce);

	AddVertsForEllipsoid3D(m_verts, Vec3(), m_xRadius, m_yRadius, m_zRadius, 32, 16);
}


void EllipsoidPLTD::Render() const
{
	g_theRenderer->BindShader(g_theGame->m_lightingShader);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(GetModelMatrix(), m_color);
	g_theRenderer->DrawVertexArray(m_verts);
}


void EllipsoidPLTD::RenderPreview(Vec3 position, float xRadius, float yRadius, float zRadius, EulerAngles orientation, float gravityXRadius, float gravityYRadius, float gravityZRadius, Rgba8 color)
{
	std::vector<Vertex_PCU> verts;

	AddVertsForEllipsoid3D(verts, Vec3(), xRadius, yRadius, zRadius, 32, 16);
	Mat44 modelMatrix = orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	modelMatrix.SetTranslation3D(position);
	Rgba8 previewColor = Rgba8(color.r, color.g, color.b, 127);

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(modelMatrix, previewColor);
	g_theRenderer->DrawVertexArray(verts);

	std::vector<Vertex_PCU> gravVerts;

	AddVertsForEllipsoid3D(gravVerts, Vec3(), gravityXRadius, gravityYRadius, gravityZRadius, 32, 16);

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(modelMatrix, g_previewGravFieldColor);
	g_theRenderer->DrawVertexArray(gravVerts);
}


bool EllipsoidPLTD::CollideWithPlayer(Player* player)
{
	bool pushed = PushSphereOutOfFixedEllipsoid3D(player->m_position, player->m_collisionRadius, m_position, m_xRadius, m_yRadius, m_zRadius, m_orientation);

	if (pushed)
	{
		Vec3 nearestPoint = GetNearestPointOnEllipsoid3D(player->m_position, m_position, m_xRadius, m_yRadius, m_zRadius, m_orientation);
		Vec3 pushDirection = nearestPoint - player->m_position;
		pushDirection.Normalize();
		if (DotProduct3D(pushDirection, -player->m_orientation.GetKBasis3D()) > GROUNDED_THRESHOLD)
		{
			player->BecomeGrounded();
		}
	}

	return pushed;
}


Vec3 EllipsoidPLTD::GetNearestPointOnPlanetoid(Vec3 playerPos) const
{
	return GetNearestPointOnEllipsoid3D(playerPos, m_position, m_xRadius, m_yRadius, m_zRadius, m_orientation);
}


//
//rounded cube planetoid functions
//
RoundCubePLTD::RoundCubePLTD(Vec3 position, float length, float width, float height, float roundedness, EulerAngles orientation, bool includeField, float gravityLength, float gravityWidth, float gravityHeight, float gravityForce, Rgba8 color)
	: Planetoid(position, orientation, color)
	, m_length(length)
	, m_width(width)
	, m_height(height)
	, m_roundedness(roundedness)
{
	if(includeField) m_field = new RoundCubeField(this, gravityLength, gravityWidth, gravityHeight, gravityForce);

	AddVertsForRoundedCube3D(m_verts, Vec3(), m_length * 0.5f, m_width * 0.5f, m_height * 0.5f, m_roundedness);
}


void RoundCubePLTD::Render() const
{
	g_theRenderer->BindShader(g_theGame->m_lightingShader);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(GetModelMatrix(), m_color);
	g_theRenderer->DrawVertexArray(m_verts);
}


void RoundCubePLTD::RenderPreview(Vec3 position, float length, float width, float height, float roundedness, EulerAngles orientation, float gravityLength, float gravityWidth, float gravityHeight, Rgba8 color)
{
	std::vector<Vertex_PCU> verts;

	AddVertsForRoundedCube3D(verts, Vec3(), length * 0.5f, width * 0.5f, height * 0.5f, roundedness);
	Mat44 modelMatrix = orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	modelMatrix.SetTranslation3D(position);
	Rgba8 previewColor = Rgba8(color.r, color.g, color.b, 127);

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(modelMatrix, previewColor);
	g_theRenderer->DrawVertexArray(verts);

	std::vector<Vertex_PCU> gravVerts;

	AddVertsForRoundedCube3D(gravVerts, Vec3(),gravityLength * 0.5f, gravityWidth * 0.5f, gravityHeight * 0.5f, roundedness);

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(modelMatrix, g_previewGravFieldColor);
	g_theRenderer->DrawVertexArray(gravVerts);
}


bool RoundCubePLTD::CollideWithPlayer(Player* player)
{
	bool pushed = PushSphereOutOfFixedRoundedCube3D(player->m_position, player->m_collisionRadius, m_position, m_length, m_width, m_height, m_roundedness, m_orientation);

	if (pushed)
	{
		Vec3 nearestPoint = GetNearestPointOnRoundedCube3D(player->m_position, m_position, m_length, m_width, m_height, m_roundedness, m_orientation);
		Vec3 pushDirection = nearestPoint - player->m_position;
		pushDirection.Normalize();
		//if surface is ground, player becomes grounded
		if (DotProduct3D(pushDirection, -player->m_orientation.GetKBasis3D()) > GROUNDED_THRESHOLD)
		{
			player->BecomeGrounded();
		}
		//if surface is wall, player begins wall slide
		else if (DotProduct3D(pushDirection, player->m_orientation.GetIBasis3D()) > WALL_THRESHOLD && !player->m_isGrounded)
		{
			player->StartWallSlide(-pushDirection.GetNormalized());
		}
	}

	return pushed;
}


Vec3 RoundCubePLTD::GetNearestPointOnPlanetoid(Vec3 playerPos) const
{
	return GetNearestPointOnRoundedCube3D(playerPos, m_position, m_length, m_width, m_height, m_roundedness, m_orientation);
}


//
//torus planetoid functions
//
TorusPLTD::TorusPLTD(Vec3 position, float tubeRadius, float holeRadius, EulerAngles orientation, bool includeField, float gravityRadius, float gravityForce, Rgba8 color)
	: Planetoid(position, orientation, color)
	, m_tubeRadius(tubeRadius)
	, m_holeRadius(holeRadius)
{
	if(includeField) m_field = new TorusField(this, m_tubeRadius + gravityRadius, m_holeRadius - gravityRadius, gravityForce);

	AddVertsForTorus3D(m_verts, Vec3(), m_tubeRadius, m_holeRadius, 16, 32);
}


void TorusPLTD::Render() const
{
	g_theRenderer->BindShader(g_theGame->m_lightingShader);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(GetModelMatrix(), m_color);
	g_theRenderer->DrawVertexArray(m_verts);
}


void TorusPLTD::RenderPreview(Vec3 position, float tubeRadius, float holeRadius, EulerAngles orientation, float gravityRadius, Rgba8 color)
{
	std::vector<Vertex_PCU> verts;

	AddVertsForTorus3D(verts, Vec3(), tubeRadius, holeRadius);
	Mat44 modelMatrix = orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	modelMatrix.SetTranslation3D(position);
	Rgba8 previewColor = Rgba8(color.r, color.g, color.b, 127);

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(modelMatrix, previewColor);
	g_theRenderer->DrawVertexArray(verts);

	std::vector<Vertex_PCU> gravVerts;

	AddVertsForTorus3D(gravVerts, Vec3(), gravityRadius + tubeRadius, holeRadius - gravityRadius);

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(modelMatrix, g_previewGravFieldColor);
	g_theRenderer->DrawVertexArray(gravVerts);
}


bool TorusPLTD::CollideWithPlayer(Player* player)
{
	bool pushed = PushSphereOutOfFixedTorus3D(player->m_position, player->m_collisionRadius, m_position, m_tubeRadius, m_holeRadius, m_orientation);

	if (pushed)
	{
		Vec3 nearestPoint = GetNearestPointOnTorus3D(player->m_position, m_position, m_tubeRadius, m_holeRadius, m_orientation);
		Vec3 pushDirection = nearestPoint - player->m_position;
		pushDirection.Normalize();
		if (DotProduct3D(pushDirection, -player->m_orientation.GetKBasis3D()) > GROUNDED_THRESHOLD)
		{
			player->BecomeGrounded();
		}
	}

	return pushed;
}


Vec3 TorusPLTD::GetNearestPointOnPlanetoid(Vec3 playerPos) const
{
	return GetNearestPointOnTorus3D(playerPos, m_position, m_tubeRadius, m_holeRadius, m_orientation);
}


//
//bowl planetoid functions
//
BowlPLTD::BowlPLTD(Vec3 position, float radius, float thickness, EulerAngles orientation, bool includeField, float gravityRadius, float gravityForce, Rgba8 color)
	: Planetoid(position, orientation, color)
	, m_radius(radius)
	, m_thickness(thickness)
{
	if(includeField) m_field = new BowlField(this, radius + gravityRadius, gravityRadius, radius - thickness, gravityForce);

	AddVertsForBowl();
}


void BowlPLTD::Render() const
{
	g_theRenderer->BindShader(g_theGame->m_lightingShader);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(GetModelMatrix(), m_color);
	g_theRenderer->DrawVertexArray(m_verts);
}


void BowlPLTD::RenderPreview(Vec3 position, float radius, float thickness, EulerAngles orientation, float gravityRadius, Rgba8 color)
{
	std::vector<Vertex_PCU> verts;

	AddVertsForBowl(verts, radius, thickness);
	Mat44 modelMatrix = orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	modelMatrix.SetTranslation3D(position);
	Rgba8 previewColor = Rgba8(color.r, color.g, color.b, 127);

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(modelMatrix, previewColor);
	g_theRenderer->DrawVertexArray(verts);

	std::vector<Vertex_PCU> gravVerts;

	float degreesPerSlice = 360.0f / static_cast<float>(32);
	float degreesPerStack = 180.0f / static_cast<float>(8);

	for (int stackIndex = 0; stackIndex < 8; stackIndex++)
	{
		float topDegreesLat = -90.0f + (static_cast<float>(stackIndex) * degreesPerStack * 0.5f);
		float bottomDegreesLat = -90.0f + (static_cast<float>(stackIndex + 1) * degreesPerStack * 0.5f);

		for (int sliceIndex = 0; sliceIndex < 32; sliceIndex++)
		{
			float leftDegreesLong = static_cast<float>(sliceIndex) * degreesPerSlice;
			float rightDegreesLong = static_cast<float>(sliceIndex + 1) * degreesPerSlice;

			Vec3 bottomLeftCoords = Vec3::MakeFromPolarDegrees(bottomDegreesLat, leftDegreesLong, radius + gravityRadius);
			Vec3 bottomRightCoords = Vec3::MakeFromPolarDegrees(bottomDegreesLat, rightDegreesLong, radius + gravityRadius);
			Vec3 topLeftCoords = Vec3::MakeFromPolarDegrees(topDegreesLat, leftDegreesLong, radius + gravityRadius);
			Vec3 topRightCoords = Vec3::MakeFromPolarDegrees(topDegreesLat, rightDegreesLong, radius + gravityRadius);

			gravVerts.emplace_back(Vertex_PCU(bottomLeftCoords, Rgba8(), Vec2()));
			gravVerts.emplace_back(Vertex_PCU(topRightCoords, Rgba8(), Vec2()));
			gravVerts.emplace_back(Vertex_PCU(bottomRightCoords, Rgba8(), Vec2()));
			
			gravVerts.emplace_back(Vertex_PCU(bottomLeftCoords, Rgba8(), Vec2()));
			gravVerts.emplace_back(Vertex_PCU(topLeftCoords, Rgba8(), Vec2()));
			gravVerts.emplace_back(Vertex_PCU(topRightCoords, Rgba8(), Vec2()));
		}
	}

	Vec3 topCenter = Vec3(0.0f, 0.0f, gravityRadius);

	for (int edgeIndex = 0; edgeIndex < 32; edgeIndex++)
	{
		float startDegrees = static_cast<float>(edgeIndex) * degreesPerSlice;
		float endDegrees = static_cast<float>(edgeIndex + 1) * degreesPerSlice;

		Vec3 baseEdgeStart = Vec3::MakeFromPolarDegrees(0.0f, startDegrees, radius + gravityRadius);
		Vec3 baseEdgeEnd = Vec3::MakeFromPolarDegrees(0.0f, endDegrees, radius + gravityRadius);

		Vec3 topEdgeStart = topCenter + Vec3::MakeFromPolarDegrees(0.0f, startDegrees, radius + gravityRadius);
		Vec3 topEdgeEnd = topCenter + Vec3::MakeFromPolarDegrees(0.0f, endDegrees, radius + gravityRadius);

		//draw triangle at top
		gravVerts.push_back(Vertex_PCU(topCenter, Rgba8(), Vec2()));
		gravVerts.push_back(Vertex_PCU(topEdgeStart, Rgba8(), Vec2()));
		gravVerts.push_back(Vertex_PCU(topEdgeEnd, Rgba8(), Vec2()));

		//draw side quad
		AddVertsForQuad3D(gravVerts, baseEdgeStart, baseEdgeEnd, topEdgeStart, topEdgeEnd);
	}

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(modelMatrix, g_previewGravFieldColor);
	g_theRenderer->DrawVertexArray(gravVerts);
}


bool BowlPLTD::CollideWithPlayer(Player* player)
{
	bool pushed = PushSphereOutOfPlanetoid(player->m_position, player->m_collisionRadius);

	if (pushed)
	{
		Vec3 nearestPoint = GetNearestPointOnPlanetoid(player->m_position);
		Vec3 pushDirection = nearestPoint - player->m_position;
		pushDirection.Normalize();
		//if surface is ground, player becomes grounded
		if (DotProduct3D(pushDirection, -player->m_orientation.GetKBasis3D()) > GROUNDED_THRESHOLD)
		{
			player->BecomeGrounded();
		}
		//if surface is wall, player begins wall slide
		else if (DotProduct3D(pushDirection, player->m_orientation.GetIBasis3D()) > WALL_THRESHOLD && !player->m_isGrounded)
		{
			player->StartWallSlide(-pushDirection.GetNormalized());
		}
	}
	
	return pushed;
}


Vec3 BowlPLTD::GetNearestPointOnPlanetoid(Vec3 playerPos) const
{
	Mat44 bowlModelMat = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	bowlModelMat.SetTranslation3D(m_position);
	Mat44 bowlWorldToLocalMat = bowlModelMat.GetOrthonormalInverse();
	Vec3 playerPosInLocalSpace = bowlWorldToLocalMat.TransformPosition3D(playerPos);
	
	Vec3 nearestPointOnOutside = GetNearestPointOnSphereEdge3D(playerPosInLocalSpace, Vec3(), m_radius);
	if (nearestPointOnOutside.z > 0.0f)
	{
		Vec2 nearestPointOnOutside2D = GetNearestPointOnDiscEdge2D(Vec2(playerPosInLocalSpace.x, playerPosInLocalSpace.y), Vec2(), m_radius);
		nearestPointOnOutside = Vec3(nearestPointOnOutside2D.x, nearestPointOnOutside2D.y, 0.0f);
	}
	Vec3 nearestPointOnInside = GetNearestPointOnSphereEdge3D(playerPosInLocalSpace, Vec3(), m_radius - m_thickness);
	if (nearestPointOnInside.z > 0.0f)
	{
		Vec2 nearestPointOnInside2D = GetNearestPointOnDiscEdge2D(Vec2(playerPosInLocalSpace.x, playerPosInLocalSpace.y), Vec2(), m_radius - m_thickness);
		nearestPointOnInside = Vec3(nearestPointOnInside2D.x, nearestPointOnInside2D.y, 0.0f);
	}
	Vec2 nearestPointOnLip2D = GetNearestPointOnDisc2D(Vec2(playerPosInLocalSpace.x, playerPosInLocalSpace.y), Vec2(), m_radius);
	if (GetDistanceSquared2D(nearestPointOnLip2D, Vec2()) < (m_radius - m_thickness) * (m_radius - m_thickness))
	{
		nearestPointOnLip2D = GetNearestPointOnDiscEdge2D(Vec2(playerPosInLocalSpace.x, playerPosInLocalSpace.y), Vec2(), m_radius - m_thickness);
	}
	Vec3 nearestPointOnLip = Vec3(nearestPointOnLip2D.x, nearestPointOnLip2D.y, 0.0f);

	float outsideDistanceSquared = GetDistanceSquared3D(nearestPointOnOutside, playerPosInLocalSpace);
	float insideDistanceSquared = GetDistanceSquared3D(nearestPointOnInside, playerPosInLocalSpace);
	float lipDistanceSquared = GetDistanceSquared3D(nearestPointOnLip, playerPosInLocalSpace);

	Vec3 nearestPointInLocalSpace = Vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	if (outsideDistanceSquared <= insideDistanceSquared && outsideDistanceSquared <= lipDistanceSquared)
	{
		nearestPointInLocalSpace = nearestPointOnOutside;
	}
	else if (insideDistanceSquared <= outsideDistanceSquared && insideDistanceSquared <= lipDistanceSquared)
	{
		nearestPointInLocalSpace = nearestPointOnInside;
	}
	else //lipDistanceSquared <= outsideDistanceSquared && lipDistanceSquared <= insideDistanceSquared
	{
		nearestPointInLocalSpace = nearestPointOnLip;
	}

	Vec3 nearestPoint = bowlModelMat.TransformPosition3D(nearestPointInLocalSpace);

	return nearestPoint;
}


bool BowlPLTD::PushSphereOutOfPlanetoid(Vec3& sphereCenter, float sphereRadius)
{
	Vec3 nearestPoint = GetNearestPointOnPlanetoid(sphereCenter);

	return PushSphereOutOfFixedPoint3D(sphereCenter, sphereRadius, nearestPoint);
}


void BowlPLTD::AddVertsForBowl()
{
	//add exterior hemisphere
	float degreesPerSlice = 360.0f / 32.0f;
	float degreesPerStack = 90.0f / 8.0f;

	for (int stackIndex = 0; stackIndex < 8; stackIndex++)
	{
		float topDegreesLat = -90.0f + (static_cast<float>(stackIndex) * degreesPerStack);
		float bottomDegreesLat = topDegreesLat + degreesPerStack;

		for (int sliceIndex = 0; sliceIndex < 32; sliceIndex++)
		{
			float leftDegreesLong = static_cast<float>(sliceIndex) * degreesPerSlice;
			float rightDegreesLong = leftDegreesLong + degreesPerSlice;

			Vec3 bottomLeftNormal = Vec3::MakeFromPolarDegrees(bottomDegreesLat, leftDegreesLong);
			Vec3 bottomLeftCoords = bottomLeftNormal * m_radius;
			Vec3 bottomRightNormal = Vec3::MakeFromPolarDegrees(bottomDegreesLat, rightDegreesLong);
			Vec3 bottomRightCoords = bottomRightNormal * m_radius;
			Vec3 topLeftNormal = Vec3::MakeFromPolarDegrees(topDegreesLat, leftDegreesLong);
			Vec3 topLeftCoords = topLeftNormal * m_radius;
			Vec3 topRightNormal = Vec3::MakeFromPolarDegrees(topDegreesLat, rightDegreesLong);
			Vec3 topRightCoords = topRightNormal * m_radius;

			m_verts.push_back(Vertex_PCUTBN(bottomLeftCoords, bottomLeftNormal, Rgba8()));
			m_verts.push_back(Vertex_PCUTBN(topRightCoords, topRightNormal, Rgba8()));
			m_verts.push_back(Vertex_PCUTBN(bottomRightCoords, bottomRightNormal, Rgba8()));
			
			m_verts.push_back(Vertex_PCUTBN(bottomLeftCoords, bottomLeftNormal, Rgba8()));
			m_verts.push_back(Vertex_PCUTBN(topLeftCoords, topLeftNormal, Rgba8()));
			m_verts.push_back(Vertex_PCUTBN(topRightCoords, topRightNormal, Rgba8()));
		}
	}
	
	//add interior hemisphere
	for (int stackIndex = 0; stackIndex < 8; stackIndex++)
	{
		float topDegreesLat = -90.0f + (static_cast<float>(stackIndex) * degreesPerStack);
		float bottomDegreesLat = topDegreesLat + degreesPerStack;

		for (int sliceIndex = 0; sliceIndex < 32; sliceIndex++)
		{
			float leftDegreesLong = static_cast<float>(sliceIndex) * degreesPerSlice;
			float rightDegreesLong = leftDegreesLong + degreesPerSlice;

			Vec3 bottomLeftNormal = Vec3::MakeFromPolarDegrees(bottomDegreesLat, leftDegreesLong);
			Vec3 bottomLeftCoords = bottomLeftNormal * (m_radius - m_thickness);
			Vec3 bottomRightNormal = Vec3::MakeFromPolarDegrees(bottomDegreesLat, rightDegreesLong);
			Vec3 bottomRightCoords = bottomRightNormal * (m_radius - m_thickness);
			Vec3 topLeftNormal = Vec3::MakeFromPolarDegrees(topDegreesLat, leftDegreesLong);
			Vec3 topLeftCoords = topLeftNormal * (m_radius - m_thickness);
			Vec3 topRightNormal = Vec3::MakeFromPolarDegrees(topDegreesLat, rightDegreesLong);
			Vec3 topRightCoords = topRightNormal * (m_radius - m_thickness);
			
			m_verts.push_back(Vertex_PCUTBN(topRightCoords, topRightNormal, Rgba8()));
			m_verts.push_back(Vertex_PCUTBN(bottomLeftCoords, bottomLeftNormal, Rgba8()));
			m_verts.push_back(Vertex_PCUTBN(bottomRightCoords, bottomRightNormal, Rgba8()));
		
			m_verts.push_back(Vertex_PCUTBN(topLeftCoords, topLeftNormal, Rgba8()));
			m_verts.push_back(Vertex_PCUTBN(bottomLeftCoords, bottomLeftNormal, Rgba8()));
			m_verts.push_back(Vertex_PCUTBN(topRightCoords, topRightNormal, Rgba8()));
		}
	}

	//add connecting rim
	for (int sliceIndex = 0; sliceIndex < 32; sliceIndex++)
	{
		float leftDegrees = static_cast<float>(sliceIndex) * degreesPerSlice;
		float rightDegrees = leftDegrees + degreesPerSlice;
		
		Vec3 bottomLeft = Vec3::MakeFromPolarDegrees(0.0f, leftDegrees, m_radius);
		Vec3 bottomRight = Vec3::MakeFromPolarDegrees(0.0f, rightDegrees, m_radius);
		Vec3 topLeft = Vec3::MakeFromPolarDegrees(0.0f, leftDegrees, m_radius - m_thickness);
		Vec3 topRight = Vec3::MakeFromPolarDegrees(0.0f, rightDegrees, m_radius - m_thickness);
		
		AddVertsForQuad3D(m_verts, bottomLeft, bottomRight, topLeft, topRight);
	}
}


void BowlPLTD::AddVertsForBowl(std::vector<Vertex_PCU>& verts, float radius, float thickness)
{
	//add exterior hemisphere
	float degreesPerSlice = 360.0f / 32.0f;
	float degreesPerStack = 90.0f / 8.0f;

	for (int stackIndex = 0; stackIndex < 8; stackIndex++)
	{
		float topDegreesLat = -90.0f + (static_cast<float>(stackIndex) * degreesPerStack);
		float bottomDegreesLat = topDegreesLat + degreesPerStack;

		for (int sliceIndex = 0; sliceIndex < 32; sliceIndex++)
		{
			float leftDegreesLong = static_cast<float>(sliceIndex) * degreesPerSlice;
			float rightDegreesLong = leftDegreesLong + degreesPerSlice;

			Vec3 bottomLeftNormal = Vec3::MakeFromPolarDegrees(bottomDegreesLat, leftDegreesLong);
			Vec3 bottomLeftCoords = bottomLeftNormal * radius;
			Vec3 bottomRightNormal = Vec3::MakeFromPolarDegrees(bottomDegreesLat, rightDegreesLong);
			Vec3 bottomRightCoords = bottomRightNormal * radius;
			Vec3 topLeftNormal = Vec3::MakeFromPolarDegrees(topDegreesLat, leftDegreesLong);
			Vec3 topLeftCoords = topLeftNormal * radius;
			Vec3 topRightNormal = Vec3::MakeFromPolarDegrees(topDegreesLat, rightDegreesLong);
			Vec3 topRightCoords = topRightNormal * radius;

			verts.push_back(Vertex_PCU(bottomLeftCoords, Rgba8(), Vec2()));
			verts.push_back(Vertex_PCU(topRightCoords, Rgba8(), Vec2()));
			verts.push_back(Vertex_PCU(bottomRightCoords, Rgba8(), Vec2()));
			
			verts.push_back(Vertex_PCU(bottomLeftCoords, Rgba8(), Vec2()));
			verts.push_back(Vertex_PCU(topLeftCoords, Rgba8(), Vec2()));
			verts.push_back(Vertex_PCU(topRightCoords, Rgba8(), Vec2()));
		}
	}

	//add interior hemisphere
	for (int stackIndex = 0; stackIndex < 8; stackIndex++)
	{
		float topDegreesLat = -90.0f + (static_cast<float>(stackIndex) * degreesPerStack);
		float bottomDegreesLat = topDegreesLat + degreesPerStack;

		for (int sliceIndex = 0; sliceIndex < 32; sliceIndex++)
		{
			float leftDegreesLong = static_cast<float>(sliceIndex) * degreesPerSlice;
			float rightDegreesLong = leftDegreesLong + degreesPerSlice;

			Vec3 bottomLeftNormal = Vec3::MakeFromPolarDegrees(bottomDegreesLat, leftDegreesLong);
			Vec3 bottomLeftCoords = bottomLeftNormal * (radius - thickness);
			Vec3 bottomRightNormal = Vec3::MakeFromPolarDegrees(bottomDegreesLat, rightDegreesLong);
			Vec3 bottomRightCoords = bottomRightNormal * (radius - thickness);
			Vec3 topLeftNormal = Vec3::MakeFromPolarDegrees(topDegreesLat, leftDegreesLong);
			Vec3 topLeftCoords = topLeftNormal * (radius - thickness);
			Vec3 topRightNormal = Vec3::MakeFromPolarDegrees(topDegreesLat, rightDegreesLong);
			Vec3 topRightCoords = topRightNormal * (radius - thickness);

			verts.push_back(Vertex_PCU(topRightCoords, Rgba8(), Vec2()));
			verts.push_back(Vertex_PCU(bottomLeftCoords, Rgba8(), Vec2()));
			verts.push_back(Vertex_PCU(bottomRightCoords, Rgba8(), Vec2()));

			verts.push_back(Vertex_PCU(topLeftCoords, Rgba8(), Vec2()));
			verts.push_back(Vertex_PCU(bottomLeftCoords, Rgba8(), Vec2()));
			verts.push_back(Vertex_PCU(topRightCoords, Rgba8(), Vec2()));
		}
	}

	//add connecting rim
	for (int sliceIndex = 0; sliceIndex < 32; sliceIndex++)
	{
		float leftDegrees = static_cast<float>(sliceIndex) * degreesPerSlice;
		float rightDegrees = leftDegrees + degreesPerSlice;

		Vec3 bottomLeft = Vec3::MakeFromPolarDegrees(0.0f, leftDegrees, radius);
		Vec3 bottomRight = Vec3::MakeFromPolarDegrees(0.0f, rightDegrees, radius);
		Vec3 topLeft = Vec3::MakeFromPolarDegrees(0.0f, leftDegrees, radius - thickness);
		Vec3 topRight = Vec3::MakeFromPolarDegrees(0.0f, rightDegrees, radius - thickness);

		AddVertsForQuad3D(verts, bottomLeft, bottomRight, topLeft, topRight);
	}
}


//
//mobius strip planetoid functions
//
MobiusPLTD::MobiusPLTD(Vec3 position, float radius, float halfWidth, EulerAngles orientation, bool includeField, float gravityHeight, float gravityForce, Rgba8 color)
	: Planetoid(position, orientation, color)
	, m_radius(radius)
	, m_halfWidth(halfWidth)
{
	//add field
	UNUSED(includeField);
	UNUSED(gravityHeight);
	UNUSED(gravityForce);

	AddVertsForMobiusStrip3D(m_verts, Vec3(), m_radius, m_halfWidth, 256);
}


void MobiusPLTD::Render() const
{
	g_theRenderer->BindShader(g_theGame->m_lightingShader);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(GetModelMatrix(), m_color);
	g_theRenderer->DrawVertexArray(m_verts);
}


void MobiusPLTD::RenderPreview(Vec3 position, float radius, float halfWidth, EulerAngles orientation, float gravityHeight, Rgba8 color)
{
	std::vector<Vertex_PCU> verts;

	AddVertsForMobiusStrip3D(verts, Vec3(), radius, halfWidth, 256);
	Mat44 modelMatrix = orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	modelMatrix.SetTranslation3D(position);
	Rgba8 previewColor = Rgba8(color.r, color.g, color.b, 127);

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(modelMatrix, previewColor);
	g_theRenderer->DrawVertexArray(verts);

	std::vector<Vertex_PCU> gravVerts;

	UNUSED(gravityHeight);
}


bool MobiusPLTD::CollideWithPlayer(Player* player)
{
	Vec3 referencePointLocal = GetModelMatrix().GetOrthonormalInverse().TransformPosition3D(player->m_position);

	bool wasPlayerPushed = false;
	for (int vertIndex = 0; vertIndex < m_verts.size(); vertIndex += 3)
	{
		if (m_verts[vertIndex].m_position == m_verts[vertIndex + 1].m_position || m_verts[vertIndex].m_position == m_verts[vertIndex + 2].m_position || m_verts[vertIndex + 1].m_position 
			== m_verts[vertIndex + 2].m_position)
		{
			continue;
		}

		Vec3 triNearestLocalPoint = GetNearestPointOnTriangle3D(referencePointLocal, m_verts[vertIndex].m_position, m_verts[vertIndex + 1].m_position, m_verts[vertIndex + 2].m_position);

		if (triNearestLocalPoint.x != triNearestLocalPoint.x)
		{
			continue;
		}

		Vec3 triNearestPoint = GetModelMatrix().TransformPosition3D(triNearestLocalPoint);

		bool wasPlayerPushedByTri = PushSphereOutOfFixedPoint3D(player->m_position, player->m_collisionRadius, triNearestPoint);
		if (wasPlayerPushedByTri)
		{
			wasPlayerPushed = true;
			Vec3 pushDirection = triNearestPoint - player->m_position;
			pushDirection.Normalize();
			if (DotProduct3D(pushDirection, -player->m_orientation.GetKBasis3D()) > GROUNDED_THRESHOLD)
			{
				player->BecomeGrounded();
			}
		}
	}

	return wasPlayerPushed;
}


Vec3 MobiusPLTD::GetNearestPointOnPlanetoid(Vec3 playerPos) const
{
	//return GetNearestPointOnMobiusStrip3D(playerPos, m_position, m_radius, m_halfWidth, m_orientation);
	return Vec3(FLT_MAX, FLT_MAX, FLT_MAX);
}


//
//wire planetoid functions
//
WirePLTD::WirePLTD(Vec3 position, float radius, WirePerlinParameters perlinStruct, EulerAngles orientation, bool includeField, float gravityRadius, float gravityForce, Rgba8 color)
	: Planetoid(position, orientation, color)
	, m_radius(radius)
{
	WirePerlinParameters& ps = perlinStruct;
	
	//worm setup
	RandomNumberGenerator rng;
	rng.SeedRNG(ps.m_rngSeed);
	int numSegments = rng.RollRandomIntInRange(ps.m_minSegments, ps.m_maxSegments);
	Vec3 segmentStart = Vec3();
	m_wirePositions.emplace_back(segmentStart);
	float horizontalDirectionDegrees = 0.0f;

	//calculate each segment
	for (int segIndex = 0; segIndex < numSegments; segIndex++)
	{
		float horizontalDirectionChange = ps.m_maxYawChange * Compute3dPerlinNoise(segmentStart.x + m_position.x, segmentStart.y + m_position.y, segmentStart.z + m_position.z, 
			ps.m_perlinScaleYaw, ps.m_perlinOctavesYaw, ps.m_perlinOctavePersistYaw, ps.m_perlinOctaveScaleYaw, true, ps.m_rngSeed);
		//horizontalDirectionChange = RangeMap(horizontalDirectionChange, -1.0f, 1.0f, -ps.m_maxYawChange, ps.m_maxYawChange);
		horizontalDirectionDegrees += horizontalDirectionChange;		

		float verticalDirectionDegrees = ps.m_maxPitchChange * Compute3dPerlinNoise(segmentStart.x + m_position.x, segmentStart.y + m_position.y, segmentStart.z + m_position.z, 
			ps.m_perlinScalePitch, ps.m_perlinOctavesPitch, ps.m_perlinOctavePersistPitch, ps.m_perlinOctaveScalePitch, true, ps.m_rngSeed);

		float segmentLength = static_cast<float>(rng.RollRandomFloatInRange(ps.m_segMinLength, ps.m_segMaxLength));
		Vec3 segmentVector = Vec3::MakeFromPolarDegrees(verticalDirectionDegrees, horizontalDirectionDegrees, segmentLength);
		Vec3 segmentEnd = segmentStart + Vec3(segmentVector.x, segmentVector.y, segmentVector.z);

		m_wirePositions.emplace_back(segmentEnd);

		segmentStart = segmentEnd;
	}

	//create field
	if(includeField) m_field = new WireField(this, gravityRadius, gravityForce);

	AddVertsForWire(m_verts);
}


void WirePLTD::Render() const
{
	g_theRenderer->BindShader(g_theGame->m_lightingShader);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(GetModelMatrix(), m_color);
	g_theRenderer->DrawVertexArray(m_verts);
}


void WirePLTD::RenderPreview(Vec3 position, float radius, WirePerlinParameters perlinStruct, EulerAngles orientation, float gravityRadius, Rgba8 color)
{
	std::vector<Vec3> wirePositions;
	WirePerlinParameters& ps = perlinStruct;

	//worm setup
	RandomNumberGenerator rng;
	rng.SeedRNG(ps.m_rngSeed);
	int numSegments = rng.RollRandomIntInRange(ps.m_minSegments, ps.m_maxSegments);
	Vec3 segmentStart = Vec3();
	wirePositions.emplace_back(segmentStart);
	float horizontalDirectionDegrees = 0.0f;

	//calculate each segment
	for (int segIndex = 0; segIndex < numSegments; segIndex++)
	{
		float horizontalDirectionChange = ps.m_maxYawChange * Compute3dPerlinNoise(segmentStart.x + position.x, segmentStart.y + position.y, segmentStart.z + position.z,
			ps.m_perlinScaleYaw, ps.m_perlinOctavesYaw, ps.m_perlinOctavePersistYaw, ps.m_perlinOctaveScaleYaw, true, ps.m_rngSeed);
		//horizontalDirectionChange = RangeMap(horizontalDirectionChange, -1.0f, 1.0f, -ps.m_maxYawChange, ps.m_maxYawChange);
		horizontalDirectionDegrees += horizontalDirectionChange;

		float verticalDirectionDegrees = ps.m_maxPitchChange * Compute3dPerlinNoise(segmentStart.x + position.x, segmentStart.y + position.y, segmentStart.z + position.z,
			ps.m_perlinScalePitch, ps.m_perlinOctavesPitch, ps.m_perlinOctavePersistPitch, ps.m_perlinOctaveScalePitch, true, ps.m_rngSeed);

		float segmentLength = static_cast<float>(rng.RollRandomFloatInRange(ps.m_segMinLength, ps.m_segMaxLength));
		Vec3 segmentVector = Vec3::MakeFromPolarDegrees(verticalDirectionDegrees, horizontalDirectionDegrees, segmentLength);
		Vec3 segmentEnd = segmentStart + Vec3(segmentVector.x, segmentVector.y, segmentVector.z);

		wirePositions.emplace_back(segmentEnd);

		segmentStart = segmentEnd;
	}
	
	std::vector<Vertex_PCU> verts;

	AddVertsForWireStatic(verts, wirePositions, radius);
	Mat44 modelMatrix = orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	modelMatrix.SetTranslation3D(position);
	Rgba8 previewColor = Rgba8(color.r, color.g, color.b, 127);

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(modelMatrix, previewColor);
	g_theRenderer->DrawVertexArray(verts);

	std::vector<Vertex_PCU> gravVerts;

	AddVertsForWireStatic(gravVerts, wirePositions, gravityRadius);

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(modelMatrix, g_previewGravFieldColor);
	g_theRenderer->DrawVertexArray(gravVerts);
}


bool WirePLTD::CollideWithPlayer(Player* player)
{
	bool pushOut = false;
	
	for (int segIndex = 0; segIndex < m_wirePositions.size() - 1; segIndex++)
	{
		Vec3 wireStart = m_wirePositions[segIndex];
		wireStart = GetModelMatrix().TransformPosition3D(wireStart);
		Vec3 wireEnd = m_wirePositions[segIndex + 1];
		wireEnd = GetModelMatrix().TransformPosition3D(wireEnd);
		
		if (PushSphereOutOfFixedCapsule3D(player->m_position, player->m_collisionRadius, wireStart, wireEnd, m_radius))
		{
			pushOut = true;
			Vec3 nearestPoint = GetNearestPointOnCapsule3D(player->m_position, wireStart, wireEnd, m_radius);
			Vec3 pushDirection = nearestPoint - player->m_position;
			pushDirection.Normalize();
			if (DotProduct3D(pushDirection, -player->m_orientation.GetKBasis3D()) > GROUNDED_THRESHOLD)
			{
				player->BecomeGrounded();
			}
		}
	}

	return pushOut;
}


Vec3 WirePLTD::GetNearestPointOnPlanetoid(Vec3 playerPos) const
{
	//INCOMPLETE
	UNUSED(playerPos);
	return Vec3();
}


void WirePLTD::AddVertsForWire(std::vector<Vertex_PCUTBN>& verts) const
{
	for (int segIndex = 0; segIndex < m_wirePositions.size() - 1; segIndex++)
	{
		AddVertsForCapsule3D(verts, m_wirePositions[segIndex], m_wirePositions[segIndex + 1], m_radius);
	}
}


void WirePLTD::AddVertsForWireStatic(std::vector<Vertex_PCU>& verts, std::vector<Vec3> const& wirePositions, float radius)
{
	for (int segIndex = 0; segIndex < wirePositions.size() - 1; segIndex++)
	{
		AddVertsForCapsule3D(verts, wirePositions[segIndex], wirePositions[segIndex + 1], radius);
	}
}


//
//prefab planetoid functions
//
PrefabPLTD::~PrefabPLTD()
{
	if (m_model != nullptr)
	{
		delete m_model;
	}
}


void PrefabPLTD::Render() const
{
	if (m_model != nullptr)
	{
		m_model->RenderGPUMesh(g_theGame->m_sunDirection, g_theGame->m_sunIntensity, g_theGame->m_ambientIntensity);
	}
}


void PrefabPLTD::RenderPreview(Model* model, Vec3 position, EulerAngles orientation, Rgba8 color, bool includeField, int modelType, float gravScale)
{
	//#ToDo: Add gravity field to preview

	if (model != nullptr)
	{
		model->m_position = position;
		model->m_orientation = orientation;
		model->m_color = color;
		model->RenderGPUMesh(g_theGame->m_sunDirection, g_theGame->m_sunIntensity, g_theGame->m_ambientIntensity);

		if (includeField)
		{
			switch (modelType)
			{
				case 0:
				{
					std::vector<Vertex_PCU> gravVerts;

					AddVertsForSphere3D(gravVerts, Vec3(), gravScale, 32, 16);

					Mat44 modelMatrix = orientation.GetAsMatrix_XFwd_YLeft_ZUp();
					modelMatrix.SetTranslation3D(position + modelMatrix.TransformVectorQuantity3D(Vec3(0.0f, 0.0f, 6.5f)));

					g_theRenderer->BindShader(nullptr);
					g_theRenderer->BindTexture(nullptr);
					g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
					g_theRenderer->SetModelConstants(modelMatrix, g_previewGravFieldColor);
					g_theRenderer->DrawVertexArray(gravVerts);

					break;
				}
				case 1:
				{
					std::vector<Vertex_PCU> verts;

					Mat44 modelMatrix = orientation.GetAsMatrix_XFwd_YLeft_ZUp();
					modelMatrix.SetTranslation3D(position);
					Vec3 boneStart = Vec3(-12.5f, -25.5f, -18.0f);
					boneStart = modelMatrix.TransformPosition3D(boneStart);
					Vec3 boneEnd = Vec3(-12.5f, 26.5f, -18.0f);
					boneEnd = modelMatrix.TransformPosition3D(boneEnd);

					AddVertsForSector3D(verts, gravScale, boneStart, boneEnd, orientation.m_pitchDegrees + 45.0f, 80.0f);

					g_theRenderer->BindShader(nullptr);
					g_theRenderer->BindTexture(nullptr);
					g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
					g_theRenderer->SetModelConstants(Mat44(), g_gravFieldColor);
					g_theRenderer->DrawVertexArray(verts);

					break;
				}
				case 2:
				{
					std::vector<Vertex_PCU> gravVerts;

					AddVertsForSphere3D(gravVerts, Vec3(), gravScale, 32, 16);

					Mat44 modelMatrix = orientation.GetAsMatrix_XFwd_YLeft_ZUp();
					modelMatrix.SetTranslation3D(position);

					g_theRenderer->BindShader(nullptr);
					g_theRenderer->BindTexture(nullptr);
					g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
					g_theRenderer->SetModelConstants(modelMatrix, g_previewGravFieldColor);
					g_theRenderer->DrawVertexArray(gravVerts);

					break;
				}
				case 3:
				{
					std::vector<Vertex_PCU> gravVerts;

					float halfLength = 49.0f;
					float halfWidth = 49.0f;

					Vec3 fbl = Vec3(halfLength, halfWidth, 0.0f);
					Vec3 fbr = Vec3(halfLength, -halfWidth, 0.0f);
					Vec3 bbl = Vec3(-halfLength, halfWidth, 0.0f);
					Vec3 bbr = Vec3(-halfLength, -halfWidth, 0.0f);
					Vec3 ftl = Vec3(halfLength, halfWidth, gravScale);
					Vec3 ftr = Vec3(halfLength, -halfWidth, gravScale);
					Vec3 btl = Vec3(-halfLength, halfWidth, gravScale);
					Vec3 btr = Vec3(-halfLength, -halfWidth, gravScale);

					AddVertsForCube3D(gravVerts, fbl, fbr, ftl, ftr, bbl, bbr, btl, btr);

					Mat44 modelMatrix = orientation.GetAsMatrix_XFwd_YLeft_ZUp();
					modelMatrix.SetTranslation3D(position);

					g_theRenderer->BindShader(nullptr);
					g_theRenderer->BindTexture(nullptr);
					g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
					g_theRenderer->SetModelConstants(modelMatrix, g_previewGravFieldColor);
					g_theRenderer->DrawVertexArray(gravVerts);

					break;
				}
			}
		}
	}
}


bool PrefabPLTD::CollideWithPlayer(Player* player)
{
	//early out if player is very far away
	if (GetDistanceSquared3D(player->m_position, m_position) > 20000.0f)
	{
		return false;
	}

	return m_model->PushPlayerOutOfAllTrisOnModel(player);
}


Vec3 PrefabPLTD::GetNearestPointOnPlanetoid(Vec3 playerPos) const
{
	return m_model->GetNearestPointOnModel(playerPos);
}


//
//teapot prefab planetoid functions
//
TeapotPLTD::TeapotPLTD(Vec3 position, float scale, EulerAngles orientation, Rgba8 color, bool includeField, float gravityRadius, float gravityForce)
	: PrefabPLTD(position, scale, orientation, color)
{
	m_model = new Model(position, scale, orientation, color);
	m_model->ParseXMLFileForOBJ("Data/Models/Teapot.xml");

	if(includeField) m_field = new SphereField(this, gravityRadius, gravityForce, Vec3(0.0f, 0.0f, 6.5f));
}


//
//sky station prefab planetoid functions
//
SkyStationPLTD::SkyStationPLTD(Vec3 position, float scale, EulerAngles orientation, Rgba8 color, bool includeField, float gravityRadius, float gravityForce)
	: PrefabPLTD(position, scale, orientation, color)
{
	m_model = new Model(position, scale, orientation, color);
	m_model->ParseXMLFileForOBJ("Data/Models/DrumSeparateRocketPlanet.xml");

	Mat44 modelMatrix = GetModelMatrix();
	modelMatrix.AppendScaleUniform3D(scale);
	Vec3 boneStart = Vec3(-12.5f, -25.5f, -18.0f);
	boneStart = modelMatrix.TransformPosition3D(boneStart);
	Vec3 boneEnd = Vec3(-12.5f, 26.5f, -18.0f);
	boneEnd = modelMatrix.TransformPosition3D(boneEnd);

	float forwardDegrees = 45.0f + orientation.m_pitchDegrees;
	float wedgeDegrees = 80.0f;
	if(includeField) m_field = new WedgeField(this, gravityRadius, boneStart, boneEnd, forwardDegrees, wedgeDegrees, gravityForce);
}


//
//mountain prefab planetoid functions
//
MountainPLTD::MountainPLTD(Vec3 position, float scale, EulerAngles orientation, Rgba8 color, bool includeField, float gravityRadius, float gravityForce)
	: PrefabPLTD(position, scale, orientation, color)
{
	m_model = new Model(position, scale, orientation, color);
	m_model->ParseXMLFileForOBJ("Data/Models/MountainPlanet.xml");

	if(includeField) m_field = new SphereField(this, gravityRadius, gravityForce);
}


//
//fortress prefab planetoid functions
//
FortressPLTD::FortressPLTD(Vec3 position, float scale, EulerAngles orientation, Rgba8 color, bool includeField, float gravityHeight, float gravityForce)
	: PrefabPLTD(position, scale, orientation, color)
{
	m_model = new Model(position, scale, orientation, color);
	m_model->ParseXMLFileForOBJ("Data/Models/OldFortressPlanet.xml");

	if(includeField) m_field = new PlaneField(this, 49.0f, 49.0f, gravityHeight, gravityForce);
}
