#pragma once

#include <CoreMinimal.h>

struct FImage
{
	FImage()
		: Texture (nullptr)
	{}

	void Draw() { ImGui::Image((void*)Texture, Size); }

	union
	{
		ImVec2 Size;
		struct
		{
			float Width;
			float Height;
		};
	};

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