#include "Fonts/MonoLisa.cpp"
#include "Fonts/SevenSegment.cpp"

#include <stdint.h>
#include "imgui.h"

namespace Utils
{
	const ImWchar FontRanges[] = { 0x0020, 0x03ff, 0 };

	ImFont* LoadFont(int32_t Size, int32_t Index)
	{
		return ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(MonoLisa_compressed_data, MonoLisa_compressed_size, (float)Size, 0, Index >= 0 ? &FontRanges[Index] : nullptr);
	}
}