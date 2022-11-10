// ZX-Convert.cpp : Defines the entry point for the application.

#define WIN32_LEAN_AND_MEAN

#include <string>
#include <stdint.h>
#include <vector>
#include <windows.h>
#include <shellapi.h>
#include <Core/AppFramework.h>

using namespace std;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    int32_t NumArgs;
    LPWSTR* lpszArgv = CommandLineToArgvW(lpCmdLine, &NumArgs);

	vector<wstring> Args;
	for (int i = 0; i < NumArgs; ++i)
	{
		Args.push_back(wstring(lpszArgv[i]));
	}
    LocalFree(lpszArgv);

    FAppFramework AppFramework;
    return AppFramework.Launch(Args);
}