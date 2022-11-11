// ZX-Convert.cpp : Defines the entry point for the application.

#define WIN32_LEAN_AND_MEAN

#include <string>
#include <stdint.h>
#include <vector>
#include <windows.h>
#include <shellapi.h>
#include <Core/AppFramework.h>

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    int32_t NumArgs;
    LPWSTR* lpszArgv = CommandLineToArgvW(lpCmdLine, &NumArgs);

    std::vector<std::wstring> Args;
	for (int i = 0; i < NumArgs; ++i)
	{
		Args.push_back(std::wstring(lpszArgv[i]));
	}
    LocalFree(lpszArgv);

    return FAppFramework::Get().Launch(Args);
}
