#pragma once

#include <CoreMinimal.h>
#include "Viewer\ViewChild.h"

enum class ESequencerControl
{
	None,
	First,
	Previous,
	Play,
	Next,
	Last,
};

class SSequencer : public SViewChild
{
public:
	virtual void Initialize() override;
	virtual void Render() override;

private:
	void RenderControlButtons();
	void RenderSequencer();

	void RenderPopupBlockMenu();

	void ClickedSpriteBlok(std::shared_ptr<FSpriteBlock> Block);
	void PropertiesSpriteBlock();
	void RemoveSpriteBlok();

	bool DrawButton(const char* StringID, const FImage& Image, const ImVec4& BackgroundColor, const ImVec4& TintColor, const ImVec4& SelectedColor);
	void DrawLayer(std::shared_ptr<FSprite> Sprite, std::shared_ptr<FSpriteLayer> SpriteLayer, uint32_t NumLayer, uint32_t NumFrame);
	void DrawFrames(std::shared_ptr<FSprite> Sprite, std::shared_ptr<FSpriteLayer> SpriteLayer, uint32_t NumLayer, uint32_t NumFrame);

	bool bVisible = true;
	bool bLock = false;
	ESequencerControl Selected = ESequencerControl::None;

	// popup menu
	std::weak_ptr<FSprite> CachedSelectedSprite;
	std::weak_ptr<FSpriteLayer> CachedSelectedSpriteLeyer;
	std::weak_ptr<FSpriteBlock> CachedSelectedSpriteBlock;

	// images
	std::shared_ptr<FImage> ImageFirstButton;
	std::shared_ptr<FImage> ImagePreviousButton;
	std::shared_ptr<FImage> ImagePlayButton;
	std::shared_ptr<FImage> ImageNextButton;
	std::shared_ptr<FImage> ImageLastButton;

	std::shared_ptr<FImage> ImageVisibleEnable;
	std::shared_ptr<FImage> ImageVisibleDisable;
	std::shared_ptr<FImage> ImageLock;
	std::shared_ptr<FImage> ImageUnlock;
	std::shared_ptr<FImage> ImageEmpty;
	std::shared_ptr<FImage> ImageFill;
};
