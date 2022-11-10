#include "Utils.h"
#include "Fonts/MonoLisa.cpp"
#include "Fonts/SevenSegment.cpp"
#include "Viewer/Windows/FileDialog.h"

const ImWchar FontRanges[] = { 0x0020, 0x03ff, 0 };

ImFont* Utils::LoadFont(int32_t Size, int32_t Index)
{
	return ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(MonoLisa_compressed_data, MonoLisa_compressed_size, (float)Size, 0, Index >= 0 ? &FontRanges[Index] : nullptr);
}

uint32_t Utils::OpenWindowFileDialog(std::function<void()> OnCallback, std::string Path /*= ""*/ )
{
	return SFileDialog::OpenWindow(OnCallback, Path);
}

void Utils::CloseWindowFileDialog(uint32_t& Handle)
{
	SFileDialog::CloseWindow(Handle);
	Handle = INDEX_NONE;
}
