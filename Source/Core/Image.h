#pragma once

#define WIN32_LEAN_AND_MEAN

#include <memory>
#include <string>
#include <vector>
#include <d3d11.h>
#include <stdint.h>
#include <windows.h>
#include "imgui.h"

struct FImage
{
	void Draw() { ImGui::Image((void*)Texture, ImVec2((float)Width, (float)Height)); }

	uint32_t Width;
	uint32_t Height;

	ID3D11ShaderResourceView* Texture;
};

class FImageBase
{
public:
	static FImageBase& Get();
	virtual ~FImageBase();
	
	std::shared_ptr<FImage> Load(std::string Filename);

protected:
	std::vector<std::shared_ptr<FImage>> ImagesInfo;
};