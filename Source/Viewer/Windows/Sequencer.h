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

	bool bVisible = true;
	bool bLock = false;
	ESequencerControl Selected = ESequencerControl::None;

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
