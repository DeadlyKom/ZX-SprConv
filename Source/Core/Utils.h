#pragma once

#include <stdint.h>
#include "imgui.h"

#define INDEX_NONE (-1)

namespace Utils
{
	static ImFont* LoadFont(int32_t Size, int32_t Index = INDEX_NONE);
}