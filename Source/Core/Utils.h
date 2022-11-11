#pragma once

#define WIN32_LEAN_AND_MEAN

#include <string>
#include <stdio.h>
#include <stdint.h>
#include <windows.h>
#include <functional>
#include "imgui.h"
#include "Delegates.h"
#include "Image.h"
#include "Viewer\Windows\FileDialog.h"

#define INDEX_NONE (-1)
#define BUFFER_SIZE 512

namespace Utils
{
	ImFont* LoadFont(int32_t Size, int32_t Index = INDEX_NONE);
	std::shared_ptr<FImage> LoadImage(std::string Filename);

	DelegateHandle OpenWindowFileDialog(std::string FileDialogName, EDialogMode Mode, std::function<void(std::filesystem::path)> OnCallback, std::filesystem::path Path = "", std::string FilterTypes = "*.*");
	void CloseWindowFileDialog(DelegateHandle& Handle);

	template <typename... Args>
	std::wstring Format(const std::wstring& format, Args... args)
	{
		wchar_t Buffer[BUFFER_SIZE];
		const size_t size = swprintf(Buffer, BUFFER_SIZE, format.c_str(), args...) + 1;
		return Buffer;
	}

	template <typename... Args>
	std::string Format(const std::string& format, Args... args)
	{
		char Buffer[BUFFER_SIZE];
		const size_t size = sprintf(Buffer, format.c_str(), args...) + 1;
		return Buffer;
	}

	std::wstring Utf8ToUtf16(const std::string& String);
	std::string Utf16ToUtf8(const std::wstring& String);
}
