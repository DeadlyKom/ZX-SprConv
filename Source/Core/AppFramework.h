#pragma once

#define WIN32_LEAN_AND_MEAN

#include <string>
#include <stdint.h>
#include <map>
#include <vector>
#include <memory>
#include <windows.h>
#include <codecvt>
#include <functional>
#include <algorithm>

#include <d3d11.h>
#include <tchar.h>

#include "imgui.h"

//#include <vector>
//#include <utility>
//#include <stdio.h>
//#include <iostream>
//#include <xlocbuf>
//#include <codecvt>
//#include <iosfwd>
//#include <sstream>
//#include <fstream>
//#include <codecvt>

#include "Utils.h"
#include "Viewer\Viewer.h"

using namespace std;

class FAppFramework : enable_shared_from_this<FAppFramework>
{
public:
	FAppFramework();
	virtual ~FAppFramework();
	static FAppFramework& Get();

	int32_t Launch(const vector<wstring>& Args, int32_t Width = -1, int32_t Height = -1);
	void Release();

	virtual void Startup(const vector<wstring>& Args);
	virtual void Initialize();
	virtual void Shutdown();
	virtual void Tick(float DeltaTime) {};
	virtual void Render();
	virtual void SetRectWindow(uint16_t Width, uint16_t Height);

	uint32_t BindRender(const function<void()>& Callback);
	void UnbindRender(uint32_t Handle);

protected:
	//template <typename... Args>
	//wstring Format(const wstring& format, Args... args)
	//{
	//	wchar_t Buffer[512];
	//	size_t size = swprintf(Buffer, 512, format.c_str(), args...) + 1;
	//	return Buffer;
	//}
	//wstring s2ws(const string& str)
	//{
	//	using convert_typeX = codecvt_utf8<wchar_t>;
	//	wstring_convert<convert_typeX, wchar_t> converterX;
	//	return converterX.from_bytes(str);
	//}
	//string ws2s(const wstring& wstr)
	//{
	//	using convert_typeX = codecvt_utf8<wchar_t>;
	//	wstring_convert<convert_typeX, wchar_t> converterX;
	//	return converterX.to_bytes(wstr);
	//}

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

	wstring ClassName;
	wstring WindowName;

	bool bVsync;

	ImVec4 BackgroundColor;

	ImFont* Font;
	ImFont* SevenSegmentFont;
	int32_t FontSize;

	SViewer Viewer;

	uint32_t HandleCounter;
	map<uint32_t, function<void()>> RenderEvents;
};
