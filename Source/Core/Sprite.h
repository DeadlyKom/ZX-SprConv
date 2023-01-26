#pragma once

#include <CoreMinimal.h>
#include "Image.h"

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
		, Size(0.0f, 0.0f)
		, Pivot(0.0f, 0.0f)
		, Name("")
	{}

	uint32_t NumFrame;
	ImVec2 Size;
	ImVec2 Pivot;
	std::string Name;
	std::vector<FSpriteLayer> Layers;
};
