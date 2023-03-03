#pragma once

#include <CoreMinimal.h>
#include "Viewer\ViewChild.h"

enum class EPropertyType
{
	Unknow,
	Sprite,
	SpriteLayer,
	SpriteBlock,
};


class SProperty : public SViewChild
{
public:
	SProperty();

	virtual void Initialize() override;
	virtual void Render() override;

	void SetProperty(EPropertyType PropertyType, ...);

private:
	void RenderPropertySprite();
	void RenderPropertySpriteLayer();
	void RenderPropertySpriteBlock();

	void InitPropertySprite();
	void InitPropertySpriteBlock();

	EPropertyType VisiblePropertyType;

	// sprite
	char SpriteNameBuffer[BUFFER_SIZE_INPUT] = "";
	char SpriteWidthBuffer[BUFFER_SIZE_INPUT] = "";
	char SpriteHeightBuffer[BUFFER_SIZE_INPUT] = "";

	// sprite block
	char SpriteBlockNameBuffer[BUFFER_SIZE_INPUT] = "";

	// cached
	std::weak_ptr<FSprite> CachedSprite;
	std::weak_ptr<FSpriteBlock> CachedSpriteBlock;
};
