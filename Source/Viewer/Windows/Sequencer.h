#pragma once

#include <CoreMinimal.h>
#include "Viewer\ViewChild.h"

class SSequencer : public SViewChild
{
public:
	virtual void Initialize() override;
	virtual void Render() override;

private:
	bool bVisible = true;
	bool bLock = false;

	std::shared_ptr<FImage> ImageVisibleEnable;
	std::shared_ptr<FImage> ImageVisibleDisable;
	std::shared_ptr<FImage> ImageLock;
	std::shared_ptr<FImage> ImageUnlock;
	std::shared_ptr<FImage> ImageEmpty;
	std::shared_ptr<FImage> ImageFill;
};
