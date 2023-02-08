#pragma once

#include <CoreMinimal.h>
#include "Viewer\ViewChild.h"

enum class EToolType
{
	None,
	Marquee,
	Pan,
	Eraser,
	Hand,
	Move,
};

class SViewer;

class STools : public SViewChild
{
	friend SViewer;

public:
	virtual void Initialize() override;
	virtual void Render() override;

private:
	inline bool IsEqualSelected(EToolType NewSelecte) const { return Selected == NewSelecte; }
	EToolType GetSelected() const { return Selected; }
	EToolType SetSelect(EToolType NewSelecte);

	std::shared_ptr<FImage> ImageMarquee;
	std::shared_ptr<FImage> ImagePan;
	std::shared_ptr<FImage> ImageEraser;
	std::shared_ptr<FImage> ImageHand;
	std::shared_ptr<FImage> ImageMove;

	EToolType Selected = EToolType::None;
};
