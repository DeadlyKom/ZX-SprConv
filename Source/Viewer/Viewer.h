#pragma once

#include <CoreMinimal.h>

enum class EWindowsType
{
	ImageList = 0,
	Sprite,
	Palette,
};

class SViewer : public SWindow
{
public:
	SViewer();

	std::shared_ptr<SWindow> GetWindow(EWindowsType Type);

	virtual void NativeInitialize(FNativeDataInitialize Data) override;
	virtual void Initialize() override;
	virtual void Render() override;
	virtual void Destroy() override;

private:
	std::map<EWindowsType, std::shared_ptr<SWindow>> Windows;
};
