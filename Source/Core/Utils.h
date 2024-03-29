#pragma once

#include <CoreMinimal.h>

#define BUFFER_SIZE 512

namespace Utils
{
	FFrameworkFlags& GetFrameworkFlags();

	ImFont* LoadFont(int32_t Size, int32_t Index = INDEX_NONE);
	std::shared_ptr<FImage> LoadImage(std::string Filename);
	std::shared_ptr<FImage> LoadImageFromResource(WORD ID, std::wstring Folder);
	
	uint8_t* LoadImageToMemory(const std::string& Filename, ImVec2& OutSize); 	// call	FreeImageToMemory
	bool ResizeRegion(uint8_t* ImageData, const ImVec2& OriginalSize, const ImVec2& RequiredSize, uint8_t*& OutputImageData, const ImVec2& uv0, const ImVec2& uv1);
	void FreeImageToMemory(uint8_t* Data);

	DelegateHandle OpenWindowFileDialog(std::string FileDialogName, EDialogMode Mode, std::function<void(std::filesystem::path)> OnCallback, std::filesystem::path FilePath = "", std::string FilterTypes = "*.*");
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
