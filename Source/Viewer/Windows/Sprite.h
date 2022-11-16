#pragma once

#include <CoreMinimal.h>

class SSprite : public SWindow
{
public:
	SSprite();
	virtual void NativeInitialize(FNativeDataInitialize Data) override;
	virtual void Initialize() override;
	virtual void Render() override;
	virtual void Destroy() override;

	// callback
	void OnDrawCallback(const ImDrawList* ParentList, const ImDrawCmd* CMD);

private:
	void UpdateShader();
	void SetScale(float scaleY);
	void SetScale(ImVec2 NewScale);
	void SetImagePosition(ImVec2 NewPosition);
	void RoundImagePosition();
	Transform2D GetTexelsToPixels(ImVec2 screenTopLeft, ImVec2 screenViewSize, ImVec2 uvTopLeft, ImVec2 uvViewSize, ImVec2 textureSize);

	// event
	void OnSelectedFileImage(const std::filesystem::directory_entry& Path);

	float TimeCounter = 0.0f;

	ID3D11Device* Device;
	ID3D11DeviceContext* DeviceContext;
	ID3D11PixelShader* PS_Grid;
	ID3D11PixelShader* PS_MarchingAnts;
	ID3D11Buffer* PCB_Grid;
	ID3D11Buffer* PCB_MarchingAnts;

	bool ForceNearestSampling = true;						// if true fragment shader will always sample from texel centers
	bool AttributeGrid = false;
	float  PremultiplyAlpha = 0.0f;							// if 1 then color will be multiplied by alpha in shader, before blend stage
	float  DisableFinalAlpha = 0.0f;						// if 1 then fragment shader will always output alpha = 1
	ImVec2 GridWidth = { 0.0f, 0.0f };						// width in UV coords of grid line
	ImVec4 GridColor = { 0.025f, 0.025f, 0.15f, 0.0f };
	ImVec4 BackgroundColor = { 0.0f, 1.0f, 0.0f, 0.0f };	// color used for alpha blending

	ImVec2 uv0;
	ImVec2 uv1;

	ImVec2 ScaleMin = { 0.02f, 0.02f };
	ImVec2 ScaleMax = { 500, 500 };

	float PixelAspectRatio = 1.0f;							// Values other than 1 not supported yet
	float MinimumGridSize = 4.0f;							// Don't draw the grid if lines would be closer than MinimumGridSize pixels

	float ZoomRate = 2.0f;									// How fast mouse wheel affects zoom

	// View State
	bool bDragging = false;									// Is user currently dragging to pan view
	
	ImVec2 ImagePosition = { 0.5f, 0.5f };					// The UV value at the center of the current view
	ImVec2 Scale = { 1.0f, 1.0f };							// 1 pixel is 1 texel
	
	ImVec2 PanelTopLeftPixel = { 0, 0 };					// Top left of view in ImGui pixel coordinates
	ImVec2 PanelSize = { 0, 0 };							// Size of area allocated to drawing the image in pixels.

	ImVec2 ViewTopLeftPixel = { 0, 0 };						// Position in ImGui pixel coordinates
	ImVec2 ViewSize = { 0, 0 };								// Rendered size of current image. This could be smaller than panel size if user has zoomed out.
	ImVec2 ViewSizeUV = { 0, 0 };							// Visible region of the texture in UV coordinates
	
	/* Conversion transforms to go back and forth between screen pixels  (what ImGui considers screen pixels) and texels*/
	Transform2D TexelsToPixels;
	Transform2D PixelsToTexels;

	std::shared_ptr<FImage> Image;
	std::shared_ptr<FImage> MarchingAnts;
};
