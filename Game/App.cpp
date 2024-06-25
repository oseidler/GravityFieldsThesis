#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/backends/imgui_impl_win32.h"
#include "ThirdParty/imgui/backends/imgui_impl_dx11.h"


App* g_theApp = nullptr;

Renderer*	 g_theRenderer = nullptr;
InputSystem* g_theInput = nullptr;
AudioSystem* g_theAudio = nullptr;
Window*		 g_theWindow = nullptr;

Game* g_theGame = nullptr;


//public game flow functions
void App::Startup()
{
	EventSystemConfig eventSystemConfig;
	g_theEventSystem = new EventSystem(eventSystemConfig);
	
	InputSystemConfig inputSystemConfig;
	g_theInput = new InputSystem(inputSystemConfig);

	WindowConfig windowConfig;
	windowConfig.m_windowTitle = "Owen Seidler Thesis Project";
	windowConfig.m_clientAspect = 1.77777778f;
	windowConfig.m_isFullscreen = true;
	windowConfig.m_inputSystem = g_theInput;
	g_theWindow = new Window(windowConfig);

	RendererConfig rendererConfig;
	rendererConfig.m_window = g_theWindow;
	g_theRenderer = new Renderer(rendererConfig);

	DevConsoleConfig devConsoleConfig;
	devConsoleConfig.m_renderer = g_theRenderer;
	devConsoleConfig.m_camera = &m_devConsoleCamera;
	g_theDevConsole = new DevConsole(devConsoleConfig);
	
	AudioSystemConfig audioSystemConfig;
	g_theAudio = new AudioSystem(audioSystemConfig);
	
	g_theEventSystem->Startup();
	g_theDevConsole->Startup();
	g_theInput->Startup();
	g_theWindow->Startup();
	g_theRenderer->Startup();
	g_theAudio->Startup();

	DebugRenderConfig debugRenderConfig;
	debugRenderConfig.m_renderer = g_theRenderer;
	DebugRenderSystemStartup(debugRenderConfig);

	//imgui setup
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; 

	//platform-specific imgui setup
	ImGui_ImplWin32_Init(g_theWindow->GetHwnd());
	ImGui_ImplDX11_Init(g_theRenderer->GetDevice(), g_theRenderer->GetDeviceContext());

	g_theGame = new Game();
	g_theGame->Startup();

	SubscribeEventCallbackFunction("quit", Event_Quit);

	m_devConsoleCamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y));

	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MAJOR, "Controls: ");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " Xbox Left Stick: Move");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " Xbox Right Stick: Rotate Camera");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " Xbox A: Jump");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " Xbox Bumpers: Move Up/Down (free fly only)");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " Xbox Select: Respawn at world origin");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " Xbox Left Trigger: Crouch");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " F1: Open Sandbox Mode Menu");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " F2: Switch Camera Mode");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " F3: Invert Free Camera (in free camera mode)");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " F4: Change No-Gravity Mode (free-fly or maintain direction)");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " F5: Toggle Debug Gravity View");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " F6: Toggle Lighting Menu");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " F7: Enter Free-Fly Mode");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " F8: Restart Game");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " ~: Open Dev Console");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " Escape: Exit Game");
}


void App::Run()
{
	while (!IsQuitting())
	{
		RunFrame();
	}
}


void App::Shutdown()
{
	g_theGame->Shutdown();
	delete g_theGame;
	g_theGame = nullptr;

	//imgui shutdown
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	DebugRenderSystemShutdown();

	g_theAudio->Shutdown();
	delete g_theAudio;
	g_theAudio = nullptr;

	g_theRenderer->Shutdown();
	delete g_theRenderer;
	g_theRenderer = nullptr;

	g_theWindow->Shutdown();
	delete g_theWindow;
	g_theWindow = nullptr;

	g_theInput->Shutdown();
	delete g_theInput;
	g_theInput = nullptr;

	g_theDevConsole->Shutdown();
	delete g_theDevConsole;
	g_theDevConsole = nullptr;

	g_theEventSystem->Shutdown();
	delete g_theEventSystem;
	g_theEventSystem = nullptr;
}


void App::RunFrame()
{
	//tick the system clock
	Clock::TickSystemClock();

	//run through the four parts of the frame
	BeginFrame();
	Update();
	Render();
	RenderImGui();
	//render dev console separately from and after rest of game
	g_theRenderer->BeginCamera(m_devConsoleCamera);
	g_theDevConsole->Render(AABB2(0.0f, 0.0f, SCREEN_CAMERA_SIZE_X * 0.9f, SCREEN_CAMERA_SIZE_Y * 0.9f));
	g_theRenderer->EndCamera(m_devConsoleCamera);
	EndFrame();
}


//
//public app utilities
//
bool App::HandleQuitRequested()
{
	m_isQuitting = true;
	return m_isQuitting;
}


//
//static app utilities
//
bool App::Event_Quit(EventArgs& args)
{
	UNUSED(args);

	if (g_theApp != nullptr)
	{
		g_theApp->HandleQuitRequested();
	}

	return true;
}


//
//private game flow functions
//
void App::BeginFrame()
{
	g_theEventSystem->BeginFrame();
	g_theDevConsole->BeginFrame();
	g_theInput->BeginFrame();
	g_theWindow->BeginFrame();
	g_theRenderer->BeginFrame();
	g_theAudio->BeginFrame();

	DebugRenderBeginFrame();
}


void App::Update()
{
	//quit or leave attract mode if q is pressed
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		HandleQuitRequested();
	}

	//recreate game if f8 is pressed
	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		RestartGame();
	}

	//set mouse state based on game
	if (!g_theDevConsole->IsOpen() && Window::GetWindowContext()->HasFocus() && !g_theGame->m_isSandboxMode && !g_theGame->m_lightingMenuOpen)
	{
		g_theInput->SetCursorMode(true, true);
	}
	else
	{
		g_theInput->SetCursorMode(false, false);
	}

	//update the game
	g_theGame->Update();

	//go back to the start if the game finishes
	if (g_theGame->m_isFinished)
	{
		RestartGame();
	}
}


void App::Render() const
{	
	//imgui begin frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	
	g_theGame->Render();
}


void App::RenderImGui()
{
	g_theGame->RenderImGui();

	//imgui end frame
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}


void App::EndFrame()
{
	g_theEventSystem->EndFrame();
	g_theDevConsole->EndFrame();
	g_theInput->EndFrame();
	g_theWindow->EndFrame();
	g_theRenderer->EndFrame();
	g_theAudio->EndFrame();

	DebugRenderEndFrame();
}


//
//private app utilities
//
void App::RestartGame()
{
	//delete old game
	g_theGame->Shutdown();
	delete g_theGame;
	g_theGame = nullptr;

	//initialize new game
	g_theGame = new Game();
	g_theGame->Startup();
}
