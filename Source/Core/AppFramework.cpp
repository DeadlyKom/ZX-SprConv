#include "AppFramework.h"

#include <ctime>
#include <fstream>

#include "backends\imgui_impl_win32.h"
#include "backends\imgui_impl_dx11.h"

#include "..\ZX-Convert\Resource.h"
#include "Fonts\Fonts.h"

namespace
{
	enum class ECommandLine
	{
		Unknow,
		Log,
		Fullscreen,
	};

	std::map<std::wstring, ECommandLine> CommandLineArray =
	{
		{TEXT("-log"), ECommandLine::Log},
		{TEXT("-fullscreen"), ECommandLine::Fullscreen}
	};
}

namespace Path
{
	const char* Log = "Saved/Logs";
	const char* Config = "Saved/Config";

	const char* LogFilename = "imgui_log.txt";
	const char* IniFilename = "imgui.ini";
}

namespace KeywordArg
{
	const std::wstring FULLSCREEN = TEXT("-fullscreen");
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LONG_PTR CALLBACK AppFrameworkProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static FAppFramework* AppFramework = nullptr;

	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
	{
		return true;
	}

	switch (msg)
	{
	case WM_CREATE:
		AppFramework = (FAppFramework*)((LPCREATESTRUCT)lParam)->lpCreateParams;
		break;

	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED)
		{
			AppFramework->SetRectWindow(LOWORD(lParam), HIWORD(lParam));
		}
		return 0;

	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU)			// Disable ALT application menu
		{
			return 0;
		}
		break;

	case WM_DPICHANGED:
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
		{
			//const int dpi = HIWORD(wParam);
			//printf("WM_DPICHANGED to %d (%.0f%%)\n", dpi, (float)dpi / 96.0f * 100.0f);
			const RECT* suggested_rect = (RECT*)lParam;
			SetWindowPos(hWnd, NULL, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);;
}

FAppFramework::FAppFramework()
	: AtomClass(0)
	, Device(nullptr)
	, DeviceContext(nullptr)
	, SwapChain(nullptr)
	, RenderTargetView(nullptr)
	, ClassName(TEXT("DefaultClassName"))
	, WindowName(TEXT("ZX-Editor"))
	, bVsync(true)
	, BackgroundColor(ImVec4(0.0f, 0.0f, 0.0f, 1.0f))
	, Font(nullptr)
	, SevenSegmentFont(nullptr)
	, FontSize(15)
{
	ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	ScreenHeight = GetSystemMetrics(SM_CYSCREEN);
}

FAppFramework::~FAppFramework()
{}

FAppFramework& FAppFramework::Get()
{
	static std::shared_ptr<FAppFramework> InstanceAppFramework(new FAppFramework());
	return *InstanceAppFramework.get();
}

int32_t FAppFramework::Launch(const std::vector<std::wstring>& Args, int32_t Width /*= -1*/, int32_t Height /*= -1*/)
{
	// initialize
	WindowWidth = Width;
	WindowHeight = Height;

	// startup
	if (!Startup(Args))
	{
		Shutdown();
		return 1;
	}
	Register();

	if (!Create(WindowWidth, WindowHeight))
	{
		Shutdown();
		return 1;
	}
	
	Initialize();
	if(!StartupGUI())
	{
		Shutdown();
		return 1;
	}

	// main loop
	int32_t Return = Run();

	// shutdown
	Shutdown();

	return Return;
}

void FAppFramework::Release()
{
	CleanupDeviceD3D();

	if (AtomClass != 0)
	{
		UnregisterClass((LPCWSTR)AtomClass, hInstance);
	}
}

bool FAppFramework::Startup(const std::vector<std::wstring>& Args)
{
	for (const std::wstring& Arg : Args)
	{
		const std::map< std::wstring, ECommandLine>::iterator& SearchIt = CommandLineArray.find(Arg);
		const ECommandLine CommandLine = SearchIt != CommandLineArray.end() ? SearchIt->second : ECommandLine::Unknow;

		switch (CommandLine)
		{
		case ECommandLine::Unknow:
			break;

		case ECommandLine::Log:
			Flags.bLog = true;
			break;

		case ECommandLine::Fullscreen:
			WindowWidth = ScreenWidth;
			WindowHeight = ScreenHeight;
			break;

		default:
			break;
		}
	}

	IniFilePath = Utils::Format("%s/%s", Path::Config, Path::IniFilename);
	const bool bInitField = InitField();

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& IO = ImGui::GetIO();
	IO.IniFilename = IniFilePath.c_str();

	if (Flags.bLog)
	{
		char Buffer[80];
		std::time_t Time = std::time(nullptr);
		std::tm* Now = std::localtime(&Time);

		if (true)
		{
			strftime(Buffer, sizeof(Buffer), "%d.%m.%Y-%H.%M.%S", Now);
			LogFilePath = Utils::Format("%s/%s.log", Path::Log, Buffer);
			IO.LogFilename = LogFilePath.c_str();
		}
		else
		{		
			LogFilePath = Utils::Format("%s/%s.log", Path::Log, Path::LogFilename);
			IO.LogFilename = LogFilePath.c_str();
		}
		ImGui::LogToFile();
		strftime(Buffer, sizeof(Buffer), "Log file open, %Y/%m/%d %X", Now);
		ImGui::LogText(IM_NEWLINE);
		LOG(Buffer);
	}

	if (!bInitField)
	{
		LOG("Framework: Startup fail");
		return false;
	}

	LOG("Framework: Startup success");
	return true;
}

void FAppFramework::Initialize()
{
	LOG("Framework: Initialize");

	Viewer = std::make_shared<SViewer>();

	FNativeDataInitialize Data;
	Data.Device = Device;
	Data.DeviceContext = DeviceContext;
	Viewer->NativeInitialize(Data);
}

void FAppFramework::Shutdown()
{
	LOG("Framework: Shutdown");

	if (Viewer != nullptr)
	{
		Viewer->Destroy();
		Viewer.reset();
	}

	// internal
	ShutdownGUI();
	DestroyWindow(hwndAppFramework);
	Release();
}

void FAppFramework::Tick(float DeltaTime)
{
	Viewer->Tick(DeltaTime);
}

void FAppFramework::Render()
{
	Viewer->Render();
	OnRender.Broadcast();
}

void FAppFramework::SetRectWindow(uint16_t Width, uint16_t Height)
{
	if (Device != nullptr)
	{
		CleanupRenderTarget();
		SwapChain->ResizeBuffers(0, (UINT)Width, (UINT)Height, DXGI_FORMAT_UNKNOWN, 0);
		CreateRenderTarget();
	}
}

std::string FAppFramework::LoadShaderResource(WORD ID)
{
	HRSRC hRes = FindResource(hInstance, MAKEINTRESOURCE(ID), TEXT("SHADER"));
	HGLOBAL hGlob = hRes != NULL ? LoadResource(hInstance, hRes) : NULL;
	LPVOID lpResLock = hGlob != NULL ? LockResource(hGlob) : NULL;
	if (lpResLock == NULL)
	{
		return "";
	}

	DWORD Size = SizeofResource(hInstance, hRes);
	const char* pData = reinterpret_cast<const char*>(LockResource(hGlob));
	std::string Result(pData, Size);
	FreeResource(hGlob);
	return Result;
}

std::vector<char> FAppFramework::FromResource(WORD ID, std::wstring Folder /*= TEXT("")*/)
{
	HRSRC hRes = FindResource(hInstance, MAKEINTRESOURCE(ID), Folder.c_str());
	HGLOBAL hGlob = hRes != NULL ? LoadResource(hInstance, hRes) : NULL;
	LPVOID lpResLock = hGlob != NULL ? LockResource(hGlob) : NULL;
	if (lpResLock == NULL)
	{
		return std::vector<char>();
	}

	DWORD Size = SizeofResource(hInstance, hRes);
	const char* pData = reinterpret_cast<const char*>(LockResource(hGlob));
	std::vector<char> Result(pData, pData + Size);
	FreeResource(hGlob);
	return Result;
}

void FAppFramework::Register()
{
	LOG("Framework: Register");

	WNDCLASSEX wcex;
	hInstance = GetModuleHandle(nullptr);

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_CLASSDC/* | CS_HREDRAW | CS_VREDRAW*/;
	wcex.lpfnWndProc = (WNDPROC)AppFrameworkProc;
	wcex.cbClsExtra =
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOW));
	wcex.hCursor = nullptr;
	wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = ClassName.c_str();
	wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));

	AtomClass = RegisterClassEx(&wcex);
}

bool FAppFramework::Create(int32_t Width, int32_t Height)
{
	LOG("Framework: Create windows");

	const uint32_t WindowWidth = Width > 0 ? Width : ScreenWidth >> 1;
	const uint32_t WindowHeight = Height > 0 ? Height : ScreenHeight >> 1;
	const uint32_t WindowPositionX = (ScreenWidth - WindowWidth) >> 1;
	const uint32_t WindowPositionY = (ScreenHeight - WindowHeight) >> 1;

	hwndAppFramework = CreateWindowEx(NULL,
		(LPCWSTR)AtomClass,
		WindowName.c_str(),
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		WindowPositionX, WindowPositionY,
		WindowWidth, WindowHeight,
		nullptr,
		nullptr,
		hInstance,
		this);

	if (!CreateDeviceD3D())
	{
		LOG("Framework: Create windows failed");
		return false;
	}

	UpdateWindow(hwndAppFramework);
	GetClientRect(hwndAppFramework, &RectWindow);

	LOG("Framework: Create windows success");
	return true;
}

bool FAppFramework::CreateDeviceD3D()
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hwndAppFramework;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT CreateDeviceFlags = 0;
	//CreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL FeatureLevel;
	const D3D_FEATURE_LEVEL FeatureLevelArray[3] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL, CreateDeviceFlags, FeatureLevelArray, 3, D3D11_SDK_VERSION, &sd, &SwapChain, &Device, &FeatureLevel, &DeviceContext)))
	{
		LOG_ERROR("Framework: Create device and swap chain");
		return false;
	}

	CreateRenderTarget();
	return true;
}

void FAppFramework::CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (SwapChain != nullptr)
	{
		SwapChain->Release();
		SwapChain = nullptr;
	}
	if (DeviceContext != nullptr)
	{
		DeviceContext->Release();
		DeviceContext = nullptr;
	}
	if (Device != nullptr)
	{
		Device->Release();
		Device = nullptr;
	}
}

void FAppFramework::CreateRenderTarget()
{
	ID3D11Texture2D* BackBuffer;
	SwapChain->GetBuffer(0, IID_PPV_ARGS(&BackBuffer));
	if (BackBuffer != nullptr)
	{
		Device->CreateRenderTargetView(BackBuffer, nullptr, &RenderTargetView);
		BackBuffer->Release();
	}
}

void FAppFramework::CleanupRenderTarget()
{
	if (RenderTargetView != nullptr)
	{
		RenderTargetView->Release();
		RenderTargetView = nullptr;
	}
}

bool FAppFramework::InitField()
{
	if (!std::filesystem::exists(Path::Log))
	{
		if (!std::filesystem::create_directories(Path::Log))
		{
			return false;
		}
	}

	if (!std::filesystem::exists(Path::Config))
	{
		if (!std::filesystem::create_directories(Path::Config))
		{
			return false;
		}
	}

	if (!std::filesystem::exists(IniFilePath.c_str()))
	{
		SaveDefaultImGuiIni();
	}

	return true;
}

void FAppFramework::SaveDefaultImGuiIni()
{
	std::string DefaultFileContent = R"(
[Window][Palette]
Pos=0,634
Size=192,80
Collapsed=0
DockId=0x0000000C,0

[Window][DockSpaceViewport_11111111]
Pos=0,19
Size=1008,695
Collapsed=0

[Window][Debug##Default]
ViewportPos=549,839
ViewportId=0x9F5F46A1
Size=602,386
Collapsed=0

[Window][Sprite Editor]
Pos=260,20
Size=318,286
Collapsed=0
DockId=0x00000009,0

[Window][Image List]
Pos=0,19
Size=192,613
Collapsed=0
DockId=0x0000000B,0

[Window][Select File]
ViewportPos=611,294
ViewportId=0x798C690B
Size=850,420
Collapsed=0

[Window][Dear ImGui Metrics/Debugger]
ViewportPos=2122,270
ViewportId=0x8FC55800
Size=978,596
Collapsed=0

[Window][Dear ImGui Debug Log]
ViewportPos=2019,198
ViewportId=0x8E350B18
Size=1734,772
Collapsed=0

[Window][Dear ImGui Stack Tool]
ViewportPos=55,58
ViewportId=0x5B1A3814
Size=601,498
Collapsed=0

[Window][Dear ImGui Style Editor]
ViewportPos=106,235
ViewportId=0x6D551092
Size=413,942
Collapsed=0

[Window][About Dear ImGui]
ViewportPos=672,145
ViewportId=0x34B4C494
Size=731,114
Collapsed=0

[Window][Same title as another window##1]
ViewportPos=1349,228
ViewportId=0x78C07CE7
Size=560,528
Collapsed=0

[Window][Debug]
Pos=1519,20
Size=367,702
Collapsed=0
DockId=0x00000003,0

[Window][Debug111]
Size=76,1034
Collapsed=0

[Window][Build Sprite]
Pos=580,20
Size=311,338
Collapsed=0
DockId=0x00000004,0

[Window][Set Sprite]
Pos=580,341
Size=311,160
Collapsed=0
DockId=0x00000010,0

[Window][Sequencer]
Pos=194,512
Size=531,202
Collapsed=0
DockId=0x0000000A,0

[Window][Tools]
Pos=194,19
Size=531,48
Collapsed=0
DockId=0x0000000D,0

[Window][Quit]
Pos=316,191
Size=311,118
Collapsed=0

[Window][CreateSprite]
Pos=348,109
Size=248,282
Collapsed=0

[Window][Create Sprite]
Pos=127,-53
Size=248,393
Collapsed=0

[Window][Sprite Constructor]
Pos=1452,75
Size=341,942
Collapsed=0
DockId=0x00000011,0

[Window][Property]
Pos=727,372
Size=281,342
Collapsed=0
DockId=0x00000012,0

[Window][SpriteConstructor]
Pos=727,19
Size=281,351
Collapsed=0
DockId=0x00000011,0

[Window][SpriteEditor]
Pos=194,69
Size=531,441
Collapsed=0
DockId=0x00000009,0

[Window][Grid Settings]
ViewportPos=1438,271
ViewportId=0x0F8E5DF4
Size=263,282
Collapsed=0

[Table][0x172A0CD1,4]
RefScale=14
Column 0  Width=32 Sort=0^
Column 1  Width=50
Column 2  Width=55
Column 3  Weight=1.0000

[Table][0xC12F73A2,3]
RefScale=14
Column 0  Width=32
Column 1  Width=32
Column 2  Width=236

[Table][0x0EAA33F0,4]
RefScale=13
Column 0  Width=32
Column 1  Width=32
Column 2  Width=114
Column 3  Width=32

[Docking][Data]
DockSpace           ID=0x8B93E3BD Window=0xA787BDB4 Pos=267,213 Size=1008,695 Split=X Selected=0xB3D8F9A0
  DockNode          ID=0x00000001 Parent=0x8B93E3BD SizeRef=192,997 Split=Y Selected=0x41D61AAD
    DockNode        ID=0x0000000B Parent=0x00000001 SizeRef=182,454 HiddenTabBar=1 Selected=0x41D61AAD
    DockNode        ID=0x0000000C Parent=0x00000001 SizeRef=182,59 HiddenTabBar=1 Selected=0x7E84447F
  DockNode          ID=0x00000002 Parent=0x8B93E3BD SizeRef=1726,997 Split=X Selected=0xB3D8F9A0
    DockNode        ID=0x00000007 Parent=0x00000002 SizeRef=467,997 Split=Y Selected=0xB3D8F9A0
      DockNode      ID=0x0000000D Parent=0x00000007 SizeRef=214,48 HiddenTabBar=1 Selected=0xD44407B5
      DockNode      ID=0x0000000E Parent=0x00000007 SizeRef=214,432 Split=Y Selected=0xF3734F40
        DockNode    ID=0x00000009 Parent=0x0000000E SizeRef=319,744 CentralNode=1 HiddenTabBar=1 Selected=0xF3734F40
        DockNode    ID=0x0000000A Parent=0x0000000E SizeRef=319,202 HiddenTabBar=1 Selected=0x3E6B10D2
    DockNode        ID=0x00000008 Parent=0x00000002 SizeRef=281,997 Split=Y Selected=0x508C396C
      DockNode      ID=0x00000005 Parent=0x00000008 SizeRef=256,493 Split=Y Selected=0x508C396C
        DockNode    ID=0x00000003 Parent=0x00000005 SizeRef=406,336 Selected=0x392A5ADD
        DockNode    ID=0x00000004 Parent=0x00000005 SizeRef=406,364 Selected=0x508C396C
      DockNode      ID=0x00000006 Parent=0x00000008 SizeRef=256,206 Split=Y Selected=0x03E5C57B
        DockNode    ID=0x0000000F Parent=0x00000006 SizeRef=311,319 Split=Y Selected=0x39C75561
          DockNode  ID=0x00000011 Parent=0x0000000F SizeRef=616,505 HiddenTabBar=1 Selected=0x39C75561
          DockNode  ID=0x00000012 Parent=0x0000000F SizeRef=616,491 HiddenTabBar=1 Selected=0x1EA09189
        DockNode    ID=0x00000010 Parent=0x00000006 SizeRef=311,160 Selected=0x03E5C57B
)";

	std::ofstream File;
	File.open(IniFilePath.c_str(), std::ios_base::out);
	File << DefaultFileContent;
	File.close();
}

bool FAppFramework::StartupGUI()
{
	LOG("Framework: Startup ImGui");

	ImGuiIO& IO = ImGui::GetIO();
	IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	IO.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	ImGuiStyle& Style = ImGui::GetStyle();

	// when viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	if (IO.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		Style.WindowRounding = 0.0f;
		Style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	//
	Style.WindowMenuButtonPosition = ImGuiDir_Right;

	// Setup Platform/Renderer backends
	if (!ImGui_ImplWin32_Init(hwndAppFramework))
	{
		LOG_ERROR("Framework: ImGui_ImplWin32_Init");
		return false;
	}
	if (!ImGui_ImplDX11_Init(Device, DeviceContext))
	{
		LOG_ERROR("Framework: ImGui_ImplDX11_Init");
		return false;
	}

	//FFonts& Fonts = FFonts::Get();
	//Fonts.LoadFont(11, 0);
	//Font = Fonts.GetFont(Fonts.LoadFont(14, 0));
	//SevenSegmentFont = Fonts.GetFont(Fonts.LoadFont(50));

	return true;
}

void FAppFramework::ShutdownGUI()
{
	if (ImGui::GetCurrentContext() == nullptr && ImGui::GetIO().BackendRendererUserData != nullptr)
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
	}

	ImGui::LogFinish();
	ImGui::DestroyContext();
}

int32_t FAppFramework::Run()
{
	LOG("Framework: Run");

	MSG msg;
	bool bRun = true;

	while (bRun)
	{
		while (PeekMessage(&msg, nullptr, 0, 0, PM_NOREMOVE))
		{
			if (GetMessage(&msg, nullptr, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				bRun = false;
				break;
			}
		}

		if (bRun)
		{
			Idle();
		}

		bRun = bRun ? Viewer->IsOpen() : false;
	}

	return (int32_t)msg.wParam;
}

void FAppFramework::Idle()
{
	const float DeltaTime = 1.0f / 60.0f;
	Tick(DeltaTime);

	// prepare ImGui frame
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	ImGui::PushFont(Font);

	Render();

	//ImGui::ShowMetricsWindow();
	//ImGui::ShowDebugLogWindow();
	//ImGui::ShowStackToolWindow();

	ImGui::PopFont();

	// rendering ImGui frame
	{
		ImGui::Render();
		const float clear_color_with_alpha[4] = { BackgroundColor.x * BackgroundColor.w, BackgroundColor.y * BackgroundColor.w, BackgroundColor.z * BackgroundColor.w, BackgroundColor.w };
		DeviceContext->OMSetRenderTargets(1, &RenderTargetView, nullptr);
		DeviceContext->ClearRenderTargetView(RenderTargetView, clear_color_with_alpha);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		// update and Render additional Platform Windows
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
		SwapChain->Present(!!bVsync, 0);
	}
}
