#pragma once

#include <CoreMinimal.h>
#include "Viewer\ViewChild.h"

class SSpriteConstructor : public SViewChild
{
public:
	SSpriteConstructor();

	virtual void NativeInitialize(FNativeDataInitialize Data) override;
	virtual void Initialize() override;
	virtual void Render() override;

	// callback
	void OnDrawCallback(const ImDrawList* ParentList, const ImDrawCmd* CMD);

private:
	void RenderSpriteList();
	void HandleKeyboardInputs();

	// direct X
	ID3D11Device* Device;
	ID3D11DeviceContext* DeviceContext;
	ID3D11PixelShader* PS_Grid;
	ID3D11Buffer* PCB_Grid;

	uint32_t ScaleVisible;
	std::shared_ptr<FImage> ImageEmpty;
};
