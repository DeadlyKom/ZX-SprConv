#pragma once

#include <CoreMinimal.h>

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

class STools : public SWindow
{
	friend SViewer;

public:
	virtual void Initialize() override;
	virtual void Render() override;

private:
	EToolType GetSelected() const { return Selected; }
	EToolType SetSelect(EToolType NewSelecte);

	std::shared_ptr<FImage> ImageMarquee;
	std::shared_ptr<FImage> ImagePan;
	std::shared_ptr<FImage> ImageEraser;
	std::shared_ptr<FImage> ImageHand;
	std::shared_ptr<FImage> ImageMove;

	EToolType Selected = EToolType::None;
};
