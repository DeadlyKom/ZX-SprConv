#pragma once

#define WIN32_LEAN_AND_MEAN

#include <d3d11.h>
#include <tchar.h>
#include <stdint.h>
#include <windows.h>

#include <map>
#include <vector>
#include <memory>
#include <string>
#include <codecvt>
#include <functional>
#include <algorithm>

#include "imgui.h"

#include "Utils.h"
#include "Viewer\Viewer.h"
#include "Delegates.h"

class FImageBase;

class FAppFramework : public std::enable_shared_from_this<FAppFramework>
{
	friend FImageBase;
	DECLARE_MULTICAST_DELEGATE(FRenderDelegate);

public:
	FAppFramework();
	virtual ~FAppFramework();
	static FAppFramework& Get();

	int32_t Launch(const std::vector<std::wstring>& Args, int32_t Width = -1, int32_t Height = -1);
	void Release();

	virtual void Startup(const  std::vector<std::wstring>& Args);
	virtual void Initialize();
	virtual void Shutdown();
	virtual void Tick(float DeltaTime) {};
	virtual void Render();
	virtual void SetRectWindow(uint16_t Width, uint16_t Height);

	FRenderDelegate OnRender;

private:
	void Register();
	bool Create(int32_t Width, int32_t Height);
	int32_t Run();
	void Idle();

	bool CreateDeviceD3D();
	void CleanupDeviceD3D();
	void CreateRenderTarget();
	void CleanupRenderTarget();

	void StartupGUI();
	void ShutdownGUI();

	HINSTANCE hInstance;
	ATOM AtomClass;
	HWND hwndAppFramework;
	RECT RectWindow;

	ID3D11Device* Device;
	ID3D11DeviceContext* DeviceContext;
	IDXGISwapChain* SwapChain;
	ID3D11RenderTargetView* RenderTargetView;

	uint32_t ScreenWidth;
	uint32_t ScreenHeight;
	uint32_t WindowWidth;
	uint32_t WindowHeight;

	std::wstring ClassName;
	std::wstring WindowName;

	bool bVsync;

	ImVec4 BackgroundColor;

	ImFont* Font;
	ImFont* SevenSegmentFont;
	int32_t FontSize;

	std::shared_ptr<SViewer> Viewer;
};
