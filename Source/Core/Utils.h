#pragma once

#include <string>
#include <stdint.h>
#include <functional>
#include "imgui.h"

#define INDEX_NONE (-1)

namespace Utils
{
	ImFont* LoadFont(int32_t Size, int32_t Index = INDEX_NONE);
	uint32_t OpenWindowFileDialog(std::function<void()> OnCallback, std::string Path = "");
	void CloseWindowFileDialog(uint32_t& Handle);
}
