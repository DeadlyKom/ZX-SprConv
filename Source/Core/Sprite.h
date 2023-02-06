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
	FSpriteBlock();

	void Initialize();
	void Release();
	inline bool IsValid() const { return ImageSprite != nullptr && ImageSprite->Size.x > 0.0 && ImageSprite->Size.y > 0.0f; }
	
	ImVec2 Offset;
	ImRect Marquee;

	std::string Filename;
	std::shared_ptr<FImage> ImageSprite;
};

struct FSpriteFrame
{
	std::vector<FSpriteBlock> Blocks;
};

struct FSpriteLayer
{
	FSpriteLayer();
	void Release();

	// frame
	inline bool IsValidFrame(uint32_t FrameNum) const { return SpriteFrame.size() > FrameNum; }
	FSpriteFrame& AddFrame();

	// block
	FSpriteFrame* GetSpritesBlocks(uint32_t FrameNum);
	bool AddSpriteBlock(FSpriteBlock& NewSpriteBlock, uint32_t FrameNum);

	bool bVisible;
	bool bLock;
	bool bEmpty;
	std::string Name;
	std::vector<FSpriteFrame> SpriteFrame;
};

struct FSprite
{
	FSprite();

	void Initialize();
	void Release();

	// frame
	void AddFrame();
	FSpriteFrame* GetFrame(uint32_t LayerNum, uint32_t FrameNum);

	// layer
	FSpriteLayer& AddLayer();
	inline bool IsValidLayer(uint32_t LayerNum) const { return Layers.size() > LayerNum; }

	bool Draw(const char* StringID, const ImVec2& VisibleSize, uint32_t FrameNum = 0);
	inline bool IsValid() const { return Size.x > 0.0 && Size.y > 0.0f && ImageSprite != nullptr; }
	static std::string ColotModeToString(EColorMode Mode);

	uint32_t NumFrame;
	ImVec2 Size;
	ImVec2 Pivot;
	EColorMode ColorMode;

	std::string Name;
	std::vector<FSpriteLayer> Layers;
	std::shared_ptr<FImage> ImageSprite;

	// internal
	bool bAnimation;
	uint32_t SelectedLayer;
	uint32_t SelectedFrame;
};
