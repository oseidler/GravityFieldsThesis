#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB3.hpp"


//forward declarations
class  Shader;
class  Player;
class  Planetoid;
class  PlanePLTD;
class  SpherePLTD;
class  CapsulePLTD;
class  EllipsoidPLTD;
class  RoundCubePLTD;
class  TorusPLTD;
class  BowlPLTD;
class  MobiusPLTD;
class  WirePLTD;
struct WirePerlinParameters;
class  TeapotPLTD;
class  SkyStationPLTD;
class  MountainPLTD;
class  FortressPLTD;
class  Model;


class Game 
{
//public member functions
public:
	//game flow functions
	void Startup();
	void Update();
	void Render() const;
	void RenderImGui();
	void Shutdown();

	//planetoid spawning functions
	void ClearAllPlanetoids();
	void AddPlanetoidsForPlaytestingCourse();
	PlanePLTD*		SpawnPlane(Vec3 position, float halfLength, float halfWidth, EulerAngles orientation, bool includeField, float gravityHeight, float gravityForce = 100.0f, Rgba8 color = Rgba8());
	SpherePLTD*		SpawnSphere(Vec3 position, float radius, bool includeField, float gravityRadius, float gravityForce = 100.0f, Rgba8 color = Rgba8());
	CapsulePLTD*	SpawnCapsule(Vec3 position, float radius, float boneLength, Vec3 boneDirection, bool includeField, float gravityRadius, float gravityForce = 100.0f, Rgba8 color = Rgba8());
	EllipsoidPLTD*	SpawnEllipsoid(Vec3 position, float xRadius, float yRadius, float zRadius, EulerAngles orientation, bool includeField, float gravityXRadius, float gravityYRadius, float gravityZRadius, float gravityForce = 100.0f, Rgba8 color = Rgba8());
	RoundCubePLTD*	SpawnRoundedCube(Vec3 position, float length, float width, float height, float roundedness, EulerAngles orientation, bool includeField, float gravityLength, float gravityWidth, float gravityHeight, float gravityForce = 100.0f, Rgba8 color = Rgba8());
	TorusPLTD*		SpawnTorus(Vec3 position, float tubeRadius, float holeRadius, EulerAngles orientation, bool includeField, float gravityRadius, float gravityForce = 100.0f, Rgba8 color = Rgba8());
	BowlPLTD*		SpawnBowl(Vec3 position, float radius, float thickness, EulerAngles orientation, bool includeField, float gravityRadius, float gravityForce = 100.0f, Rgba8 color = Rgba8());
	MobiusPLTD*		SpawnMobiusStrip(Vec3 position, float radius, float width, EulerAngles orientation, bool includeField, float gravityHeight, float gravityForce = 100.0f, Rgba8 color = Rgba8());
	WirePLTD*		SpawnWire(Vec3 position, float radius, WirePerlinParameters perlinStruct, EulerAngles orientation, bool includeField, float gravityRadius, float gravityForce = 100.0f, Rgba8 color = Rgba8());
	TeapotPLTD*		SpawnTeapot(Vec3 position, float scale, EulerAngles orientation, bool includeField, float gravityRadius, float gravityForce, Rgba8 color);
	SkyStationPLTD* SpawnSkyStation(Vec3 position, float scale, EulerAngles orientation, bool includeField, float gravityRadius, float gravityForce, Rgba8 color);
	MountainPLTD*	SpawnMountain(Vec3 position, float scale, EulerAngles orientation, bool includeField, float gravityRadius, float gravityForce, Rgba8 color);
	FortressPLTD*	SpawnFortress(Vec3 position, float scale, EulerAngles orientation, bool includeField, float gravityHeight, float gravityForce, Rgba8 color);

	//mode switching functions
	void EnterSandboxMode();
	void ExitSandboxMode();

//public member variables
public:
	//game state bools
	bool		m_isFinished = false;
	bool		m_isDebugView = false;

	//game clock
	Clock m_gameClock = Clock();

	//game entities
	std::vector<Planetoid*> m_planetoids;
	Player* m_player = nullptr;
	Model*  m_previewModel = nullptr;
	int		m_previousModelIndex = -1;

	//rendering variables
	Shader* m_lightingShader = nullptr;
	Rgba8   m_skyColor = Rgba8(50, 50, 50);
	Vec3    m_sunDirection = Vec3(0.5f, -0.5f, -1.0f);
	float   m_sunIntensity = 0.925f;
	float   m_ambientIntensity = 0.35f;

	//imgui variables
	bool m_isSandboxMode = false;
	bool m_lightingMenuOpen = false;

	//playtest course variables
	Vec3 m_playtestEnterZone = Vec3(115.0f, 0.0f, 5.0f);
	Vec3 m_playtestStartingPoint = Vec3(250.0f, 0.0f, 11.5f);
	bool m_inPlaytestCourse = false;
	std::vector<AABB3> m_checkpoints;
	AABB3* m_currentCheckpoint = nullptr;
	int	m_currentSection = 0;
	float m_section1Duration = 0.0f;
	float m_section2Duration = 0.0f;
	float m_section3Duration = 0.0f;
	float m_section4Duration = 0.0f;
	float m_section1StartTime = -1.0f;
	float m_section2StartTime = -1.0f;
	float m_section3StartTime = -1.0f;
	float m_section4StartTime = -1.0f;
	std::vector<float> m_standardJumpTimes;
	std::vector<float> m_doubleJumpTimes;
	std::vector<float> m_tripleJumpTimes;
	std::vector<float> m_longJumpTimes;
	std::vector<float> m_wallJumpTimes;
	std::vector<float> m_sideFlipTimes;
	std::vector<float> m_backFlipTimes;
	std::vector<float> m_section1RespawnTimes;
	std::vector<float> m_section2RespawnTimes;
	std::vector<float> m_section3RespawnTimes;
	std::vector<float> m_section4RespawnTimes;

//private member functions
private:
	//game flow sub-functions
	void RenderPlanetoids() const;

	//gravity management functions
	void ApplyGravity();

	//collision management functions
	void CollidePlayerWithAllPlanetoids();

//private member variables
private:
	//camera variables
	Camera m_screenCamera;
};
