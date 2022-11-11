#pragma once

#include <memory>
#include <vector>
#include "imgui.h"
#include "Core\Utils.h"

class FFonts
{
public:
	FFonts();
	virtual ~FFonts();
	static FFonts& Get();

	uint32_t LoadFont(int32_t Size, int32_t Index = INDEX_NONE);
	ImFont* GetFont(uint32_t Handle);

private:
	std::vector<ImFont*> Fonts;
};
