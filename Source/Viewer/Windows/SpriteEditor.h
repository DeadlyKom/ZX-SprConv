#pragma once

#include <CoreMinimal.h>
#include "Viewer\ViewChild.h"

class SSpriteEditor : public SViewChild
{
public:
	SSpriteEditor();
	virtual void NativeInitialize(FNativeDataInitialize Data) override;
	virtual void Initialize() override;
	virtual void Render() override;
	virtual void Tick(float DeltaTime);
	virtual void Destroy() override;

	// callback
	void OnDrawCallback(const ImDrawList* ParentList, const ImDrawCmd* CMD);

private:
	// render
	void RenderEditorSprite();
	void DrawMarquee(const ImRect& Window);
	void RenderPopupMenu();

	// internal
	void RoundImagePosition();
	void SetScale(float scaleY);
	void SetScale(ImVec2 NewScale);
	void SetImagePosition(ImVec2 NewPosition);
	ImVec2 CalculatePanelSize();
	ImVec2 ConverPositionToPixel(const ImVec2& Position);
	Transform2D GetTexelsToPixels(const ImVec2& ScreenTopLeft, const ImVec2& ScreenViewSize, const ImVec2& UVTopLeft, const ImVec2& UVViewSize, const ImVec2& TextureSize);

	// input
	void HandleKeyboardInputs();
	void HandleMouseInputs();
	void HandleMarqueeInput();

	// event
	void OnSelectedFileImage(const std::filesystem::directory_entry& FilePath);

	// directX
	ID3D11Device* Device;
	ID3D11DeviceContext* DeviceContext;
	ID3D11PixelShader* PS_Grid;
	ID3D11PixelShader* PS_LineMarchingAnts;
	ID3D11Buffer* PCB_Grid;
	ID3D11Buffer* PCB_MarchingAnts;

	// shader variable
	float TimeCounter;
	bool bForceNearestSampling;
	ImVec2 GridWidth;
	ImVec2 GridSize;
	ImVec2 GridOffset;
	ImVec4 GridColor;
	ImVec4 BackgroundColor;

	// scale
	float ZoomRate;
	ImVec2 Scale;
	ImVec2 OldScale;
	ImVec2 ScaleMin;
	ImVec2 ScaleMax;

	float PixelAspectRatio;
	float MinimumGridSize;

	// view state	
	ImVec2 ImagePosition;
	ImVec2 PanelTopLeftPixel;
	ImVec2 PanelSize;

	ImVec2 ViewTopLeftPixel;
	ImVec2 ViewSize;
	ImVec2 ViewSizeUV;

	// texture
	ImRect UV;
	ImVec2 TextureSizePixels;
	
	Transform2D TexelsToPixels;
	Transform2D PixelsToTexels;

	bool bDragging;
	std::shared_ptr<FImage> Image;
	std::filesystem::directory_entry ImageFilePath;

	// popup
	bool bPopupMenu;

	// marquee
	bool bMarqueeActive;
	bool bMarqueeVisible;
	bool bMouseInsideMarquee;
	ImRect MarqueeRect;
};
