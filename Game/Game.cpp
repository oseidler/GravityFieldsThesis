#include "Game/Game.hpp"
#include "Game/Player.hpp"
#include "Game/App.hpp"
#include "Game/Planetoids.hpp"
#include "Game/GravityFields.hpp"
#include "Game/Model.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Core/BufferUtils.hpp"
#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/backends/imgui_impl_win32.h"
#include "ThirdParty/imgui/backends/imgui_impl_dx11.h"


//game flow functions
void Game::Startup()
{
	//add player to scene
	m_player = new Player(this);
	m_player->m_position = m_player->m_playerStartPosition;
	m_player->m_playerCamera.SetUseMatrixOrientationMode(false);

	//create lighting shader
	m_lightingShader = g_theRenderer->CreateShader("Data/Shaders/SpriteLit");
	
	//add test planetoids to scene
	AddPlanetoidsForPlaytestingCourse();
	//SpawnPlane(Vec3(1.0f, -1.0f, -25.0f), 50.0f, 50.0f, EulerAngles(), true, 20.0f, GRAVITY_STANDARD, Rgba8(90, 0, 140));
	//SpawnSphere(Vec3(28.0f, 0.0f, 0.0f), 7.0f, true, 20.0f, GRAVITY_STANDARD, Rgba8(220, 120, 50));
	//SpawnCapsule(Vec3(2.5f, 28.0f, 0.0f), 3.0f, 5.0f, Vec3(0.0f, 0.5f, 0.5f), true, 7.0f, GRAVITY_STANDARD, Rgba8(128, 0, 0));
	//SpawnEllipsoid(Vec3(18.0f, -18.0f, 6.5f), 5.0f, 4.0f, 8.0f, EulerAngles(0.0f, 15.0f, 45.0f), true, 7.0f, 6.0f, 10.0f, GRAVITY_STANDARD, Rgba8(50, 150, 255));
	//SpawnTorus(Vec3(47.0f, 0.0f, 5.0f), 3.0f, 4.0f, EulerAngles(0.0f, -30.0f, -60.0f), true, 4.5f, GRAVITY_STANDARD, Rgba8(0, 255, 100));
	//SpawnRoundedCube(Vec3(6.0f, 15.0f, 0.0f), 10.0f, 8.0f, 6.0f, 0.1f, EulerAngles(30.0f, 0.0f, 0.0f), true, 14.0f, 12.0f, 10.0f, GRAVITY_STANDARD, Rgba8(200, 200, 200));
	//SpawnRoundedCube(Vec3(8.0f, 0.0f, 0.0f), 10.0f, 8.0f, 6.0f, 0.5f, EulerAngles(), true, 15.0f, 13.0f, 11.0f, GRAVITY_STANDARD, Rgba8(130, 130, 130));
	//SpawnRoundedCube(Vec3(6.0f, -15.0f, 0.0f), 10.0f, 8.0f, 6.0f, 0.8f, EulerAngles(0.0f, 60.0f, 15.0f), true, 14.0f, 12.0f, 10.0f, GRAVITY_STANDARD, Rgba8(75, 75, 75));
	//WirePerlinParameters ps;
	//ps.m_rngSeed = static_cast<int>(GetCurrentTimeSeconds());
	//ps.m_minSegments = 7;
	//ps.m_maxSegments = 10;
	//ps.m_maxPitchChange = 60.0f;
	//ps.m_perlinOctavesPitch = 4;
	//ps.m_maxYawChange = 60.0f;
	//ps.m_perlinOctavesYaw = 4;
	//ps.m_segMinLength = 2.5f;
	//ps.m_segMaxLength = 5.5f;
	//SpawnWire(Vec3(25.0f, 15.0f, 1.0f), 2.5f, ps, EulerAngles(0.0f, 25.0f, 90.0f), true, 4.5f, GRAVITY_STANDARD, Rgba8(255, 127, 0));
	//SpawnTeapot(Vec3(-45.0f, 5.0f, 15.0f), 1.0f, EulerAngles(0.0f, 0.0f, 0.0f), true, 17.5f, GRAVITY_STANDARD, Rgba8(255, 0, 255));
	//SpawnSkyStation(Vec3(-45.0f, -15.0f, 0.0f), 1.0f, EulerAngles(90.0f, 0.0f, 0.0f), true, 40.0f, GRAVITY_STANDARD, Rgba8(0, 255, 255));
	//SpawnMountain(Vec3(-28.0f, -40.0f, 0.0f), 1.0f, EulerAngles(0.0f, 0.0f, 0.0f), true, 40.0f, GRAVITY_STANDARD, Rgba8(255, 255, 0));
	//SpawnFortress(Vec3(-60.0f, 60.0f, -5.0f), 1.0f, EulerAngles(0.0f, 0.0f, 0.0f), true, 35.0f, GRAVITY_STANDARD, Rgba8(127, 200, 56));
	//SpawnBowl(Vec3(15.0f, 15.0f, 15.0f), 10.0f, 3.0f, EulerAngles(30.0f, 10.0f, 45.0f), true, 5.0f, GRAVITY_STANDARD, Rgba8(195, 178, 134));
	////SpawnMobiusStrip(Vec3(8.0f, 0.0f, 15.0f), 5.0f, 3.0f, EulerAngles(), true, 1.0f, GRAVITY_STANDARD, Rgba8(200, 200, 0));

	//SpawnPlane(Vec3(1.0f, 3.0f, -15.0f), 4.0f, 10.0f, EulerAngles(0.0f, 0.0f, 90.0f), false, 0.0f, 0.0f, Rgba8(200, 0, 0));
	//SpawnPlane(Vec3(1.0f, -3.0f, -15.0f), 4.0f, 10.0f, EulerAngles(0.0f, 0.0f, -90.0f), false, 0.0f, 0.0f, Rgba8(200, 0, 0));
	//SpawnPlane(Vec3(1.0f, 3.01f, -15.0f), 4.0f, 10.0f, EulerAngles(0.0f, 0.0f, -90.0f), false, 0.0f, 0.0f, Rgba8(200, 0, 0));
	//SpawnPlane(Vec3(1.0f, -3.01f, -15.0f), 4.0f, 10.0f, EulerAngles(0.0f, 0.0f, 90.0f), false, 0.0f, 0.0f, Rgba8(200, 0, 0));

	//set camera bounds
	m_player->m_playerCamera.SetOrthoView(Vec2(WORLD_CAMERA_MIN_X, WORLD_CAMERA_MIN_Y), Vec2(WORLD_CAMERA_MAX_X, WORLD_CAMERA_MAX_Y));
	m_player->m_playerCamera.SetPerspectiveView(g_theWindow->GetConfig().m_clientAspect, 60.0f, 0.1f, 400.0f);
	m_player->m_playerCamera.SetRenderBasis(Vec3(0.0f, 0.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));

	m_screenCamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y));

	//add world axes
	DebugAddWorldArrow(Vec3(), Vec3(1.0f, 0.0f, 0.0f), 0.1f, -1.0f, Rgba8(255, 0, 0), Rgba8(255, 0, 0));
	DebugAddWorldArrow(Vec3(), Vec3(0.0f, 1.0f, 0.0f), 0.1f, -1.0f, Rgba8(0, 255, 0), Rgba8(0, 255, 0));
	DebugAddWorldArrow(Vec3(), Vec3(0.0f, 0.0f, 1.0f), 0.1f, -1.0f, Rgba8(0, 0, 255), Rgba8(0, 0, 255));
}


void Game::Update()
{
	//key press to switch in and out of sandbox mode
	if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
	{
		if (m_isSandboxMode) ExitSandboxMode();
		else				 EnterSandboxMode();
	}

	//key to enable or disable the lighting menu
	if (g_theInput->WasKeyJustPressed(KEYCODE_F6))
	{
		m_lightingMenuOpen = !m_lightingMenuOpen;
	}

	Clock& sysClock = Clock::GetSystemClock();
	std::string gameInfo = Stringf("Time: %.2f  FPS: %.1f  Time Scale: %.2f", sysClock.GetTotalSeconds(), 1.0f/sysClock.GetDeltaSeconds(), m_gameClock.GetTimeScale());
	DebugAddScreenText(gameInfo, Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y), 16.0f, Vec2(1.0f, 1.0f), 0.0f, Rgba8(), Rgba8());

	Vec3& pos = m_player->m_position;
	std::string posMessage = Stringf("Player position: %.2f, %.2f, %.2f", pos.x, pos.y, pos.z);
	DebugAddMessage(posMessage, 0.0f);

	//update player
	m_player->Update(m_gameClock.GetDeltaSeconds());

	//update gravity fields
	ApplyGravity();

	//handle collision
	CollidePlayerWithAllPlanetoids();

	//teleport player to playtest course when they enter the starting area
	if (!m_inPlaytestCourse && (GetDistanceSquared3D(pos, m_playtestEnterZone) < 5.0f)/* || g_theInput->WasKeyJustPressed(KEYCODE_COMMA)*/)
	{
		m_inPlaytestCourse = true;
		pos = m_playtestStartingPoint;
		m_player->m_orientation = Mat44();
	}
	/*if (g_theInput->WasKeyJustPressed(KEYCODE_PERIOD))
	{
		m_inPlaytestCourse = true;
		pos = m_checkpoints[1].GetCenter();
		m_player->m_orientation = Mat44();
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_SEMICOLON))
	{
		m_inPlaytestCourse = true;
		pos = m_checkpoints[2].GetCenter();
		m_player->m_orientation = Mat44();
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_SINGLEQUOTE))
	{
		m_inPlaytestCourse = true;
		pos = m_checkpoints[3].GetCenter();
		m_player->m_orientation = Mat44();
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFTBRACKET))
	{
		m_inPlaytestCourse = true;
		pos = Vec3(1100.0f, -108.0f, 500.0f);
		m_player->m_orientation = Mat44();
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHTBRACKET))
	{
		m_inPlaytestCourse = true;
		pos = Vec3(1323.0f, -108.0f, 829.0f);
		m_player->m_orientation = Mat44();
	}*/

	//update playtest course
	if (m_inPlaytestCourse)
	{
		//for playtest course, update checkpoints
		for (int cpIndex = 0; cpIndex < m_checkpoints.size(); cpIndex++)
		{
			AABB3& cp = m_checkpoints[cpIndex];

			Vec3 nearestPoint = GetNearestPointOnAABB3D(pos, cp);
			if (IsPointInsideSphere3D(nearestPoint, pos, m_player->m_collisionRadius))
			{
				if (m_currentCheckpoint != &cp)
				{
					m_currentCheckpoint = &cp;
					m_currentSection = cpIndex + 1;

					switch (m_currentSection)
					{
						case 1:
						{
							m_section1StartTime = m_gameClock.GetTotalSeconds();
							break;
						}
						case 2:
						{
							m_section2StartTime = m_gameClock.GetTotalSeconds();
							break;
						}
						case 3:
						{
							m_section3StartTime = m_gameClock.GetTotalSeconds();
							break;
						}
						case 4:
						{
							m_section4StartTime = m_gameClock.GetTotalSeconds();
							break;
						}
					}
				}
			}
		}

		std::string sectionStr = Stringf("Current Section: %i", m_currentSection);
		DebugAddMessage(sectionStr, 0.0f);

		//track time of each section
		switch (m_currentSection)
		{
			case 1:
			{
				m_section1Duration += m_gameClock.GetDeltaSeconds();
				//DebugAddMessage("Current Section: 1", 0.0f);
				/*std::string section1TimeStr = Stringf("Section Time: %.2f", m_section1Duration);
				DebugAddMessage(section1TimeStr, 0.0f);*/
				break;
			}
			case 2:
			{
				m_section2Duration += m_gameClock.GetDeltaSeconds();
				//DebugAddMessage("Current Section: 2", 0.0f);
				/*std::string section2TimeStr = Stringf("Section Time: %.2f", m_section2Duration);
				DebugAddMessage(section2TimeStr, 0.0f);*/
				break;
			}
			case 3:
			{
				m_section3Duration += m_gameClock.GetDeltaSeconds();
				//DebugAddMessage("Current Section: 3", 0.0f);
				/*std::string section3TimeStr = Stringf("Section Time: %.2f", m_section3Duration);
				DebugAddMessage(section3TimeStr, 0.0f);*/
				break;
			}
			case 4: 
			{
				m_section4Duration += m_gameClock.GetDeltaSeconds();
				//DebugAddMessage("Current Section: 4", 0.0f);
				/*std::string section4TimeStr = Stringf("Section Time: %.2f", m_section4Duration);
				DebugAddMessage(section4TimeStr, 0.0f);*/
				break;
			}
		}
	}
}


void Game::Render() const
{
	g_theRenderer->ClearScreen(m_skyColor);

	g_theRenderer->BeginCamera(m_player->m_playerCamera);	//render game world with the world camera

	//game renderering here
	g_theRenderer->SetLightConstants(g_theGame->m_sunDirection, g_theGame->m_sunIntensity, g_theGame->m_ambientIntensity);
	RenderPlanetoids();
	m_player->Render();

	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::DISABLED);
	if (m_isDebugView)
	{
		for (int pltdIndex = 0; pltdIndex < m_planetoids.size(); pltdIndex++)
		{
			if (m_planetoids[pltdIndex] != nullptr)
			{
				m_planetoids[pltdIndex]->DebugRender();
			}
		}
	}
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);

	g_theRenderer->EndCamera(m_player->m_playerCamera);

	//debug world rendering
	DebugRenderWorld(m_player->m_playerCamera);

	//debug screen rendering
	DebugRenderScreen(m_screenCamera);
}


void Game::RenderImGui()
{
	//declare static variables to hold imgui data
	static int currentPlanetoidShape = 0;
	static const char* comboBoxOptions = { "Quad\0Sphere\0Capsule\0Ellipsoid\0Rounded Cube\0Torus\0Bowl\0Perlin Wire\0Model\0" };
	static float pltdPos[3] = {0.0f, 0.0f, 0.0f};

	static float planeHalfLength = 1.0f;
	static float planeHalfWidth = 1.0f;
	static float planeAngle[3] = { 0.0f, 0.0f, 0.0f };
	static float planeGravityHeight = 1.0f;

	static float spherePltdRadius = 1.0f;
	static float sphereGravityRadius = 2.0f;

	static float capsuleRadius = 1.0f;
	static float capsuleLength = 1.0f;
	static float capsuleDir[3] = { 1.0f, 0.0f, 0.0f };
	static float capsuleGravityRadius = 1.0f;

	static float ellipsoidRadii[3] = { 1.0f, 1.0f, 1.0f };
	static float ellipsoidAngle[3] = { 0.0f, 0.0f, 0.0f };
	static float ellipsoidGravRadii[3] = { 2.0f, 2.0f, 2.0f };

	static float roundCubeDim[3] = { 1.0f, 1.0f, 1.0f };
	static float roundCubeRounded = 0.5f;
	static float roundCubeAngle[3] = { 0.0f, 0.0f, 0.0f };
	static float roundCubeGravDim[3] = { 2.0f, 2.0f, 2.0f };

	static float torusTubeRadius = 1.0f;
	static float torusHoleRadius = 1.0f;
	static float torusAngle[3] = { 0.0f, 0.0f, 0.0f };
	static float torusGravRadius = 2.0f;

	static float bowlRadius = 1.0f;
	static float bowlThickness = 0.1f;
	static float bowlAngle[3] = { 0.0f, 0.0f, 0.0f };
	static float bowlGravRadius = 1.0f;

	/*static float mobiusStripRadius = 1.0f;
	static float mobiusStripWidth = 1.0f;
	static float mobiusStripAngle[3] = { 0.0f, 0.0f, 0.0f };
	static float mobiusStripGravHeight = 1.0f;*/

	static float wireRadius = 1.0f;
	static float wireAngle[3] = { 0.0f, 0.0f, 0.0f };
	static WirePerlinParameters wirePerlin = WirePerlinParameters();
	static float wireGravRadius = 2.0f;

	static const char* prefabTypes = { "Teapot\0Sky Station\0Mountain Planet\0Fortress" };
	static int currentPrefabType = 0;
	static float prefabAngle[3] = { 0.0f, 0.0f, 0.0f };
	static float prefabGravScale = 1.0f;

	static bool  includeGravField = true;
	static float pltdGravityForce = GRAVITY_STANDARD;

	static Rgba8 pltdColor = Rgba8();
	static float pltdSpawnColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	constexpr float FLOAT_BOX_WIDTH = 119.0f;

	//if in sandbox mode, render imgui for spawning planetoids
	if (m_isSandboxMode)
	{
		g_theRenderer->BeginCamera(m_player->m_playerCamera);
		ImGui::SetNextWindowPos(ImVec2(10.0f, 20.0f));
		ImGui::SetNextWindowSize(ImVec2(350.0f, 650.0f));
		ImGui::Begin("Planet Spawn Menu");

		ImGui::Combo("Planet Shape", &currentPlanetoidShape, comboBoxOptions);
		ImGui::NewLine();

		ImGui::InputFloat3("Position", pltdPos, "%.1f");

		//plane
		if (currentPlanetoidShape == 0)
		{
			ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
			ImGui::InputFloat("Half Length", &planeHalfLength, 1.0f, 0.5f, "%.1f");
			ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
			ImGui::InputFloat("Half Width", &planeHalfWidth, 1.0f, 0.5f, "%.1f");
			ImGui::InputFloat3("Orientation", planeAngle, "%.1f");
			planeHalfLength = GetClamped(planeHalfLength, 0.0f, FLT_MAX);
			planeHalfWidth = GetClamped(planeHalfWidth, 0.0f, FLT_MAX);
			
			ImGui::NewLine();
			ImGui::Checkbox("Include Gravity Field", &includeGravField);
			if (includeGravField)
			{
				ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
				ImGui::InputFloat("Gravity Field Height", &planeGravityHeight, 1.0f, 0.5f, "%.1f");
				planeGravityHeight = GetClamped(planeGravityHeight, 0.0f, FLT_MAX);

				PlanePLTD::RenderPreview(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), planeHalfLength, planeHalfWidth, EulerAngles(planeAngle[0], planeAngle[1], planeAngle[2]), planeGravityHeight,
					pltdColor);
			}
			else
			{
				PlanePLTD::RenderPreview(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), planeHalfLength, planeHalfWidth, EulerAngles(planeAngle[0], planeAngle[1], planeAngle[2]), 0.0f,
					pltdColor);
			}
		}
		//sphere
		else if (currentPlanetoidShape == 1)
		{
			ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
			ImGui::InputFloat("Radius", &spherePltdRadius, 1.0f, 0.5f, "%.1f");
			spherePltdRadius = GetClamped(spherePltdRadius, 0.0f, FLT_MAX);

			ImGui::NewLine();
			ImGui::Checkbox("Include Gravity Field", &includeGravField);
			if (includeGravField)
			{
				ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
				ImGui::InputFloat("Gravity Field Radius", &sphereGravityRadius, 1.0f, 0.5f, "%.1f");
				sphereGravityRadius = GetClamped(sphereGravityRadius, 0.0f, FLT_MAX);

				SpherePLTD::RenderPreview(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), spherePltdRadius, sphereGravityRadius, pltdColor);
			}
			else
			{
				SpherePLTD::RenderPreview(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), spherePltdRadius, 0.0f, pltdColor);
			}
		}
		//capsule
		else if (currentPlanetoidShape == 2)
		{
			ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
			ImGui::InputFloat("Radius", &capsuleRadius, 1.0f, 0.5f, "%.1f");
			capsuleRadius = GetClamped(capsuleRadius, 0.0f, FLT_MAX);
			ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
			ImGui::InputFloat("Length", &capsuleLength, 1.0f, 0.5f, "%.1f");
			capsuleLength = GetClamped(capsuleLength, 0.0f, FLT_MAX);
			ImGui::InputFloat3("Direction", capsuleDir, "%.1f");

			ImGui::NewLine();
			ImGui::Checkbox("Include Gravity Field", &includeGravField);
			if (includeGravField)
			{
				ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
				ImGui::InputFloat("Gravity Field Radius", &capsuleGravityRadius, 1.0f, 0.5f, "%.1f");
				capsuleGravityRadius = GetClamped(capsuleGravityRadius, 0.0f, FLT_MAX);

				CapsulePLTD::RenderPreview(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), capsuleRadius, capsuleLength, Vec3(capsuleDir[0], capsuleDir[1], capsuleDir[2]), capsuleGravityRadius,
					pltdColor);
			}
			else
			{
				CapsulePLTD::RenderPreview(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), capsuleRadius, capsuleLength, Vec3(capsuleDir[0], capsuleDir[1], capsuleDir[2]), 0.0f,
					pltdColor);
			}
		}
		//ellipsoid
		else if (currentPlanetoidShape == 3)
		{
			ImGui::InputFloat3("Dimensions", ellipsoidRadii, "%.1f");
			ellipsoidRadii[0] = GetClamped(ellipsoidRadii[0], 0.0f, FLT_MAX);
			ellipsoidRadii[1] = GetClamped(ellipsoidRadii[1], 0.0f, FLT_MAX);
			ellipsoidRadii[2] = GetClamped(ellipsoidRadii[2], 0.0f, FLT_MAX);
			ImGui::InputFloat3("Orientation", ellipsoidAngle, "%.1f");

			ImGui::NewLine();
			ImGui::Checkbox("Include Gravity Field", &includeGravField);
			if (includeGravField)
			{
				ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
				ImGui::InputFloat3("Gravity Field Dimensions", ellipsoidGravRadii, "%.1f");
				ellipsoidGravRadii[0] = GetClamped(ellipsoidGravRadii[0], 0.0f, FLT_MAX);
				ellipsoidGravRadii[1] = GetClamped(ellipsoidGravRadii[1], 0.0f, FLT_MAX);
				ellipsoidGravRadii[2] = GetClamped(ellipsoidGravRadii[2], 0.0f, FLT_MAX);

				EllipsoidPLTD::RenderPreview(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), ellipsoidRadii[0], ellipsoidRadii[1], ellipsoidRadii[2], EulerAngles(ellipsoidAngle[0], ellipsoidAngle[1],
					ellipsoidAngle[2]), ellipsoidGravRadii[0], ellipsoidGravRadii[1], ellipsoidGravRadii[2], pltdColor);
			}
			else
			{
				EllipsoidPLTD::RenderPreview(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), ellipsoidRadii[0], ellipsoidRadii[1], ellipsoidRadii[2], EulerAngles(ellipsoidAngle[0], ellipsoidAngle[1],
					ellipsoidAngle[2]), 0.0f, 0.0f, 0.0f, pltdColor);
			}
		}
		//rounded cube
		else if (currentPlanetoidShape == 4)
		{
			ImGui::InputFloat3("Dimensions", roundCubeDim, "%.1f");
			roundCubeDim[0] = GetClamped(roundCubeDim[0], 0.0f, FLT_MAX);
			roundCubeDim[1] = GetClamped(roundCubeDim[1], 0.0f, FLT_MAX);
			roundCubeDim[2] = GetClamped(roundCubeDim[2], 0.0f, FLT_MAX);
			ImGui::InputFloat3("Orientation", roundCubeAngle, "%.1f");
			ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
			ImGui::InputFloat("Roundedness", &roundCubeRounded, 0.1f, 0.5f, "%.1f");
			roundCubeRounded = GetClamped(roundCubeRounded, 0.0f, 1.0f);

			ImGui::NewLine();
			ImGui::Checkbox("Include Gravity Field", &includeGravField);
			if (includeGravField)
			{
				ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
				ImGui::InputFloat3("Gravity Field Dimensions", roundCubeGravDim, "%.1f");
				roundCubeGravDim[0] = GetClamped(roundCubeGravDim[0], 0.0f, FLT_MAX);
				roundCubeGravDim[1] = GetClamped(roundCubeGravDim[1], 0.0f, FLT_MAX);
				roundCubeGravDim[2] = GetClamped(roundCubeGravDim[2], 0.0f, FLT_MAX);

				RoundCubePLTD::RenderPreview(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), roundCubeDim[0], roundCubeDim[1], roundCubeDim[2], roundCubeRounded, EulerAngles(roundCubeAngle[0],
					roundCubeAngle[1], roundCubeAngle[2]), roundCubeGravDim[0], roundCubeGravDim[1], roundCubeGravDim[2], pltdColor);
			}
			else
			{
				RoundCubePLTD::RenderPreview(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), roundCubeDim[0], roundCubeDim[1], roundCubeDim[2], roundCubeRounded, EulerAngles(roundCubeAngle[0],
					roundCubeAngle[1], roundCubeAngle[2]), 0.0f, 0.0f, 0.0f, pltdColor);
			}
		}
		//torus
		else if (currentPlanetoidShape == 5)
		{
			ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
			ImGui::InputFloat("Tube Radius", &torusTubeRadius, 1.0f, 0.5f, "%.1f");
			torusTubeRadius = GetClamped(torusTubeRadius, 0.0f, FLT_MAX);
			ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
			ImGui::InputFloat("Hole Radius", &torusHoleRadius, 1.0f, 0.5f, "%.1f");
			torusHoleRadius = GetClamped(torusHoleRadius, 0.0f, FLT_MAX);
			ImGui::InputFloat3("Orientation", torusAngle, "%.1f");

			ImGui::NewLine();
			ImGui::Checkbox("Include Gravity Field", &includeGravField);
			if (includeGravField)
			{
				ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
				ImGui::InputFloat("Gravity Field Radius", &torusGravRadius, 1.0f, 0.5f, "%.1f");
				torusGravRadius = GetClamped(torusGravRadius, 0.0f, FLT_MAX);

				TorusPLTD::RenderPreview(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), torusTubeRadius, torusHoleRadius, EulerAngles(torusAngle[0], torusAngle[1], torusAngle[2]), torusGravRadius,
					pltdColor);
			}
			else
			{
				TorusPLTD::RenderPreview(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), torusTubeRadius, torusHoleRadius, EulerAngles(torusAngle[0], torusAngle[1], torusAngle[2]), 0.0f,
					pltdColor);
			}
		}
		//bowl
		else if (currentPlanetoidShape == 6)
		{
			ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
			ImGui::InputFloat("Radius", &bowlRadius, 1.0f, 0.5f, "%.1f");
			bowlRadius = GetClamped(bowlRadius, 0.0f, FLT_MAX);
			ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
			ImGui::InputFloat("Thickness", &bowlThickness, 0.1f, 0.5f, "%.1f");
			bowlThickness = GetClamped(bowlThickness, 0.0f, bowlRadius);
			ImGui::InputFloat3("Orientation", bowlAngle, "%.1f");

			ImGui::NewLine();
			ImGui::Checkbox("Include Gravity Field", &includeGravField);
			if (includeGravField)
			{
				ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
				ImGui::InputFloat("Gravity Field Radius", &bowlGravRadius, 1.0f, 0.5f, "%.1f");
				bowlGravRadius = GetClamped(bowlGravRadius, 0.0f, FLT_MAX);

				BowlPLTD::RenderPreview(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), bowlRadius, bowlThickness, EulerAngles(bowlAngle[0], bowlAngle[1], bowlAngle[2]), bowlGravRadius, pltdColor);
			}
			else
			{
				BowlPLTD::RenderPreview(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), bowlRadius, bowlThickness, EulerAngles(bowlAngle[0], bowlAngle[1], bowlAngle[2]), 0.0f, pltdColor);
			}
		}
		//mobius strip
		/*else if (currentPlanetoidShape == 7)
		{
			ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
			ImGui::InputFloat("Radius", &mobiusStripRadius, 1.0f, 0.5f, "%.1f");
			mobiusStripRadius = GetClamped(mobiusStripRadius, 0.0f, FLT_MAX);
			ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
			ImGui::InputFloat("Width", &mobiusStripWidth, 1.0f, 0.5f, "%.1f");
			mobiusStripWidth = GetClamped(mobiusStripWidth, 0.0f, FLT_MAX);
			ImGui::InputFloat3("Orientation", mobiusStripAngle, "%.1f");

			ImGui::NewLine();
			ImGui::Checkbox("Include Gravity Field", &includeGravField);
			if (includeGravField)
			{
				ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
				ImGui::InputFloat("Gravity Field Height", &mobiusStripGravHeight, 1.0f, 0.5f, "%.1f");
				mobiusStripGravHeight = GetClamped(mobiusStripGravHeight, 0.0f, FLT_MAX);

				MobiusPLTD::RenderPreview(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), mobiusStripRadius, mobiusStripWidth * 0.5f, EulerAngles(mobiusStripAngle[0], mobiusStripAngle[1], mobiusStripAngle[2]),
					mobiusStripGravHeight, pltdColor);
			}
			else
			{
				MobiusPLTD::RenderPreview(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), mobiusStripRadius, mobiusStripWidth * 0.5f, EulerAngles(mobiusStripAngle[0], mobiusStripAngle[1], mobiusStripAngle[2]),
					0.0f, pltdColor);
			}
		}*/
		//wire
		else if (currentPlanetoidShape == 7)
		{
			ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
			ImGui::InputFloat("Wire Radius", &wireRadius, 1.0f, 0.5f, "%.1f");
			wireRadius = GetClamped(wireRadius, 0.0f, FLT_MAX);
			ImGui::InputFloat3("Orientation", wireAngle, "%.1f");

			//perlin parameters dropdown
			if (ImGui::CollapsingHeader("Perlin Parameters"))
			{
				ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
				ImGui::InputInt("RNG Seed", &wirePerlin.m_rngSeed);
				wirePerlin.m_rngSeed = GetClamped(wirePerlin.m_rngSeed, 0, INT_MAX);
				ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
				ImGui::InputInt2("Min/Max Segment Count", &wirePerlin.m_minSegments);
				wirePerlin.m_minSegments = GetClamped(wirePerlin.m_minSegments, 0, wirePerlin.m_maxSegments);
				wirePerlin.m_maxSegments = GetClamped(wirePerlin.m_maxSegments, wirePerlin.m_minSegments, INT_MAX);
				ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
				ImGui::InputFloat2("Min/Max Segment Length", &wirePerlin.m_segMinLength, "%.1f");
				wirePerlin.m_segMinLength = GetClamped(wirePerlin.m_segMinLength, 0.0f, wirePerlin.m_segMinLength);
				wirePerlin.m_segMaxLength = GetClamped(wirePerlin.m_segMaxLength, wirePerlin.m_segMinLength, FLT_MAX);

				ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
				ImGui::InputFloat("Max Yaw Change", &wirePerlin.m_maxYawChange, 1.0f, 0.5f, "%.1f");
				ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
				ImGui::InputFloat("Yaw Perlin Scale", &wirePerlin.m_perlinScaleYaw, 1.0f, 0.5f, "%.1f");
				ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
				ImGui::InputInt("Yaw Octave Count", &wirePerlin.m_perlinOctavesYaw);
				wirePerlin.m_perlinOctavesYaw = GetClamped(wirePerlin.m_perlinOctavesYaw, 0, INT_MAX);
				ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
				ImGui::InputFloat("Yaw Octave Persistence", &wirePerlin.m_perlinOctavePersistYaw, 1.0f, 0.5f, "%.1f");
				ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
				ImGui::InputFloat("Yaw Octave Scale", &wirePerlin.m_perlinOctaveScaleYaw, 1.0f, 0.5f, "%.1f");

				ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
				ImGui::InputFloat("Max Pitch Change", &wirePerlin.m_maxPitchChange, 1.0f, 0.5f, "%.1f");
				ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
				ImGui::InputFloat("Pitch Perlin Scale", &wirePerlin.m_perlinScalePitch, 1.0f, 0.5f, "%.1f");
				ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
				ImGui::InputInt("Pitch Octave Count", &wirePerlin.m_perlinOctavesPitch);
				wirePerlin.m_perlinOctavesPitch = GetClamped(wirePerlin.m_perlinOctavesPitch, 0, INT_MAX);
				ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
				ImGui::InputFloat("Pitch Octave Persistence", &wirePerlin.m_perlinOctavePersistPitch, 1.0f, 0.5f, "%.1f");
				ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
				ImGui::InputFloat("Pitch Octave Scale", &wirePerlin.m_perlinOctaveScalePitch, 1.0f, 0.5f, "%.1f");
			}

			ImGui::NewLine();
			ImGui::Checkbox("Include Gravity Field", &includeGravField);
			if (includeGravField)
			{
				ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
				ImGui::InputFloat("Gravity Field Radius", &wireGravRadius, 1.0f, 0.5f, "%.1f");
				wireGravRadius = GetClamped(wireGravRadius, 0.0f, FLT_MAX);

				WirePLTD::RenderPreview(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), wireRadius, wirePerlin, EulerAngles(wireAngle[0], wireAngle[1], wireAngle[2]), wireGravRadius, pltdColor);
			}
			else
			{
				WirePLTD::RenderPreview(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), wireRadius, wirePerlin, EulerAngles(wireAngle[0], wireAngle[1], wireAngle[2]), 0.0f, pltdColor);
			}
		}
		//prefab
		else if (currentPlanetoidShape == 8)
		{
			ImGui::Combo("Type", &currentPrefabType, prefabTypes);
			ImGui::InputFloat3("Orientation", prefabAngle, "%.1f");

			ImGui::NewLine();
			ImGui::Checkbox("Include Gravity Field", &includeGravField);
			if (includeGravField)
			{
				if (currentPrefabType == 3)
				{
					ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
					ImGui::InputFloat("Gravity Field Height", &prefabGravScale, 1.0f, 0.5f, "%.1f");
					prefabGravScale = GetClamped(prefabGravScale, 0.0f, FLT_MAX);
				}
				else
				{
					ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
					ImGui::InputFloat("Gravity Field Radius", &prefabGravScale, 1.0f, 0.5f, "%.1f");
					prefabGravScale = GetClamped(prefabGravScale, 0.0f, FLT_MAX);
				}

				Rgba8 previewColor = Rgba8(pltdColor.r, pltdColor.g, pltdColor.b, 127);

				if (currentPrefabType != m_previousModelIndex)
				{
					switch (currentPrefabType)
					{
						case 0:
						{
							m_previewModel = new Model(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), 1.0f, EulerAngles(prefabAngle[0], prefabAngle[1], prefabAngle[2]), previewColor);
							m_previewModel->ParseXMLFileForOBJ("Data/Models/Teapot.xml");

							break;
						}
						case 1:
						{
							m_previewModel = new Model(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), 1.0f, EulerAngles(prefabAngle[0], prefabAngle[1], prefabAngle[2]), previewColor);
							m_previewModel->ParseXMLFileForOBJ("Data/Models/DrumSeparateRocketPlanet.xml");

							break;
						}
						case 2:
						{
							m_previewModel = new Model(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), 1.0f, EulerAngles(prefabAngle[0], prefabAngle[1], prefabAngle[2]), previewColor);
							m_previewModel->ParseXMLFileForOBJ("Data/Models/MountainPlanet.xml");

							break;
						}
						case 3:
						{
							m_previewModel = new Model(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), 1.0f, EulerAngles(prefabAngle[0], prefabAngle[1], prefabAngle[2]), previewColor);
							m_previewModel->ParseXMLFileForOBJ("Data/Models/OldFortressPlanet.xml");

							break;
						}
					}

					m_previousModelIndex = currentPrefabType;
				}

				PrefabPLTD::RenderPreview(m_previewModel, Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), EulerAngles(prefabAngle[0], prefabAngle[1], prefabAngle[2]), previewColor, true, currentPrefabType, prefabGravScale);
			}
			else
			{
				Rgba8 previewColor = Rgba8(pltdColor.r, pltdColor.g, pltdColor.b, 127);

				if (currentPrefabType != m_previousModelIndex)
				{
					switch (currentPrefabType)
					{
						case 0:
						{
							m_previewModel = new Model(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), 1.0f, EulerAngles(prefabAngle[0], prefabAngle[1], prefabAngle[2]), previewColor);
							m_previewModel->ParseXMLFileForOBJ("Data/Models/Teapot.xml");

							break;
						}
						case 1:
						{
							m_previewModel = new Model(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), 1.0f, EulerAngles(prefabAngle[0], prefabAngle[1], prefabAngle[2]), previewColor);
							m_previewModel->ParseXMLFileForOBJ("Data/Models/DrumSeparateRocketPlanet.xml");

							break;
						}
						case 2:
						{
							m_previewModel = new Model(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), 1.0f, EulerAngles(prefabAngle[0], prefabAngle[1], prefabAngle[2]), previewColor);
							m_previewModel->ParseXMLFileForOBJ("Data/Models/MountainPlanet.xml");

							break;
						}
						case 3:
						{
							m_previewModel = new Model(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), 1.0f, EulerAngles(prefabAngle[0], prefabAngle[1], prefabAngle[2]), previewColor);
							m_previewModel->ParseXMLFileForOBJ("Data/Models/OldFortressPlanet.xml");

							break;
						}
					}

					m_previousModelIndex = currentPrefabType;
				}

				PrefabPLTD::RenderPreview(m_previewModel, Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), EulerAngles(prefabAngle[0], prefabAngle[1], prefabAngle[2]), previewColor, false, currentPrefabType, prefabGravScale);
			}
		}
		if (includeGravField)
		{
			ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
			ImGui::InputFloat("Gravity Field Strength", &pltdGravityForce, 10.0f, 0.5f, "%.1f");
		}
		
		ImGui::NewLine();
		ImGuiColorEditFlags colorFlags = 1048576;
		ImGui::ColorPicker3("Planetoid Color", pltdSpawnColor, colorFlags);
		pltdColor.SetFromFloats(pltdSpawnColor);

		ImGui::NewLine();
		if (ImGui::Button("Spawn Planetoid"))
		{
			//plane
			if (currentPlanetoidShape == 0)
			{
				SpawnPlane(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), planeHalfLength, planeHalfWidth, EulerAngles(planeAngle[0], planeAngle[1], planeAngle[2]), includeGravField, planeGravityHeight, 
					pltdGravityForce, pltdColor);
			}
			//sphere
			else if(currentPlanetoidShape == 1)
			{
				SpawnSphere(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), spherePltdRadius, includeGravField, sphereGravityRadius, pltdGravityForce, pltdColor);
			}
			//capsule
			else if (currentPlanetoidShape == 2)
			{
				SpawnCapsule(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), capsuleRadius, capsuleLength, Vec3(capsuleDir[0], capsuleDir[1], capsuleDir[2]), includeGravField, capsuleGravityRadius,
					pltdGravityForce, pltdColor);
			}
			//ellipsoid
			else if (currentPlanetoidShape == 3)
			{
				SpawnEllipsoid(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), ellipsoidRadii[0], ellipsoidRadii[1], ellipsoidRadii[2], EulerAngles(ellipsoidAngle[0], ellipsoidAngle[1], 
					ellipsoidAngle[2]), includeGravField, ellipsoidGravRadii[0], ellipsoidGravRadii[1], ellipsoidGravRadii[2], pltdGravityForce, pltdColor);
			}
			//rounded cube
			else if (currentPlanetoidShape == 4)
			{
				SpawnRoundedCube(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), roundCubeDim[0], roundCubeDim[1], roundCubeDim[2], roundCubeRounded, EulerAngles(roundCubeAngle[0], 
					roundCubeAngle[1], roundCubeAngle[2]), includeGravField, roundCubeGravDim[0], roundCubeGravDim[1], roundCubeGravDim[2], pltdGravityForce, pltdColor);
			}
			//torus
			else if (currentPlanetoidShape == 5)
			{
				SpawnTorus(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), torusTubeRadius, torusHoleRadius, EulerAngles(torusAngle[0], torusAngle[1], torusAngle[2]), includeGravField, torusGravRadius,
					pltdGravityForce, pltdColor);
			}
			//bowl
			else if (currentPlanetoidShape == 6)
			{
				SpawnBowl(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), bowlRadius, bowlThickness, EulerAngles(bowlAngle[0], bowlAngle[1], bowlAngle[2]), includeGravField, bowlGravRadius, pltdGravityForce, pltdColor);
			}
			//mobius strip
			/*else if (currentPlanetoidShape == 7)
			{
				SpawnMobiusStrip(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), mobiusStripRadius, mobiusStripWidth, EulerAngles(mobiusStripAngle[0], mobiusStripAngle[1], mobiusStripAngle[2]),
					mobiusStripGravHeight, includeGravField, pltdGravityForce, pltdColor);
			}*/
			//wire
			else if (currentPlanetoidShape == 7)
			{
				SpawnWire(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), wireRadius, wirePerlin, EulerAngles(wireAngle[0], wireAngle[1], wireAngle[2]), includeGravField, wireGravRadius, pltdGravityForce,
					pltdColor);
			}
			//prefab
			else if (currentPlanetoidShape == 8)
			{
				//teapot
				if (currentPrefabType == 0)
				{
					SpawnTeapot(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), 1.0f, EulerAngles(prefabAngle[0], prefabAngle[1], prefabAngle[2]), includeGravField, prefabGravScale, pltdGravityForce, pltdColor);
				}
				//sky station
				else if (currentPrefabType == 1)
				{
					SpawnSkyStation(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), 1.0f, EulerAngles(prefabAngle[0], prefabAngle[1], prefabAngle[2]), includeGravField, prefabGravScale, pltdGravityForce, pltdColor);
				}
				//mountain
				else if (currentPrefabType == 2)
				{
					SpawnMountain(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), 1.0f, EulerAngles(prefabAngle[0], prefabAngle[1], prefabAngle[2]), includeGravField, prefabGravScale, pltdGravityForce, pltdColor);
				}
				//fortress
				else if (currentPrefabType == 3)
				{
					SpawnFortress(Vec3(pltdPos[0], pltdPos[1], pltdPos[2]), 1.0f, EulerAngles(prefabAngle[0], prefabAngle[1], prefabAngle[2]), includeGravField, prefabGravScale, pltdGravityForce, pltdColor);
				}
			}
		}

		ImGui::NewLine();
		ImGui::NewLine();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
		if (ImGui::Button("Clear All Planetoids"))
		{
			ClearAllPlanetoids();
		}
		ImGui::PopStyleColor();

		ImGui::End();
	}

	if (m_lightingMenuOpen)
	{
		ImGui::SetNextWindowPos(ImVec2(1010.0f, 20.0f));
		ImGui::SetNextWindowSize(ImVec2(350.0f, 650.0f));
		ImGui::Begin("Lighting Menu");

		float skyColorFlt[4] = {};
		m_skyColor.GetAsFloats(skyColorFlt);
		ImGuiColorEditFlags colorFlags = 1048576;
		ImGui::ColorPicker3("Sky Color", skyColorFlt, colorFlags);
		m_skyColor.SetFromFloats(skyColorFlt);

		ImGui::NewLine();
		ImGui::InputFloat3("Sun Direction", &m_sunDirection.x, "%.1f");
		m_sunDirection.x = GetClamped(m_sunDirection.x, -1.0f, 1.0f);
		m_sunDirection.y = GetClamped(m_sunDirection.y, -1.0f, 1.0f);
		m_sunDirection.z = GetClamped(m_sunDirection.z, -1.0f, 1.0f);
		ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
		ImGui::InputFloat("Sun Intensity", &m_sunIntensity, 0.1f, 0.5f, " %.1f");
		m_sunIntensity = GetClamped(m_sunIntensity, 0.0f, 1.0f);
		ImGui::SetNextItemWidth(FLOAT_BOX_WIDTH);
		ImGui::InputFloat("Ambient Intensity", &m_ambientIntensity, 0.1f, 0.5f, " %.1f");
		m_ambientIntensity = GetClamped(m_ambientIntensity, 0.0f, 1.0f);

		ImGui::End();
	}
}


void Game::Shutdown()
{
	//export results of playtesting to file
	std::vector<uint8_t> outBuffer;
	BufferWriter bw = BufferWriter(outBuffer);
	std::string outString = Stringf("Total Course Time: %.2f\n", m_section1Duration + m_section2Duration + m_section3Duration + m_section4Duration);

	outString += Stringf("Section 1 Start Time: %.2f  -  Duration: %.2f\n", m_section1StartTime, m_section1Duration);
	for (int respawnIndex = 0; respawnIndex < m_section1RespawnTimes.size(); respawnIndex++)
	{
		outString += Stringf(" Section 1 Respawn at: %.2f\n", m_section1RespawnTimes[respawnIndex]);
	}
	outString += "\n";
	outString += Stringf("Section 2 Start Time: %.2f  -  Duration: %.2f\n", m_section2StartTime, m_section2Duration);
	for (int respawnIndex = 0; respawnIndex < m_section2RespawnTimes.size(); respawnIndex++)
	{
		outString += Stringf(" Section 2 Respawn at: %.2f", m_section2RespawnTimes[respawnIndex]);
	}
	outString += "\n";
	outString += Stringf("Section 3 Start Time: %.2f  -  Duration: %.2f\n", m_section3StartTime, m_section3Duration);
	for (int respawnIndex = 0; respawnIndex < m_section3RespawnTimes.size(); respawnIndex++)
	{
		outString += Stringf(" Section 3 Respawn at: %.2f\n", m_section3RespawnTimes[respawnIndex]);
	}
	outString += "\n";
	outString += Stringf("Section 4 Start Time: %.2f  -  Duration: %.2f\n", m_section4StartTime, m_section4Duration);
	for (int respawnIndex = 0; respawnIndex < m_section4RespawnTimes.size(); respawnIndex++)
	{
		outString += Stringf(" Section 4 Respawn at: %.2f\n", m_section4RespawnTimes[respawnIndex]);
	}
	outString += "\n";

	outString += Stringf("Num Standard Jumps: %i\n", m_standardJumpTimes.size());
	outString += Stringf("Num Double Jumps: %i\n", m_doubleJumpTimes.size());
	outString += Stringf("Num Triple Jumps: %i\n", m_tripleJumpTimes.size());
	outString += Stringf("Num Long Jumps: %i\n", m_longJumpTimes.size());
	outString += Stringf("Num Wall Jumps: %i\n", m_wallJumpTimes.size());
	outString += Stringf("Num Back Flips: %i\n", m_backFlipTimes.size());
	outString += Stringf("Num Side Flips: %i\n", m_sideFlipTimes.size());
	outString += "\n";

	outString += "Standard Jump Times: \n";
	for (int jumpIndex = 0; jumpIndex < m_standardJumpTimes.size(); jumpIndex++)
	{
		outString += Stringf(" %.2f", m_standardJumpTimes[jumpIndex]);
	}
	outString += "\n";
	outString += "Double Jump Times: \n";
	for (int jumpIndex = 0; jumpIndex < m_doubleJumpTimes.size(); jumpIndex++)
	{
		outString += Stringf(" %.2f", m_doubleJumpTimes[jumpIndex]);
	}
	outString += "\n";
	outString += "Triple Jump Times: \n";
	for (int jumpIndex = 0; jumpIndex < m_tripleJumpTimes.size(); jumpIndex++)
	{
		outString += Stringf(" %.2f", m_tripleJumpTimes[jumpIndex]);
	}
	outString += "\n";
	outString += "Long Jump Times: \n";
	for (int jumpIndex = 0; jumpIndex < m_longJumpTimes.size(); jumpIndex++)
	{
		outString += Stringf(" %.2f", m_longJumpTimes[jumpIndex]);
	}
	outString += "\n";
	outString += "Wall Jump Times: \n";
	for (int jumpIndex = 0; jumpIndex < m_wallJumpTimes.size(); jumpIndex++)
	{
		outString += Stringf(" %.2f", m_wallJumpTimes[jumpIndex]);
	}
	outString += "\n";
	outString += "Back Flip Times: \n";
	for (int jumpIndex = 0; jumpIndex < m_backFlipTimes.size(); jumpIndex++)
	{
		outString += Stringf(" %.2f", m_backFlipTimes[jumpIndex]);
	}
	outString += "\n";
	outString += "Side Flip Times: \n";
	for (int jumpIndex = 0; jumpIndex < m_sideFlipTimes.size(); jumpIndex++)
	{
		outString += Stringf(" %.2f", m_sideFlipTimes[jumpIndex]);
	}
	outString += "\n";

	for (int charIndex = 0; charIndex < outString.length(); charIndex++)
	{
		bw.AppendChar(outString[charIndex]);
	}

	std::string fileName = "Data/Exported/RenameThisAfterTest.txt";
	FileWriteFromBuffer(outBuffer, fileName);

	//delete planetoids
	for (int pltdIndex = 0; pltdIndex < m_planetoids.size(); pltdIndex++)
	{
		if (m_planetoids[pltdIndex] != nullptr)
		{
			delete m_planetoids[pltdIndex];
			m_planetoids[pltdIndex] = nullptr;
		}
	}

	if (m_previewModel != nullptr)
	{
		delete m_previewModel;
		m_previewModel = nullptr;
	}
	
	if (m_player != nullptr)
	{
		delete m_player;
		m_player = nullptr;
	}
}


//
//planetoid spawning functions
//
void Game::ClearAllPlanetoids()
{
	for (int pltdIndex = 0; pltdIndex < m_planetoids.size(); pltdIndex++)
	{
		if (m_planetoids[pltdIndex] != nullptr)
		{
			delete m_planetoids[pltdIndex];
			m_planetoids[pltdIndex] = nullptr;
		}
	}
}


void Game::AddPlanetoidsForPlaytestingCourse()
{
	//add planetoids for starting area before course
	SpawnSphere(Vec3(0.0f, 0.0f, -26.0f), 20.0f, true, 30.0f, GRAVITY_STANDARD, Rgba8(50, 150, 100));
	SpawnTorus(Vec3(50.0f, 0.0f, 0.0f), 9.0f, 22.0f, EulerAngles(), true, 12.0f, GRAVITY_STANDARD, Rgba8(255, 255, 0));
	
	SpawnCapsule(Vec3(50.0f, 47.5f, 0.0f), 5.0f, 17.5f, Vec3(0.0f, 1.0f, 0.0f), true, 12.5f, GRAVITY_STANDARD, Rgba8(0, 175, 25));
	SpawnMountain(Vec3(50.0f, 95.0f, -4.0f), 1.0f, EulerAngles(), true, 45.0f, GRAVITY_STANDARD, Rgba8(25, 150, 220));

	SpawnCapsule(Vec3(50.0f, -47.5f, 0.0f), 5.0f, 17.5f, Vec3(0.0f, -1.0f, 0.0f), true, 12.5f, GRAVITY_STANDARD, Rgba8(25, 0, 175));
	SpawnRoundedCube(Vec3(55.0f, -75.0f, 7.0f), 8.0f, 9.0f, 6.0f, 0.75f, EulerAngles(-20.0f, 0.0f, 0.0f), true, 18.0f, 19.0f, 16.0f, GRAVITY_STANDARD, Rgba8(200, 200, 200));
	SpawnRoundedCube(Vec3(65.0f, -80.0f, 14.0f), 9.0f, 8.0f, 6.0f, 0.5f, EulerAngles(15.0f, 15.0f, 15.0f), true, 19.0f, 18.0f, 16.0f, GRAVITY_STANDARD, Rgba8(150, 150, 150));
	SpawnRoundedCube(Vec3(76.0f, -76.0f, 19.0f), 10.0f, 9.0f, 12.0f, 0.25f, EulerAngles(0.0f, -5.0f, 45.0f), true, 20.0f, 19.0f, 22.0f, GRAVITY_STANDARD, Rgba8(100, 100, 100));

	SpawnCapsule(Vec3(94.0f, 0.0f, 0.0f), 5.0f, 20.0f, Vec3(1.0f, 0.0f, 0.0f), true, 12.5f, GRAVITY_STANDARD, Rgba8(100, 0, 100));
	Mat44 textTransform = Mat44::CreateTranslation3D(Vec3(114.0f, 0.0f, 10.0f));
	textTransform.AppendZRotation(180.0f);
	DebugAddWorldText("Enter Playtest Course Here", textTransform, 0.5f, Vec2(0.5f, 0.5f), -1.0f);
	
	//Section 1
	m_checkpoints.emplace_back(AABB3(Vec3(247.0f, -5.0f, 0.0f), Vec3(253.0f, 5.0f, 15.0f)));
	textTransform = Mat44::CreateTranslation3D(Vec3(253.0f, 0.0f, 15.0f));
	textTransform.AppendZRotation(180.0f);
	DebugAddWorldText("Section 1", textTransform, 0.5f, Vec2(0.5f, 0.5f), -1.0f);

	SpawnSphere(Vec3(250.0f, 0.0f, 0.0f), 10.0f, true, 20.0f, GRAVITY_STANDARD, Rgba8(50, 100, 200));
	SpawnSphere(Vec3(257.0f, -6.0f, 5.0f), 8.0f, true, 16.0f, GRAVITY_STANDARD, Rgba8(200, 100, 50));
	SpawnSphere(Vec3(265.0f, 6.0f, 11.0f), 11.0f, true, 22.0f, GRAVITY_STANDARD, Rgba8(100, 200, 50));
	SpawnSphere(Vec3(290.0f, -2.0f, 18.0f), 10.0f, true, 20.0f, GRAVITY_STANDARD, Rgba8(100, 50, 100));
	SpawnEllipsoid(Vec3(305.0f, 0.0f, 30.0f), 6.0f, 6.0f, 3.5f, EulerAngles(), true, 24.0f, 24.0f, 14.0f, GRAVITY_STANDARD, Rgba8(150, 255, 0));
	SpawnCapsule(Vec3(320.0f, 0.0f, 35.0f), 3.5f, 15.0f, Vec3(1.0f, 1.0f, 0.0f), true, 16.0f, GRAVITY_STANDARD, Rgba8(200, 75, 150));
	SpawnCapsule(Vec3(340.0f, 10.0f, 35.0f), 3.5f, 15.0f, Vec3(1.0f, -1.0f, 0.0f), true, 16.0f, GRAVITY_STANDARD, Rgba8(125, 75, 175));

	SpawnMountain(Vec3(377.0f, 0.0f, 35.0f), 1.0f, EulerAngles(315.0f, 0.0f, 0.0f), true, 45.0f, GRAVITY_STANDARD, Rgba8(0, 200, 10));
	SpawnSphere(Vec3(379.0f, 0.0f, 90.0f), 10.0f, true, 20.0f, GRAVITY_STANDARD, Rgba8(50, 255, 255));
	SpawnBowl(Vec3(408.0f, 0.0f, 90.0f), 10.0f, 3.5f, EulerAngles(45.0f, 30.0f, 10.0f), true, 10.0f, GRAVITY_STANDARD, Rgba8(255, 0, 250));
	WirePerlinParameters wirePerlin;
	wirePerlin.m_minSegments = 5;
	wirePerlin.m_maxSegments = 5;
	wirePerlin.m_maxYawChange = 90.0f;
	wirePerlin.m_maxPitchChange = 45.0f;
	wirePerlin.m_segMinLength = 10.0f;
	wirePerlin.m_segMaxLength = 25.0f;
	wirePerlin.m_rngSeed = 1234;
	SpawnWire(Vec3(427.0f, 0.0f, 87.5f), 5.0f, wirePerlin, EulerAngles(), true, 10.0f, GRAVITY_STANDARD, Rgba8(187, 108, 156));
	SpawnRoundedCube(Vec3(496.0f, 85.0f, 100.0f), 20.0f, 20.0f, 25.0f, 0.5f, EulerAngles(), true, 40.0f, 40.0f, 45.0f, GRAVITY_STANDARD, Rgba8(50, 160, 142));

	//Section 2
	m_checkpoints.emplace_back(AABB3(Vec3(511.0f, 55.0f, 102.0f), Vec3(561.0f, 115.0f, 132.0f)));
	textTransform = Mat44::CreateTranslation3D(Vec3(516.0f, 85.0f, 115.0f));
	textTransform.AppendZRotation(180.0f);
	DebugAddWorldText("Section 2", textTransform, 0.5f, Vec2(0.5f, 0.5f), -1.0f);
	SpawnRoundedCube(Vec3(536.0f, 85.0f, 102.0f), 50.0f, 50.0f, 15.0f, 0.25f, EulerAngles(), true, 75.0f, 75.0f, 40.0f, GRAVITY_STANDARD, Rgba8(50, 100, 50));
	textTransform = Mat44::CreateTranslation3D(Vec3(536.0f, 85.0f, 115.0f));
	textTransform.AppendZRotation(180.0f);
	DebugAddWorldText("Jump against a wall to wall slide\n\nJump while sliding to wall jump", textTransform, 0.5f, Vec2(0.5f, 0.5f), -1.0f);
	SpawnPlane(Vec3(545.0f, 88.0f, 120.0f), 6.0f, 11.0f, EulerAngles(0.0f, 0.0f, 90.0f), false, 0.0f, 0.0f, Rgba8(200, 0, 0));
	SpawnPlane(Vec3(545.0f, 88.1f, 120.0f), 6.0f, 11.0f, EulerAngles(0.0f, 0.0f, -90.0f), false, 0.0f, 0.0f, Rgba8(200, 0, 0));
	SpawnPlane(Vec3(545.0f, 82.0f, 120.0f), 6.0f, 11.0f, EulerAngles(0.0f, 0.0f, -90.0f), false, 0.0f, 0.0f, Rgba8(200, 0, 0));
	SpawnPlane(Vec3(545.0f, 81.9f, 120.0f), 6.0f, 11.0f, EulerAngles(0.0f, 0.0f, 90.0f), false, 0.0f, 0.0f, Rgba8(200, 0, 0));
	SpawnSphere(Vec3(545.0f, 85.0f, 152.0f), 10.0f, true, 20.0f, GRAVITY_STANDARD, Rgba8(100, 50, 50));
	SpawnSphere(Vec3(572.0f, 85.0f, 157.0f), 10.0f, true, 20.0f, GRAVITY_STANDARD, Rgba8(50, 50, 100));
	SpawnSphere(Vec3(588.0f, 85.0f, 155.0f), 8.0f, true, 18.0f, GRAVITY_STANDARD, Rgba8(105, 110, 125));

	SpawnRoundedCube(Vec3(675.0f, 85.0f, 150.0f), 160.0f, 100.0f, 10.0f, 0.01f, EulerAngles(), true, 300.0f, 200.0f, 200.0f, GRAVITY_STANDARD, Rgba8(109, 179, 240));
	textTransform = Mat44::CreateTranslation3D(Vec3(625.0f, 85.0f, 162.0f));
	textTransform.AppendZRotation(180.0f);
	DebugAddWorldText("Hold left trigger to crouch\n\nCrouch and jump while not moving to back flip", textTransform, 0.5f, Vec2(0.5f, 0.5f), -1.0f);
	SpawnRoundedCube(Vec3(635.0f, 74.0f, 160.0f), 8.0f, 3.0f, 10.0f, 0.0f, EulerAngles(), false, 0.0f, 0.0f, 0.0f, 0.0f, Rgba8(200, 0, 0));
	SpawnRoundedCube(Vec3(635.0f, 77.0f, 170.0f), 8.0f, 3.0f, 10.0f, 0.0f, EulerAngles(), false, 0.0f, 0.0f, 0.0f, 0.0f, Rgba8(200, 0, 0));
	SpawnRoundedCube(Vec3(640.0f, 85.0f, 180.0f), 16.0f, 7.5f, 5.0f, 0.0f, EulerAngles(), false, 0.0f, 0.0f, 0.0f, 0.0f, Rgba8(200, 0, 0));
	textTransform = Mat44::CreateTranslation3D(Vec3(665.0f, 85.0f, 186.0f));
	textTransform.AppendZRotation(180.0f);
	DebugAddWorldText("Run, crouch, and jump to long jump", textTransform, 0.5f, Vec2(0.5f, 0.5f), -1.0f);
	SpawnRoundedCube(Vec3(670.0f, 85.0f, 180.0f), 16.0f, 10.0f, 5.0f, 0.0f, EulerAngles(), false, 0.0f, 0.0f, 0.0f, 0.0f, Rgba8(200, 0, 0));
	SpawnRoundedCube(Vec3(686.0f, 85.0f, 185.0f), 16.0f, 10.0f, 5.0f, 0.0f, EulerAngles(), false, 0.0f, 0.0f, 0.0f, 0.0f, Rgba8(200, 0, 0));
	SpawnSkyStation(Vec3(730.0f, 85.0f, 189.0f), 1.0f, EulerAngles(-90.0f, -30.0f, 0.0f), true, 45.0f, GRAVITY_STANDARD, Rgba8(0, 255, 125));
	SpawnSphere(Vec3(745.0f, 85.0f, 225.0f), 10.0f, true, 25.0f, GRAVITY_STANDARD, Rgba8(194, 190, 5));

	//Section 3
	m_checkpoints.emplace_back(AABB3(Vec3(760.0f, 55.0f, 200.0f), Vec3(795.0f, 115.0f, 285.0f)));
	textTransform = Mat44::CreateTranslation3D(Vec3(773.0f, 85.0f, 240.0f));
	textTransform.AppendZRotation(180.0f);
	DebugAddWorldText("Section 3", textTransform, 0.5f, Vec2(0.5f, 0.5f), -1.0f);
	SpawnCapsule(Vec3(770.0f, 85.0f, 225.0f), 10.0f, 20.0f, Vec3(1.0f, 0.0f, 0.0f), true, 25.0f, GRAVITY_STANDARD, Rgba8(30, 20, 250));
	SpawnFortress(Vec3(870.0f, 84.0f, 225.0f), 1.0f, EulerAngles(225.0f, 0.0f, 0.0f), true, 40.0f, GRAVITY_STANDARD, Rgba8(70, 215, 0));
	SpawnPlane(Vec3(865.0f, 84.0f, 256.25f), 4.0f, 4.0f, EulerAngles(), false, 0.0f, 0.0f, Rgba8(70, 215, 0));
	SpawnRoundedCube(Vec3(874.0f, 80.0f, 269.0f), 6.0f, 2.0f, 20.0f, 0.0f, EulerAngles(), false, 0.0f, 0.0f, 0.0f, 0.0f, Rgba8(200, 0, 0));
	SpawnRoundedCube(Vec3(874.0f, 86.0f, 273.0f), 6.0f, 2.0f, 20.0f, 0.0f, EulerAngles(), false, 0.0f, 0.0f, 0.0f, 0.0f, Rgba8(200, 0, 0));
	SpawnRoundedCube(Vec3(874.0f, 70.0f, 277.5f), 10.0f, 18.0f, 3.0f, 0.0f, EulerAngles(), false, 0.0f, 0.0f, 0.0f, 0.0f, Rgba8(200, 0, 0));
	SpawnSphere(Vec3(874.0f, 0.0f, 282.5f), 12.0f, true, 60.0f, GRAVITY_STANDARD, Rgba8(255, 255, 0));
	SpawnBowl(Vec3(874.0f, 0.0f, 282.5f), 20.0f, 8.0f, EulerAngles(90.0f, 85.0f, 0.0f), false, 0.0f, 0.0f, Rgba8(255, 255, 0));
	SpawnCapsule(Vec3(874.0f, -50.0f, 284.0f), 12.0f, 100.0f, Vec3(0.0f, -1.0f, 0.0f), true, 50.0f, GRAVITY_STANDARD, Rgba8(140, 120, 190));
	SpawnRoundedCube(Vec3(885.0f, -57.5f, 284.0f), 30.0f, 3.0f, 50.0f, 0.0f, EulerAngles(), false, 0.0f, 0.0f, 0.0f, 0.0f, Rgba8(200, 0, 0));
	SpawnRoundedCube(Vec3(865.0f, -82.5f, 284.0f), 30.0f, 3.0f, 50.0f, 0.0f, EulerAngles(), false, 0.0f, 0.0f, 0.0f, 0.0f, Rgba8(200, 0, 0));
	SpawnRoundedCube(Vec3(874.0f, -107.5f, 300.0f), 50.0f, 3.0f, 30.0f, 0.0f, EulerAngles(), false, 0.0f, 0.0f, 0.0f, 0.0f, Rgba8(200, 0, 0));
	SpawnCapsule(Vec3(874.0f, -150.0f, 284.0f), 12.0f, 60.0f, Vec3(0.0f, 0.0f, 1.0f), true, 50.0f, GRAVITY_STANDARD, Rgba8(140, 120, 190));
	SpawnCapsule(Vec3(874.0f, -150.0f, 344.0f), 12.0f, 60.0f, Vec3(0.0f, 1.0f, 1.0f), true, 50.0f, GRAVITY_STANDARD, Rgba8(140, 120, 190));
	
	//Section 4
	m_checkpoints.emplace_back(AABB3(Vec3(880.0f, -148.0f, 386.0f), Vec3(920.0f, -68.0f, 419.0f)));
	textTransform = Mat44::CreateTranslation3D(Vec3(900.0f, -108.0f, 401.0f));
	textTransform.AppendZRotation(180.0f);
	DebugAddWorldText("Section 4", textTransform, 0.5f, Vec2(0.5f, 0.5f), -1.0f);
	textTransform = Mat44::CreateTranslation3D(Vec3(915.0f, -108.0f, 401.0f));
	textTransform.AppendZRotation(180.0f);
	DebugAddWorldText("Jump three times in a row while running\nto triple jump", textTransform, 0.5f, Vec2(0.5f, 0.5f), -1.0f);
	SpawnRoundedCube(Vec3(912.0f, -108.0f, 386.0f), 45.0f, 30.0f, 20.0f, 0.25f, EulerAngles(), true, 45.0f, 50.0f, 40.0f, GRAVITY_STANDARD, Rgba8(255, 170, 150));
	SpawnRoundedCube(Vec3(948.0f, -108.0f, 421.0f), 25.0f, 30.0f, 15.0f, 0.25f, EulerAngles(), true, 45.0f, 50.0f, 40.0f, GRAVITY_STANDARD, Rgba8(190, 170, 180));
	SpawnTorus(Vec3(1045.0f, -108.0f, 450.0f), 12.0f, 50.0f, EulerAngles(0.0f, -25.0f, 0.0f), true, 42.0f, GRAVITY_STANDARD, Rgba8(60, 54, 206));
	SpawnRoundedCube(Vec3(1045.0f, -46.0f, 450.0f), 3.0f, 60.0f, 60.0f, 0.0f, EulerAngles(0.0f, -25.0f, 0.0f), false, 0.0f, 0.0f, 0.0f, 0.0f, Rgba8(200, 0, 0));
	SpawnRoundedCube(Vec3(1045.0f, -170.0f, 450.0f), 3.0f, 60.0f, 60.0f, 0.0f, EulerAngles(0.0f, -25.0f, 0.0f), false, 0.0f, 0.0f, 0.0f, 0.0f, Rgba8(200, 0, 0));
	SpawnSphere(Vec3(1045.0f, -108.0f, 450.0f), 29.0f, true, 50.0f, GRAVITY_STANDARD, Rgba8(106, 187, 156));
	SpawnSphere(Vec3(1140.0f, -108.0f, 490.0f), 10.0f, true, 40.0f, GRAVITY_STANDARD, Rgba8(255, 0, 0));
	SpawnSphere(Vec3(1145.0f, -113.0f, 495.0f), 10.0f, true, 40.0f, GRAVITY_STANDARD, Rgba8(255, 85, 0));
	SpawnSphere(Vec3(1150.0f, -116.0f, 502.0f), 10.0f, true, 40.0f, GRAVITY_STANDARD, Rgba8(255, 170, 0));
	SpawnSphere(Vec3(1155.0f, -118.0f, 510.0f), 10.0f, true, 40.0f, GRAVITY_STANDARD, Rgba8(255, 255, 0));
	SpawnSphere(Vec3(1160.0f, -119.0f, 519.0f), 10.0f, true, 40.0f, GRAVITY_STANDARD, Rgba8(170, 255, 0));
	SpawnSphere(Vec3(1165.0f, -118.0f, 528.0f), 10.0f, true, 40.0f, GRAVITY_STANDARD, Rgba8(85, 255, 0));
	SpawnSphere(Vec3(1170.0f, -116.0f, 536.0f), 10.0f, true, 40.0f, GRAVITY_STANDARD, Rgba8(0, 255, 0));
	SpawnSphere(Vec3(1175.0f, -113.0f, 543.0f), 10.0f, true, 40.0f, GRAVITY_STANDARD, Rgba8(0, 255, 85));
	SpawnSphere(Vec3(1180.0f, -108.0f, 548.0f), 10.0f, true, 40.0f, GRAVITY_STANDARD, Rgba8(0, 255, 170));
	SpawnSphere(Vec3(1185.0f, -103.0f, 553.0f), 10.0f, true, 40.0f, GRAVITY_STANDARD, Rgba8(0, 255, 255));
	SpawnSphere(Vec3(1190.0f, -100.0f, 560.0f), 10.0f, true, 40.0f, GRAVITY_STANDARD, Rgba8(0, 170, 255));
	SpawnSphere(Vec3(1195.0f, -98.0f, 568.0f), 10.0f, true, 40.0f, GRAVITY_STANDARD, Rgba8(0, 85, 255));
	SpawnSphere(Vec3(1200.0f, -97.0f, 577.0f), 10.0f, true, 40.0f, GRAVITY_STANDARD, Rgba8(0, 0, 255));
	SpawnSphere(Vec3(1205.0f, -98.0f, 586.0f), 10.0f, true, 40.0f, GRAVITY_STANDARD, Rgba8(85, 0, 255));
	SpawnSphere(Vec3(1210.0f, -100.0f, 594.0f), 10.0f, true, 40.0f, GRAVITY_STANDARD, Rgba8(170, 0, 255));
	SpawnSphere(Vec3(1215.0f, -103.0f, 601.0f), 10.0f, true, 40.0f, GRAVITY_STANDARD, Rgba8(255, 0, 255));
	SpawnSphere(Vec3(1220.0f, -108.0f, 606.0f), 10.0f, true, 40.0f, GRAVITY_STANDARD, Rgba8(255, 0, 170));
	SpawnSphere(Vec3(1225.0f, -108.0f, 611.0f), 10.0f, true, 40.0f, GRAVITY_STANDARD, Rgba8(255, 0, 85));
	SpawnSphere(Vec3(1325.0f, -108.0f, 711.0f), 110.0f, true, 160.0f, GRAVITY_STANDARD, Rgba8());
	textTransform = Mat44::CreateTranslation3D(Vec3(1325.0f, -108.0f, 826.0f));
	textTransform.AppendZRotation(180.0f);
	DebugAddWorldText("You made it to the end! Congrats!", textTransform, 0.5f, Vec2(0.5f, 0.5f), -1.0f);
}


PlanePLTD* Game::SpawnPlane(Vec3 position, float halfLength, float halfWidth, EulerAngles orientation, bool includeField, float gravityHeight, float gravityForce, Rgba8 color)
{
	PlanePLTD* plane = new PlanePLTD(position, halfLength, halfWidth, orientation, includeField, gravityHeight, gravityForce, color);
	m_planetoids.emplace_back(plane);
	return plane;
}


SpherePLTD* Game::SpawnSphere(Vec3 position, float radius, bool includeField, float gravityRadius, float gravityForce, Rgba8 color)
{
	SpherePLTD* sphere = new SpherePLTD(position, radius, includeField, gravityRadius, gravityForce, color);
	m_planetoids.emplace_back(sphere);
	return sphere;
}


CapsulePLTD* Game::SpawnCapsule(Vec3 position, float radius, float boneLength, Vec3 boneDirection, bool includeField, float gravityRadius, float gravityForce, Rgba8 color)
{
	CapsulePLTD* capsule = new CapsulePLTD(position, radius, boneLength, boneDirection, includeField, gravityRadius, gravityForce, color);
	m_planetoids.emplace_back(capsule);
	return capsule;
}


EllipsoidPLTD* Game::SpawnEllipsoid(Vec3 position, float xRadius, float yRadius, float zRadius, EulerAngles orientation, bool includeField, float gravityXRadius, float gravityYRadius, float gravityZRadius, float gravityForce, Rgba8 color)
{
	EllipsoidPLTD* ellipsoid = new EllipsoidPLTD(position, xRadius, yRadius, zRadius, orientation, includeField, gravityXRadius, gravityYRadius, gravityZRadius, gravityForce, color);
	m_planetoids.emplace_back(ellipsoid);
	return ellipsoid;
}


RoundCubePLTD* Game::SpawnRoundedCube(Vec3 position, float length, float width, float height, float roundedness, EulerAngles orientation, bool includeField, float gravityLength, float gravityWidth, float gravityHeight, float gravityForce, Rgba8 color)
{
	RoundCubePLTD* roundCube = new RoundCubePLTD(position, length, width, height, roundedness, orientation, includeField, gravityLength, gravityWidth, gravityHeight, gravityForce, color);
	m_planetoids.emplace_back(roundCube);
	return roundCube;
}


TorusPLTD* Game::SpawnTorus(Vec3 position, float tubeRadius, float holeRadius, EulerAngles orientation, bool includeField, float gravityRadius, float gravityForce, Rgba8 color)
{
	TorusPLTD* torus = new TorusPLTD(position, tubeRadius, holeRadius, orientation, includeField, gravityRadius, gravityForce, color);
	m_planetoids.emplace_back(torus);
	return torus;
}


BowlPLTD* Game::SpawnBowl(Vec3 position, float radius, float thickness, EulerAngles orientation, bool includeField, float gravityRadius, float gravityForce, Rgba8 color)
{
	BowlPLTD* bowl = new BowlPLTD(position, radius, thickness, orientation, includeField, gravityRadius, gravityForce, color);
	m_planetoids.emplace_back(bowl);
	return bowl;
}


MobiusPLTD* Game::SpawnMobiusStrip(Vec3 position, float radius, float width, EulerAngles orientation, bool includeField, float gravityHeight, float gravityForce, Rgba8 color)
{
	MobiusPLTD* strip = new MobiusPLTD(position, radius, width * 0.5f, orientation, includeField, gravityHeight, gravityForce, color);
	m_planetoids.emplace_back(strip);
	return strip;
}


WirePLTD* Game::SpawnWire(Vec3 position, float radius, WirePerlinParameters perlinStruct, EulerAngles orientation, bool includeField, float gravityRadius, float gravityForce, Rgba8 color)
{
	WirePLTD* wire = new WirePLTD(position, radius, perlinStruct, orientation, includeField, gravityRadius, gravityForce, color);
	m_planetoids.emplace_back(wire);
	return wire;
}


TeapotPLTD* Game::SpawnTeapot(Vec3 position, float scale, EulerAngles orientation, bool includeField, float gravityRadius, float gravityForce, Rgba8 color)
{
	TeapotPLTD* teapot = new TeapotPLTD(position, scale, orientation, color, includeField, gravityRadius, gravityForce);
	m_planetoids.emplace_back(teapot);
	return teapot;
}


SkyStationPLTD* Game::SpawnSkyStation(Vec3 position, float scale, EulerAngles orientation, bool includeField, float gravityRadius, float gravityForce, Rgba8 color)
{
	SkyStationPLTD* skyStation = new SkyStationPLTD(position, scale, orientation, color, includeField, gravityRadius, gravityForce);
	m_planetoids.emplace_back(skyStation);
	return skyStation;
}


MountainPLTD* Game::SpawnMountain(Vec3 position, float scale, EulerAngles orientation, bool includeField, float gravityRadius, float gravityForce, Rgba8 color)
{
	MountainPLTD* mountain = new MountainPLTD(position, scale, orientation, color, includeField, gravityRadius, gravityForce);
	m_planetoids.emplace_back(mountain);
	return mountain;
}


FortressPLTD* Game::SpawnFortress(Vec3 position, float scale, EulerAngles orientation, bool includeField, float gravityHeight, float gravityForce, Rgba8 color)
{
	FortressPLTD* fortress = new FortressPLTD(position, scale, orientation, color, includeField, gravityHeight, gravityForce);
	m_planetoids.emplace_back(fortress);
	return fortress;
}


//
//public mode switching functions
//
void Game::EnterSandboxMode()
{
	m_isSandboxMode = true;
}


void Game::ExitSandboxMode()
{
	m_isSandboxMode = false;
}


//
//game flow sub-functions
//
void Game::RenderPlanetoids() const
{
	for (int pltdIndex = 0; pltdIndex < m_planetoids.size(); pltdIndex++)
	{
		if (m_planetoids[pltdIndex] != nullptr)
		{
			m_planetoids[pltdIndex]->Render();
		}
	}
}


//
//gravity management functions
//
void Game::ApplyGravity()
{
	for (int pltdIndex = 0; pltdIndex < m_planetoids.size(); pltdIndex++)
	{
		if (m_planetoids[pltdIndex] != nullptr && m_planetoids[pltdIndex]->m_field != nullptr)
		{
			m_planetoids[pltdIndex]->m_field->ApplyGravity(m_player);
		}
	}
}


//
//collision handling functions
//
void Game::CollidePlayerWithAllPlanetoids()
{
	for (int pltdIndex = 0; pltdIndex < m_planetoids.size(); pltdIndex++)
	{
		if (m_planetoids[pltdIndex] != nullptr)
		{
			m_planetoids[pltdIndex]->CollideWithPlayer(m_player);
		}
	}
}
