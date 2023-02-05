#include "backends\imgui_impl_win32.h"
#include "backends\imgui_impl_dx11.h"

#include "AppFramework.h"
#include "..\ZX-Convert\Resource.h"
#include "Fonts\Fonts.h"

namespace KeywordArg
{
	const std::wstring FULLSCREEN = TEXT("-fullscreen");
}

namespace
{
	enum class ECommandLine
	{
		Unknow,
		Fullscreen,
	};

	std::map<std::wstring, ECommandLine> CommandLineArray = { {TEXT("-fullscreen"), ECommandLine::Fullscreen} };
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
	Startup(Args);
	Register();

	if (!Create(WindowWidth, WindowHeight))
	{
		Shutdown();
		return 1;
	}
	
	Initialize();
	StartupGUI();

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

void FAppFramework::Startup(const std::vector<std::wstring>& Args)
{
	for(const  std::wstring& Arg : Args)
	{
		const std::map< std::wstring, ECommandLine>::iterator& SearchIt = CommandLineArray.find(Arg);
		const ECommandLine CommandLine = SearchIt != CommandLineArray.end() ? SearchIt->second : ECommandLine::Unknow;
		switch (CommandLine)
		{
		case ECommandLine::Unknow:
			break;

		case ECommandLine::Fullscreen:
			WindowWidth = ScreenWidth;
			WindowHeight = ScreenHeight;
			break;

		default:
			break;
		}
	}
}

void FAppFramework::Initialize()
{
	Viewer = std::make_shared<SViewer>();

	FNativeDataInitialize Data;
	Data.Device = Device;
	Data.DeviceContext = DeviceContext;
	Viewer->NativeInitialize(Data);
}

void FAppFramework::Shutdown()
{
	if (Viewer != nullptr)
	{
		Viewer->Destroy();
		Viewer.reset();
	}

	// internal
	ShutdownGUI();
	CleanupDeviceD3D();
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
		return false;
	}

	UpdateWindow(hwndAppFramework);
	GetClientRect(hwndAppFramework, &RectWindow);

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
	CreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL FeatureLevel;
	const D3D_FEATURE_LEVEL FeatureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL, CreateDeviceFlags, FeatureLevelArray, 2, D3D11_SDK_VERSION, &sd, &SwapChain, &Device, &FeatureLevel, &DeviceContext) != S_OK)
	{
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

void FAppFramework::StartupGUI()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& IO = ImGui::GetIO();
	IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	IO.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	ImGuiStyle& Style = ImGui::GetStyle();
	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	if (IO.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		Style.WindowRounding = 0.0f;
		Style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	//
	Style.WindowMenuButtonPosition = ImGuiDir_Right;

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hwndAppFramework);
	ImGui_ImplDX11_Init(Device, DeviceContext);

	FFonts& Fonts = FFonts::Get();
	Fonts.LoadFont(11, 0);
	Font = Fonts.GetFont(Fonts.LoadFont(14, 0));
	SevenSegmentFont = Fonts.GetFont(Fonts.LoadFont(50));
}

void FAppFramework::ShutdownGUI()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

int32_t FAppFramework::Run()
{
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
