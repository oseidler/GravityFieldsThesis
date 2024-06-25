#include "Game/Model.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Player.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Core/OBJLoader.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/MathUtils.hpp"


//
//constructor and destructor
//
Model::Model(Vec3 position, float scale, EulerAngles orientation, Rgba8 color)
	: m_position(position)
	, m_scale(scale)
	, m_orientation(orientation)
	, m_color(color)
{
	m_cpuMesh = new CPUMesh();
	m_gpuMesh = new GPUMesh();
}


Model::~Model()
{
	if (m_cpuMesh != nullptr)
	{
		delete m_cpuMesh;
	}
	if (m_gpuMesh != nullptr)
	{
		delete m_gpuMesh;
	}
}

//
//model creation and rendering
//
bool Model::ParseXMLFileForOBJ(std::string const& fileName)
{
	//parse xml for obj file name and fixup matrix
	XmlDocument modelXml;
	XmlError result = modelXml.LoadFile(fileName.c_str());
	if (result != tinyxml2::XML_SUCCESS)
	{
		ERROR_RECOVERABLE("Failed to open model xml file!");
		return false;
	}
	XmlElement* rootElement = modelXml.RootElement();
	if (rootElement == nullptr)
	{
		ERROR_RECOVERABLE("Failed to read model xml root element!");
		return false;
	}
	
	std::string const& objFilePath = ParseXmlAttribute(*rootElement, "path", "invalid path");
	std::string const& shaderName = ParseXmlAttribute(*rootElement, "shader", "invalid shader path");
	if (shaderName == "invalid shader path")
	{
		ERROR_RECOVERABLE("Couldn't find shader in xml!");
		return false;
	}
	m_shader = g_theRenderer->CreateShader(shaderName.c_str());

	XmlElement* transformElement = rootElement->FirstChildElement();
	Mat44 matrix = Mat44();
	Vec3 iBasis = ParseXmlAttribute(*transformElement, "x", Vec3());
	Vec3 jBasis = ParseXmlAttribute(*transformElement, "y", Vec3());
	Vec3 kBasis = ParseXmlAttribute(*transformElement, "z", Vec3());
	Vec3 translation = ParseXmlAttribute(*transformElement, "t", Vec3());
	matrix.SetIJKT3D(iBasis, jBasis, kBasis, translation);
	matrix.AppendScaleUniform3D(ParseXmlAttribute(*transformElement, "scale", 1.0f));

	//pass into obj loader along with vertex and index vectors from cpu mesh
	OBJLoader::LoadObjFile(objFilePath, matrix, m_cpuMesh->m_vertexes, m_cpuMesh->m_indexes);

	m_gpuMesh->m_vertexBuffer = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCUTBN), sizeof(Vertex_PCUTBN));
	g_theRenderer->CopyCPUToGPU(m_cpuMesh->m_vertexes.data(), static_cast<int>(m_cpuMesh->m_vertexes.size()) * sizeof(Vertex_PCUTBN), m_gpuMesh->m_vertexBuffer);

	if (m_cpuMesh->m_indexes.size() > 0)
	{
		m_gpuMesh->m_indexBuffer = g_theRenderer->CreateIndexBuffer(sizeof(int));
		g_theRenderer->CopyCPUToGPU(m_cpuMesh->m_indexes.data(), static_cast<int>(m_cpuMesh->m_indexes.size()) * sizeof(int), m_gpuMesh->m_indexBuffer);
	}

	return true;
}


void Model::RenderGPUMesh(Vec3 sunDirection, float sunIntensity, float ambientIntensity) const
{
	g_theRenderer->BindShader(m_shader);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetModelConstants(GetModelMatrix(), m_color);
	g_theRenderer->SetLightConstants(sunDirection, sunIntensity, ambientIntensity);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);

	if (m_cpuMesh->m_indexes.size() > 0)
	{
		g_theRenderer->DrawVertexBufferIndexed(m_gpuMesh->m_vertexBuffer, m_gpuMesh->m_indexBuffer, static_cast<int>(m_cpuMesh->m_indexes.size()));
	}
	else
	{
		g_theRenderer->DrawVertexBuffer(m_gpuMesh->m_vertexBuffer, static_cast<int>(m_cpuMesh->m_vertexes.size()));
	}
}



//
//game-centric model functions
//
Mat44 Model::GetModelMatrix() const
{
	Mat44 modelMatrix = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();

	modelMatrix.AppendScaleUniform3D(m_scale);

	modelMatrix.SetTranslation3D(m_position);

	return modelMatrix;
}


Vec3 Model::GetNearestPointOnModel(Vec3 const& referencePoint) const
{
	Vec3 referencePointLocal = GetModelMatrix().GetOrthonormalInverse().TransformPosition3D(referencePoint);
	
	Vec3 currentNearestLocalPoint = Vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	float nearestLocalPointDistSq = GetDistanceSquared3D(currentNearestLocalPoint, referencePointLocal);
	
	for (int vertIndex = 0; vertIndex < m_cpuMesh->m_indexes.size(); vertIndex+=3)
	{
		Vec3 triNearestLocalPoint = GetNearestPointOnTriangle3D(referencePointLocal, m_cpuMesh->m_vertexes[m_cpuMesh->m_indexes[vertIndex]].m_position, 
			m_cpuMesh->m_vertexes[m_cpuMesh->m_indexes[vertIndex+1]].m_position, m_cpuMesh->m_vertexes[m_cpuMesh->m_indexes[vertIndex+2]].m_position);

		if (GetDistanceSquared3D(triNearestLocalPoint, referencePointLocal) < nearestLocalPointDistSq)
		{
			currentNearestLocalPoint = triNearestLocalPoint;
			nearestLocalPointDistSq = GetDistanceSquared3D(currentNearestLocalPoint, referencePointLocal);
		}
	}
	
	Vec3 currentNearestPoint = GetModelMatrix().TransformPosition3D(currentNearestLocalPoint);

	return currentNearestPoint;
}


bool Model::PushPlayerOutOfAllTrisOnModel(Player* player)
{
	Vec3 referencePointLocal = GetModelMatrix().GetOrthonormalInverse().TransformPosition3D(player->m_position);

	bool wasPlayerPushed = false;
	for (int vertIndex = 0; vertIndex < m_cpuMesh->m_indexes.size(); vertIndex += 3)
	{
		Vec3 pointALocal = m_cpuMesh->m_vertexes[m_cpuMesh->m_indexes[vertIndex]].m_position;
		Vec3 pointBLocal = m_cpuMesh->m_vertexes[m_cpuMesh->m_indexes[vertIndex + 1]].m_position;
		Vec3 pointCLocal = m_cpuMesh->m_vertexes[m_cpuMesh->m_indexes[vertIndex + 2]].m_position;

		if (pointALocal == pointBLocal || pointALocal == pointCLocal || pointBLocal == pointCLocal)
		{
			continue;
		}
		
		Vec3 triNearestLocalPoint = GetNearestPointOnTriangle3D(referencePointLocal, m_cpuMesh->m_vertexes[m_cpuMesh->m_indexes[vertIndex]].m_position,
			m_cpuMesh->m_vertexes[m_cpuMesh->m_indexes[vertIndex + 1]].m_position, m_cpuMesh->m_vertexes[m_cpuMesh->m_indexes[vertIndex + 2]].m_position);

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
			//if surface is ground, player becomes grounded
			if (DotProduct3D(pushDirection, -player->m_orientation.GetKBasis3D()) > GROUNDED_THRESHOLD)
			{
				player->BecomeGrounded();
			}
			//if surface is wall, player begins wall slide
			else if (DotProduct3D(pushDirection, player->m_orientation.GetIBasis3D()) > WALL_THRESHOLD && !player->m_isGrounded)
			{
				Vec3 pointA = GetModelMatrix().TransformPosition3D(pointALocal);
				Vec3 pointB = GetModelMatrix().TransformPosition3D(pointBLocal);
				Vec3 pointC = GetModelMatrix().TransformPosition3D(pointCLocal);
				Vec3 triNormal = CrossProduct3D(pointB - pointA, pointC - pointA);
				player->StartWallSlide(triNormal);
			}
		}
	}

	return wasPlayerPushed;
}
