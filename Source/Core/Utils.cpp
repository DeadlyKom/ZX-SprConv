#include "Utils.h"
#include "Image.h"
#include "Fonts\MonoLisa.cpp"
#include "AppFramework.h"
#include "Fonts\SevenSegment.cpp"
#include "Fonts\ZX-Spectrum.cpp"
#include "Viewer\Windows\FileDialog.h"

const ImWchar FontRanges[] = { 0x0020, 0x03ff, 0 };

ImFont* Utils::LoadFont(int32_t Size, int32_t Index)
{
	return ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(MonoLisa_compressed_data, MonoLisa_compressed_size, (float)Size, 0, Index >= 0 ? &FontRanges[Index] : nullptr);
}

std::shared_ptr<FImage> Utils::LoadImage(std::string Filename)
{
	return FImageBase::Get().Load(Filename);
}

std::shared_ptr<FImage> Utils::LoadImageFromResource(WORD ID, std::wstring Folder)
{
	return FImageBase::Get().FromMemory(FAppFramework::Get().FromResource(ID, Folder));
}

DelegateHandle Utils::OpenWindowFileDialog(std::string FileDialogName, EDialogMode Mode, std::function<void(std::filesystem::path)> OnCallback, std::filesystem::path Path /*= ""*/, std::string FilterTypes /*= "*.*"*/)
{
	return SFileDialog::OpenWindow(FileDialogName, Mode, OnCallback, Path, FilterTypes);
}

void Utils::CloseWindowFileDialog(DelegateHandle& Handle)
{
	SFileDialog::CloseWindow(Handle);
}

std::wstring Utils::Utf8ToUtf16(const std::string& String)
{
	std::wstring wstrTo;
	wchar_t* wszTo = new wchar_t[String.length() + 1];
	wszTo[String.size()] = L'\0';
	MultiByteToWideChar(CP_ACP, 0, String.c_str(), -1, wszTo, (int)String.length());
	wstrTo = wszTo;
	delete[] wszTo;
	return wstrTo;
}

std::string Utils::Utf16ToUtf8(const std::wstring& String)
{
	std::string strTo;
	char* szTo = new char[String.length() + 1];
	szTo[String.size()] = '\0';
	WideCharToMultiByte(CP_ACP, 0, String.c_str(), -1, szTo, (int)String.length(), NULL, NULL);
	strTo = szTo;
	delete[] szTo;
	return strTo;
}
