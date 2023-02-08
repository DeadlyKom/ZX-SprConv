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

struct FSpriteBlock : std::enable_shared_from_this<FSpriteBlock>
{
	FSpriteBlock();

	void Initialize();
	void Release();
	inline bool IsValid() const { return ImageSprite != nullptr && ImageSprite->Size.x > 0.0 && ImageSprite->Size.y > 0.0f; }
	
	ImVec2 Offset;
	ImRect Marquee;

	std::string Name;
	std::string Filename;
	std::shared_ptr<FImage> ImageSprite;
};

struct FSpriteFrame : std::enable_shared_from_this<FSpriteFrame>
{
	std::vector<std::weak_ptr<FSpriteBlock>> Blocks;
};

struct FSpriteLayer : std::enable_shared_from_this<FSpriteLayer>
{
	FSpriteLayer();
	void Release();

	// frame
	inline bool IsValidFrame(uint32_t FrameNum) const { return Frames.size() > FrameNum; }
	std::shared_ptr<FSpriteFrame> GetSpritesFrame(uint32_t FrameNum);
	std::shared_ptr<FSpriteFrame> AddFrame();

	// block
	bool AddSpriteBlock(std::shared_ptr<FSpriteBlock>& NewSpriteBlock, uint32_t FrameNum);

	bool bVisible;
	bool bLock;
	bool bEmpty;
	std::string Name;
	std::vector<std::shared_ptr<FSpriteBlock>> Blocks;
	std::vector<std::shared_ptr<FSpriteFrame>> Frames;
};

struct FSprite : std::enable_shared_from_this<FSprite>
{
	FSprite();

	void Initialize();
	void Release();

	// frame
	void AddFrame();
	std::shared_ptr<FSpriteFrame> GetFrame(uint32_t LayerNum, uint32_t FrameNum);

	// layer
	std::shared_ptr<FSpriteLayer> AddLayer();
	inline bool IsValidLayer(uint32_t LayerNum) const { return Layers.size() > LayerNum; }

	bool Draw(const char* StringID, std::shared_ptr<FImage>& ImageEmpty, const ImVec2& VisibleSize, uint32_t FrameNum = 0);
	inline bool IsValid() const { return Size.x > 0.0 && Size.y > 0.0f && ImageSprite != nullptr; }
	static std::string ColotModeToString(EColorMode Mode);

	uint32_t NumFrame;
	ImVec2 Size;
	ImVec2 Pivot;
	EColorMode ColorMode;

	std::string Name;
	std::vector<std::shared_ptr<FSpriteLayer>> Layers;
	std::shared_ptr<FImage> ImageSprite;

	// internal
	bool bAnimation;
	uint32_t SelectedLayer;
	uint32_t SelectedFrame;
};
