#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/EulerAngles.hpp"


//forward declarations
class CPUMesh;
class GPUMesh;
class Shader;
struct Mat44;
struct Vec3;
class Player;


class Model
{
//public member functions
public:
	Model(Vec3 position, float scale, EulerAngles orientation, Rgba8 color);
	~Model();
	
	//model creation and rendering
	bool ParseXMLFileForOBJ(std::string const& fileName);
	void RenderGPUMesh(Vec3 sunDirection, float sunIntensity, float ambientIntensity) const;

	//game-centric model functions
	Mat44 GetModelMatrix() const;
	Vec3 GetNearestPointOnModel(Vec3 const& referencePoint) const;
	bool PushPlayerOutOfAllTrisOnModel(Player* player);

//public member variables
public:
	CPUMesh* m_cpuMesh = nullptr;
	GPUMesh* m_gpuMesh = nullptr;
	Shader*  m_shader = nullptr;

	Vec3	 m_position = Vec3();
	float	 m_scale = 1.0f;
	Rgba8	 m_color = Rgba8();
	EulerAngles m_orientation = EulerAngles();
};
