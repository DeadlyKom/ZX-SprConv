#pragma once

#include <CoreMinimal.h>
#include "Image.h"

enum class EColorMode
{
	Unknow,
	ZX,
	Indexed,
	RGB
};

struct FSpriteBlock
{
	FSpriteBlock()
		: Offset(0.0f, 0.0f)
		, Marquee(0.0f, 0.0f, 0.0f, 0.0f)
		, ImageSprite(nullptr)
		, Filename("")
	{}
	
	ImVec2 Offset;
	ImRect Marquee;

	std::shared_ptr<FImage> ImageSprite;
	std::string Filename;
};

struct FSpriteLayer
{
	FSpriteLayer()
		: bVisible(false)
		, bLock(false)
		, bEmpty(true)
		, Name("")
	{}

	bool bVisible;
	bool bLock;
	bool bEmpty;
	std::string Name;
	std::vector<std::vector<FSpriteBlock>> ArrayBlocks;
};

struct FSprite
{
	FSprite()
		: NumFrame(0)
		, Size(-1.0f, -1.0f)
		, Pivot(0.0f, 0.0f)
		, ColorMode(EColorMode::Unknow)
		, Name("")
	{}

	inline bool IsValid() const { return Size.x > 0.0 && Size.y > 0.0f; }

	uint32_t NumFrame;

	ImVec2 Size;
	ImVec2 Pivot;

	EColorMode ColorMode;

	static std::string ColotModeToString(EColorMode Mode)
	{
		switch (Mode)
		{
		case EColorMode::ZX:		return "ZX";		break;
		case EColorMode::Indexed:	return "Indexed";	break;
		case EColorMode::RGB:		return "RGBA";		break;
		case EColorMode::Unknow:
		default:					return "";			break;
		}
	}

	std::string Name;
	std::vector<FSpriteLayer> Layers;
};
