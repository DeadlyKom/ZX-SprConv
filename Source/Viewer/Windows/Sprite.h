#pragma once

#include <CoreMinimal.h>
#include "Viewer\ViewChild.h"

class SSprite : public SViewChild
{
public:
	SSprite();
	virtual void NativeInitialize(FNativeDataInitialize Data) override;
	virtual void Initialize() override;
	virtual void Render() override;
	virtual void Tick(float DeltaTime);
	virtual void Destroy() override;

	// callback
	void OnDrawCallback(const ImDrawList* ParentList, const ImDrawCmd* CMD);

private:
	void UpdateShader();
	void SetScale(float scaleY);
	void SetScale(ImVec2 NewScale);
	void SetImagePosition(ImVec2 NewPosition);
	void RoundImagePosition();
	void ChangeScale();
	Transform2D GetTexelsToPixels(ImVec2 screenTopLeft, ImVec2 screenViewSize, ImVec2 uvTopLeft, ImVec2 uvViewSize, ImVec2 textureSize);
	ImVec2 ConverPositionToPixel(const ImVec2& Position);
	void InputMarquee();
	void DrawMarquee(const ImRect& Window);

	// event
	void OnSelectedFileImage(const std::filesystem::directory_entry& Path);

	float TimeCounter = 0.0f;

	ID3D11Device* Device;
	ID3D11DeviceContext* DeviceContext;
	ID3D11PixelShader* PS_Grid;
	ID3D11PixelShader* PS_LineMarchingAnts;
	ID3D11Buffer* PCB_Grid;
	ID3D11Buffer* PCB_MarchingAnts;

	bool bForceNearestSampling = true;						// if true fragment shader will always sample from texel centers
	float  PremultiplyAlpha = 0.0f;							// if 1 then color will be multiplied by alpha in shader, before blend stage
	float  DisableFinalAlpha = 0.0f;						// if 1 then fragment shader will always output alpha = 1
	ImVec2 GridWidth = { 0.0f, 0.0f };						// width in UV coords of grid line
	ImVec4 GridColor = { 0.025f, 0.025f, 0.15f, 0.0f };
	ImVec4 BackgroundColor = { 0.0f, 1.0f, 0.0f, 0.0f };	// color used for alpha blending

	ImVec2 ScaleMin = { 0.03125f, 0.03125f };
	ImVec2 ScaleMax = { 32, 32 };

	float PixelAspectRatio = 1.0f;							// values other than 1 not supported yet
	float MinimumGridSize = 4.0f;							// don't draw the grid if lines would be closer than MinimumGridSize pixels

	float ZoomRate = 2.0f;									// how fast mouse wheel affects zoom

	// view state
	bool bDragging = false;									// is user currently dragging to pan view
	
	ImVec2 ImagePosition = { 0.5f, 0.5f };					// the UV value at the center of the current view
	ImVec2 Scale = { 1.0f, 1.0f };							// 1 pixel is 1 texel
	ImVec2 OldScale = { 1.0f, 1.0f };
	
	ImVec2 PanelTopLeftPixel = { 0.0f, 0.0f };				// top left of view in ImGui pixel coordinates
	ImVec2 PanelSize = { 0.0f, 0.0f };						// size of area allocated to drawing the image in pixels.

	ImVec2 ViewTopLeftPixel = { 0.0f, 0.0f };				// position in ImGui pixel coordinates
	ImVec2 ViewSize = { 0.0f, 0.0f };						// rendered size of current image. This could be smaller than panel size if user has zoomed out.
	ImVec2 ViewSizeUV = { 0.0f, 0.0f };						// visible region of the texture in UV coordinates

	//
	ImVec2 TextureSizePixels;
	ImVec2 uv0;
	ImVec2 uv1;

	//
	bool bMarqueeActive = false;
	bool bMarqueeVisible = false;
	ImVec2 StartMarqueePosition;
	ImVec2 EndMarqueePosition;
	
	/* conversion transforms to go back and forth between screen pixels  (what ImGui considers screen pixels) and texels*/
	Transform2D TexelsToPixels;
	Transform2D PixelsToTexels;

	std::shared_ptr<FImage> Image;
};
