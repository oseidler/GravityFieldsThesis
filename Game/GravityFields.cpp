#include "Game/GravityFields.hpp"
#include "Game/Planetoids.hpp"
#include "Game/Player.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"


//
//plane gravity functions
//
void PlaneField::ApplyGravity(Player* player) const
{
	if (player == nullptr)
	{
		return;
	}

	Vec3 playerInPltdSpace = m_planetoid->GetModelMatrix().GetOrthonormalInverse().TransformPosition3D(player->m_position);

	//#TODO: offset stuff
	playerInPltdSpace.z = GetClamped(playerInPltdSpace.z, 0.0f, m_height);
	playerInPltdSpace.x = GetClamped(playerInPltdSpace.x, -m_halfLength, m_halfLength);
	playerInPltdSpace.y = GetClamped(playerInPltdSpace.y, -m_halfWidth, m_halfWidth);

	Vec3 nearestPointInField = m_planetoid->GetModelMatrix().TransformPosition3D(playerInPltdSpace);

	if (DoSpheresOverlap(player->m_position, player->m_collisionRadius, nearestPointInField, 0.01f))
	{
		Vec3 directionOfGravity = -m_planetoid->GetModelMatrix().GetKBasis3D();
		
		directionOfGravity.Normalize();
		directionOfGravity *= m_force;

		playerInPltdSpace.z = 0.0f;

		Vec3 nearestPointOnPlane = m_planetoid->GetModelMatrix().TransformPosition3D(playerInPltdSpace);

		player->SetGravitySource(this, nearestPointOnPlane, directionOfGravity);
	}
}


void PlaneField::DebugRender() const
{
	std::vector<Vertex_PCU> verts;

	//#TODO: offset stuff
	Vec3 fbl = Vec3(m_halfLength, m_halfWidth, 0.0f);
	Vec3 fbr = Vec3(m_halfLength, -m_halfWidth, 0.0f);
	Vec3 bbl = Vec3(-m_halfLength, m_halfWidth, 0.0f);
	Vec3 bbr = Vec3(-m_halfLength, -m_halfWidth, 0.0f);
	Vec3 ftl = Vec3(m_halfLength, m_halfWidth, m_height);
	Vec3 ftr = Vec3(m_halfLength, -m_halfWidth, m_height);
	Vec3 btl = Vec3(-m_halfLength, m_halfWidth, m_height);
	Vec3 btr = Vec3(-m_halfLength, -m_halfWidth, m_height);

	AddVertsForCube3D(verts, fbl, fbr, ftl, ftr, bbl, bbr, btl, btr);

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(m_planetoid->GetModelMatrix(), g_gravFieldColor);
	g_theRenderer->DrawVertexArray(verts);
}


//
//sphere gravity functions
//
void SphereField::ApplyGravity(Player* player) const
{
	if (player == nullptr)
	{
		return;
	}

	Vec3 fieldCenter = m_planetoid->m_position + m_offset;

	if(DoSpheresOverlap(fieldCenter, m_radius, player->m_position, player->m_collisionRadius))
	{
		Vec3 directionOfGravity = ((m_planetoid->m_position + m_offset) - player->m_position);
		directionOfGravity.Normalize();
		directionOfGravity *= m_force;

		Vec3 nearestPointOnPlanetoid = m_planetoid->GetNearestPointOnPlanetoid(player->m_position);
		player->SetGravitySource(this, nearestPointOnPlanetoid, directionOfGravity);
	}
}


void SphereField::DebugRender() const
{
	std::vector<Vertex_PCU> verts;

	AddVertsForSphere3D(verts, m_offset, m_radius, 32, 16);

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(m_planetoid->GetModelMatrix(), g_gravFieldColor);
	g_theRenderer->DrawVertexArray(verts);
}


//
//capsule gravity functions
//
void CapsuleField::ApplyGravity(Player* player) const
{
	if (player == nullptr)
	{
		return;
	}

	//#ToDo: Add offset stuff
	Vec3 nearestPointOnField = GetNearestPointOnCapsule3D(player->m_position, m_boneStart, m_boneEnd, m_radius);

	if (IsSphereInFixedPoint3D(player->m_position, player->m_collisionRadius, nearestPointOnField))
	{
		Vec3 nearestPointOnBone = GetNearestPointOnLineSegment3D(player->m_position, m_boneStart, m_boneEnd);
		Vec3 directionOfGravity = nearestPointOnBone - player->m_position;
		directionOfGravity.Normalize();
		directionOfGravity *= m_force;

		Vec3 nearestPointOnPlanetoid = m_planetoid->GetNearestPointOnPlanetoid(player->m_position);
		player->SetGravitySource(this, nearestPointOnPlanetoid, directionOfGravity);
	}
}


void CapsuleField::DebugRender() const
{
	std::vector<Vertex_PCU> verts;

	//#ToDo: Add offset stuff
	AddVertsForCapsule3D(verts, Vec3(), m_boneEnd - m_planetoid->m_position, m_radius, 32, 16);

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(m_planetoid->GetModelMatrix(), g_gravFieldColor);
	g_theRenderer->DrawVertexArray(verts);
}


//
//ellipsoid gravity functions
//
void EllipsoidField::ApplyGravity(Player* player) const
{
	if (player == nullptr)
	{
		return;
	}

	//#ToDo: Decouple planetoid type from field type
	//#ToDo: Add offset stuff
	EllipsoidPLTD* pltdAsEllipsoid = dynamic_cast<EllipsoidPLTD*>(m_planetoid);

	Vec3 nearestPointOnField = GetNearestPointOnEllipsoid3D(player->m_position, m_planetoid->m_position, m_xRadius, m_yRadius, m_zRadius, m_planetoid->m_orientation);

	if (IsSphereInFixedPoint3D(player->m_position, player->m_collisionRadius, nearestPointOnField))
	{
		Vec3 nearestPointOnPlanetoid = GetNearestPointOnEllipsoid3D(player->m_position, m_planetoid->m_position, pltdAsEllipsoid->m_xRadius, pltdAsEllipsoid->m_yRadius,
			pltdAsEllipsoid->m_zRadius, m_planetoid->m_orientation);
		Vec3 directionOfGravity = nearestPointOnPlanetoid - player->m_position;
		directionOfGravity.Normalize();
		directionOfGravity *= m_force;
		player->SetGravitySource(this, nearestPointOnPlanetoid, directionOfGravity);
	}
}


void EllipsoidField::DebugRender() const
{
	std::vector<Vertex_PCU> verts;

	//#ToDo: Add offset stuff
	AddVertsForEllipsoid3D(verts, Vec3(), m_xRadius, m_yRadius, m_zRadius, 32, 16);

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(m_planetoid->GetModelMatrix(), g_gravFieldColor);
	g_theRenderer->DrawVertexArray(verts);
}


//
//rounded cube gravity functions
//
void RoundCubeField::ApplyGravity(Player* player) const
{
	if (player == nullptr)
	{
		return;
	}

	//#TODO: Decouple planetoid type from field type
	//#ToDo: Add offset stuff
	RoundCubePLTD* pltdAsRoundCube = dynamic_cast<RoundCubePLTD*>(m_planetoid);

	Vec3 nearestPointOnField = GetNearestPointOnRoundedCube3D(player->m_position, pltdAsRoundCube->m_position, m_length, m_width, m_height, pltdAsRoundCube->m_roundedness, 
		pltdAsRoundCube->m_orientation);

	if (IsSphereInFixedPoint3D(player->m_position, player->m_collisionRadius, nearestPointOnField))
	{
		Vec3 nearestPointOnPlanetoid = GetNearestPointOnRoundedCube3D(player->m_position, pltdAsRoundCube->m_position, pltdAsRoundCube->m_length, pltdAsRoundCube->m_width,
			pltdAsRoundCube->m_height, pltdAsRoundCube->m_roundedness, pltdAsRoundCube->m_orientation);
		Vec3 directionOfGravity = nearestPointOnPlanetoid - player->m_position;
		directionOfGravity.Normalize();
		directionOfGravity *= m_force;
		player->SetGravitySource(this, nearestPointOnPlanetoid, directionOfGravity);
	}
}


void RoundCubeField::DebugRender() const
{
	std::vector<Vertex_PCU> verts;

	//#TODO: Decouple planetoid type from field type
	//#ToDo: Add offset stuff
	RoundCubePLTD* pltdAsRoundCube = dynamic_cast<RoundCubePLTD*>(m_planetoid);

	AddVertsForRoundedCube3D(verts, Vec3(), m_length * 0.5f, m_width * 0.5f, m_height * 0.5f, pltdAsRoundCube->m_roundedness);

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(m_planetoid->GetModelMatrix(), g_gravFieldColor);
	g_theRenderer->DrawVertexArray(verts);
}


//
//torus gravity functions
//
void TorusField::ApplyGravity(Player* player) const
{
	if (player == nullptr)
	{
		return;
	}

	Vec3 nearestPointOnField = GetNearestPointOnTorus3D(player->m_position, m_planetoid->m_position + m_offset, m_tubeRadius, m_holeRadius,
		m_planetoid->m_orientation);

	if (IsSphereInFixedPoint3D(player->m_position, player->m_collisionRadius, nearestPointOnField))
	{
		Vec3 nearestPointOnCenterWire = GetNearestPointOnTorus3D(player->m_position, m_planetoid->m_position + m_offset, 0.0f, m_holeRadius + m_tubeRadius, m_planetoid->m_orientation);
		Vec3 directionOfGravity = nearestPointOnCenterWire - player->m_position;
		directionOfGravity.Normalize();
		directionOfGravity *= m_force;

		Vec3 nearestPointOnPlanetoid = m_planetoid->GetNearestPointOnPlanetoid(player->m_position);
		player->SetGravitySource(this, nearestPointOnPlanetoid, directionOfGravity);
	}
}


void TorusField::DebugRender() const
{
	std::vector<Vertex_PCU> verts;

	AddVertsForTorus3D(verts, m_offset, m_tubeRadius, m_holeRadius);

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(m_planetoid->GetModelMatrix(), g_gravFieldColor);
	g_theRenderer->DrawVertexArray(verts);
}


//bowl gravity functions
void BowlField::ApplyGravity(Player* player) const
{
	if (player == nullptr)
	{
		return;
	}

	Mat44 bowlModelMat = m_planetoid->m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	bowlModelMat.SetTranslation3D(m_planetoid->m_position);
	Mat44 bowlWorldToLocalMat = bowlModelMat.GetOrthonormalInverse();
	Vec3 playerPosInLocalSpace = bowlWorldToLocalMat.TransformPosition3D(player->m_position);

	//get nearest point on hemisphere part
	Vec3 nearestPointOnHemisphereLocal = GetNearestPointOnSphere3D(playerPosInLocalSpace, Vec3(), m_radius);
	if (nearestPointOnHemisphereLocal.z > 0.0f)
	{
		Vec2 nearestPointOnHemisphereLocal2D = GetNearestPointOnDisc2D(Vec2(playerPosInLocalSpace.x, playerPosInLocalSpace.y), Vec2(), m_radius);
		nearestPointOnHemisphereLocal = Vec3(nearestPointOnHemisphereLocal2D.x, nearestPointOnHemisphereLocal2D.y, 0.0f);
	}

	//get nearest point on cylinder part
	Vec3 nearestPointOnCylinderLocal = GetNearestPointOnCylinder3D(playerPosInLocalSpace, Vec3(), Vec3(0.0f, 0.0f, m_height), m_radius);

	Vec3 nearestPointOnFieldLocal = nearestPointOnHemisphereLocal;
	if (GetDistanceSquared3D(nearestPointOnHemisphereLocal, playerPosInLocalSpace) > GetDistanceSquared3D(nearestPointOnCylinderLocal, playerPosInLocalSpace))
	{
		nearestPointOnFieldLocal = nearestPointOnCylinderLocal;
	}

	Vec3 nearestPointOnField = bowlModelMat.TransformPosition3D(nearestPointOnFieldLocal);

	if (IsSphereInFixedPoint3D(player->m_position, player->m_collisionRadius, nearestPointOnField))
	{
		Vec3 directionOfGravity = Vec3();
		if (nearestPointOnFieldLocal == nearestPointOnCylinderLocal)
		{
			directionOfGravity = -bowlModelMat.GetKBasis3D();
		}
		else
		{
			if (GetDistance3D(m_planetoid->m_position, player->m_position) <= m_innerRadius)
			{
				directionOfGravity = -((m_planetoid->m_position + m_offset) - player->m_position);
			}
			else
			{
				directionOfGravity = ((m_planetoid->m_position + m_offset) - player->m_position);
			}
		}
		directionOfGravity.Normalize();
		directionOfGravity *= m_force;

		Vec3 nearestPointOnPlanetoid = m_planetoid->GetNearestPointOnPlanetoid(player->m_position);
		player->SetGravitySource(this, nearestPointOnPlanetoid, directionOfGravity);
	}
}

void BowlField::DebugRender() const
{
	std::vector<Vertex_PCU> verts;

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

			Vec3 bottomLeftCoords = Vec3::MakeFromPolarDegrees(bottomDegreesLat, leftDegreesLong, m_radius);
			Vec3 bottomRightCoords = Vec3::MakeFromPolarDegrees(bottomDegreesLat, rightDegreesLong, m_radius);
			Vec3 topLeftCoords = Vec3::MakeFromPolarDegrees(topDegreesLat, leftDegreesLong, m_radius);
			Vec3 topRightCoords = Vec3::MakeFromPolarDegrees(topDegreesLat, rightDegreesLong, m_radius);

			verts.emplace_back(Vertex_PCU(bottomLeftCoords, Rgba8(), Vec2()));
			verts.emplace_back(Vertex_PCU(topRightCoords, Rgba8(), Vec2()));
			verts.emplace_back(Vertex_PCU(bottomRightCoords, Rgba8(), Vec2()));
			
			verts.emplace_back(Vertex_PCU(bottomLeftCoords, Rgba8(), Vec2()));
			verts.emplace_back(Vertex_PCU(topLeftCoords, Rgba8(), Vec2()));
			verts.emplace_back(Vertex_PCU(topRightCoords, Rgba8(), Vec2()));
		}
	}

	Vec3 topCenter = Vec3(0.0f, 0.0f, m_height);

	for (int edgeIndex = 0; edgeIndex < 32; edgeIndex++)
	{
		float startDegrees = static_cast<float>(edgeIndex) * degreesPerSlice;
		float endDegrees = static_cast<float>(edgeIndex + 1) * degreesPerSlice;

		Vec3 baseEdgeStart = Vec3::MakeFromPolarDegrees(0.0f, startDegrees, m_radius);
		Vec3 baseEdgeEnd = Vec3::MakeFromPolarDegrees(0.0f, endDegrees, m_radius);

		Vec3 topEdgeStart = topCenter + Vec3::MakeFromPolarDegrees(0.0f, startDegrees, m_radius);
		Vec3 topEdgeEnd = topCenter + Vec3::MakeFromPolarDegrees(0.0f, endDegrees, m_radius);

		//draw triangle at top
		verts.push_back(Vertex_PCU(topCenter, Rgba8(), Vec2()));
		verts.push_back(Vertex_PCU(topEdgeStart, Rgba8(), Vec2()));
		verts.push_back(Vertex_PCU(topEdgeEnd, Rgba8(), Vec2()));

		//draw side quad
		AddVertsForQuad3D(verts, baseEdgeStart, baseEdgeEnd, topEdgeStart, topEdgeEnd);
	}

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(m_planetoid->GetModelMatrix(), g_gravFieldColor);
	g_theRenderer->DrawVertexArray(verts);
}


//
//mobius strip gravity functions
//
void MobiusField::ApplyGravity(Player* player) const
{
	if (player == nullptr)
	{
		return;
	}


}



void MobiusField::DebugRender() const
{
	std::vector<Vertex_PCU> verts;

	//add verts for field

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(m_planetoid->GetModelMatrix(), g_gravFieldColor);
	g_theRenderer->DrawVertexArray(verts);
}


//
//wire gravity functions
//
void WireField::ApplyGravity(Player* player) const
{
	if (player == nullptr)
	{
		return;
	}

	//this is the only field type that will be allowed to be coupled with the planetoid type in the final version, due to the nature of the wire planetoid
	//#ToDo: Add offset stuff
	WirePLTD* pltdAsWire = dynamic_cast<WirePLTD*>(m_planetoid);

	Vec3 nearestPointOnField = Vec3();
	bool playerIsInField = false;

	for (int segIndex = 0; segIndex < pltdAsWire->m_wirePositions.size() - 1; segIndex++)
	{
		Vec3 wireStart = pltdAsWire->m_wirePositions[segIndex];
		wireStart = pltdAsWire->GetModelMatrix().TransformPosition3D(wireStart);
		Vec3 wireEnd = pltdAsWire->m_wirePositions[segIndex + 1];
		wireEnd = pltdAsWire->GetModelMatrix().TransformPosition3D(wireEnd);

		nearestPointOnField = GetNearestPointOnCapsule3D(player->m_position, wireStart, wireEnd, m_radius);
		if (IsSphereInFixedPoint3D(player->m_position, player->m_collisionRadius, nearestPointOnField))
		{
			playerIsInField = true;
			break;
		}
	}

	if (playerIsInField)
	{
		Vec3 nearestPointOnPlanetoid = Vec3(FLT_MAX, FLT_MAX, FLT_MAX);

		for (int segIndex = 0; segIndex < pltdAsWire->m_wirePositions.size() - 1; segIndex++)
		{
			Vec3 wireStart = pltdAsWire->m_wirePositions[segIndex];
			wireStart = pltdAsWire->GetModelMatrix().TransformPosition3D(wireStart);
			Vec3 wireEnd = pltdAsWire->m_wirePositions[segIndex + 1];
			wireEnd = pltdAsWire->GetModelMatrix().TransformPosition3D(wireEnd);

			Vec3 nearestPointOnSegment = GetNearestPointOnCapsule3D(player->m_position, wireStart, wireEnd, pltdAsWire->m_radius);
			if (GetDistanceSquared3D(player->m_position, nearestPointOnSegment) < GetDistanceSquared3D(player->m_position, nearestPointOnPlanetoid))
			{
				nearestPointOnPlanetoid = nearestPointOnSegment;
			}

			Vec3 directionOfGravity = nearestPointOnPlanetoid - player->m_position;
			directionOfGravity.Normalize();
			directionOfGravity *= m_force;
			player->SetGravitySource(this, nearestPointOnPlanetoid, directionOfGravity);
		}
	}
}


void WireField::DebugRender() const
{
	std::vector<Vertex_PCU> verts;

	//this is the only field type that will be allowed to be coupled with the planetoid type in the final version, due to the nature of the wire planetoid
	//#ToDo: Add offset stuff
	WirePLTD* pltdAsWire = dynamic_cast<WirePLTD*>(m_planetoid);

	for (int segIndex = 0; segIndex < pltdAsWire->m_wirePositions.size() - 1; segIndex++)
	{
		AddVertsForCapsule3D(verts, pltdAsWire->m_wirePositions[segIndex], pltdAsWire->m_wirePositions[segIndex + 1], m_radius);
	}

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(m_planetoid->GetModelMatrix(), g_gravFieldColor);
	g_theRenderer->DrawVertexArray(verts);
}


//
//cylinder field functions
//
void CylinderField::ApplyGravity(Player* player) const
{
	if (player == nullptr)
	{
		return;
	}


}


void CylinderField::DebugRender() const
{

}


//
//wedge field functions
//
void WedgeField::ApplyGravity(Player* player) const
{
	if (player == nullptr)
	{
		return;
	}

	Vec3 nearestPointOnField = GetNearestPointOnSector3D(player->m_position, m_start, m_end, m_radius, m_forwardDegrees, m_apertureDegrees);
	
	if (IsSphereInFixedPoint3D(player->m_position, player->m_collisionRadius, nearestPointOnField))
	{
		Vec3 nearestPointOnBone = GetNearestPointOnLineSegment3D(player->m_position, m_start, m_end);
		Vec3 directionOfGravity = nearestPointOnBone - player->m_position;
		directionOfGravity.Normalize();
		directionOfGravity *= m_force;

		Vec3 nearestPointOnPlanetoid = m_planetoid->GetNearestPointOnPlanetoid(player->m_position);
		player->SetGravitySource(this, nearestPointOnPlanetoid, directionOfGravity);
	}
}


void WedgeField::DebugRender() const
{
	std::vector<Vertex_PCU> verts;

	AddVertsForSector3D(verts, m_radius, m_start, m_end, m_forwardDegrees, m_apertureDegrees);
	//AddVertsForCylinder3D(verts, m_start, m_end, m_radius);

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(Mat44(), g_gravFieldColor);
	g_theRenderer->DrawVertexArray(verts);
}
