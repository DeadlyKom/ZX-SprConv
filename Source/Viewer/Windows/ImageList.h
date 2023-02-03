#pragma once

#include <CoreMinimal.h>
#include "Viewer\ViewChild.h"

class SImageList : public SViewChild
{
	DECLARE_MULTICAST_DELEGATE(FSelectedImageDelegate, const std::filesystem::directory_entry& /*Path*/);
public:
	virtual void Initialize() override;
	virtual void Render() override;

	FSelectedImageDelegate OnSelectedImage;

private:
	int32_t FileSelectIndex;
};
