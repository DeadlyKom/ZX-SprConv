#pragma once

#include <CoreMinimal.h>
#include "Core\Sprite.h"

enum class EToolType;

enum class EWindowsType
{
	ImageList = 0,
	Tools,
	Sprite,
	Palette,
	BuildSprite,
	Sequencer,
	SetSprite,
};

struct FViewFlags
{
	bool bAttributeGrid = false;
	bool bGrid = false;
	bool bPixelGrid = true;
	int GridSize[2] = { 16, 16 };
};

class SViewer : public SWindow
{
public:
	SViewer();

	std::shared_ptr<SWindow> GetWindow(EWindowsType Type);

	virtual void NativeInitialize(FNativeDataInitialize Data) override;
	virtual void Initialize() override;
	virtual void Render() override;
	virtual void Tick(float DeltaTime) override;
	virtual void Destroy() override;

	FViewFlags& GetViewFlags() { return ViewFlags; }
	FSprite& GetSelectedSprite();

	bool IsHandTool();
	bool IsMarqueeTool();

private:
	void HandlerInput();
	FViewFlags ViewFlags;
	std::map<EWindowsType, std::shared_ptr<SWindow>> Windows;

	EToolType LastSelectedTool;
	std::vector<FSprite> Sprites;

	template <typename T>
	T* WindowCast(EWindowsType Type)
	{
		return static_cast<T*>(Windows[Type].get());
	}
};
