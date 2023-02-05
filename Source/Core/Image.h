#pragma once

#include <CoreMinimal.h>

enum ECPU_AccessFlag
{
	NONE = 0x00000L,
	READ = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_READ,
	WRITE = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE
};

class FImageBase
{
public:
	static FImageBase& Get();
	virtual ~FImageBase();

	uint8_t* LoadToMemory(const std::string& Filename, ImVec2& OutSize);	// call	FreeToMemory
	bool ResizeRegion(const uint8_t* ImageData, const ImVec2& OriginalSize, const ImVec2& RequiredSize, uint8_t*& OutputImageData, const ImVec2& uv0, const ImVec2& uv1);
	void FreeToMemory(uint8_t* Data);

	std::shared_ptr<FImage> Load(const std::string& Filename);
	std::shared_ptr<FImage> FromMemory(std::vector<char> Memory);
	std::shared_ptr<FImage> CreateTexture(void* ImageData, const ImVec2& Size, UINT CPUAccessFlags = 0, D3D11_USAGE Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT);

protected:
	std::vector<std::shared_ptr<FImage>> ImagesInfo;
};

struct FImage : public FImageBase
{
	FImage();
	virtual ~FImage();

	static bool GetImageInfo(const std::string& Filename, uint32_t& OutWidth, uint32_t& OutHeight);

	void CreateTexture(void* ImageData, const ImVec2& Size, UINT CPUAccessFlags = 0, D3D11_USAGE Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT);
	void Update(void* ImageData, const ImVec2& _Size);
	void Resize(const void* InImageData, ImVec2 OriginalSize, ImVec2 RequiredSize);
	void ResizeRegion(const uint8_t* ImageData, const ImVec2& OriginalSize, const ImVec2& RequiredSize, const ImVec2& uv0, const ImVec2& uv1);
	void Release();
	void* GetShaderResourceView() const { return ShaderResourceView; }
	bool IsValid() { return ShaderResourceView != nullptr; }
	uint32_t GetLength() const { return (uint32_t)Size.x * (uint32_t)Size.y; }
	uint32_t GetFormatSize() const { return sizeof(uint32_t); }
	bool Lock(ID3D11Resource*& TextureResource, ID3D11Texture2D*& Texture, D3D11_MAPPED_SUBRESOURCE& MappedResource);
	void Unlock(ID3D11Resource*& TextureResource, ID3D11Texture2D*& Texture);

	inline bool IsValid() const { return Size.x > 0.0 && Size.y > 0.0f && ShaderResourceView != nullptr; }

	union
	{
		ImVec2 Size;
		struct
		{
			float Width;
			float Height;
		};
	};

private:
	friend FImageBase;

	UINT CPUAccessFlags;
	D3D11_USAGE Usage;

	ID3D11ShaderResourceView* ShaderResourceView;
};
