#include "Fonts.h"

FFonts::FFonts()
{}

FFonts::~FFonts()
{}

FFonts& FFonts::Get()
{
	static std::shared_ptr<FFonts> Instance(new FFonts);
	return *Instance.get();
}

uint32_t FFonts::LoadFont(int32_t Size, int32_t Index /*= INDEX_NONE*/)
{
	Fonts.push_back(Utils::LoadFont(Size, Index));
	return uint32_t(Fonts.size()-1);
}

ImFont* FFonts::GetFont(uint32_t Handle)
{
	return Handle >= Fonts.size() ? nullptr : Fonts[Handle];
}
