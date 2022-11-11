#include "Image.h"
#include "AppFramework.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb\stb_image.h"

FImageBase& FImageBase::Get()
{
	static std::shared_ptr<FImageBase> Instance(new FImageBase());
	return *Instance.get();
}

FImageBase::~FImageBase()
{}

std::shared_ptr<FImage> FImageBase::Load(std::string Filename)
{
	std::shared_ptr<FImage> Image(new FImage());

	// load from disk into a raw RGBA buffer
	uint8_t* ImageData = stbi_load(Filename.c_str(), (int*)&Image->Width, (int*)&Image->Height, NULL, 4);
	if (ImageData == nullptr)
	{
		return nullptr;
	}

	FAppFramework& Framework = FAppFramework::Get();

	// create texture
	D3D11_TEXTURE2D_DESC Texture2D;
	ZeroMemory(&Texture2D, sizeof(Texture2D));
	Texture2D.Width = Image->Width;
	Texture2D.Height = Image->Height;
	Texture2D.MipLevels = 1;
	Texture2D.ArraySize = 1;
	Texture2D.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	Texture2D.SampleDesc.Count = 1;
	Texture2D.Usage = D3D11_USAGE_DEFAULT;
	Texture2D.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	Texture2D.CPUAccessFlags = 0;

	ID3D11Texture2D* Texture = NULL;
	D3D11_SUBRESOURCE_DATA SubResource;
	SubResource.pSysMem = ImageData;
	SubResource.SysMemPitch = Texture2D.Width * 4;
	SubResource.SysMemSlicePitch = 0;
	Framework.Device->CreateTexture2D(&Texture2D, &SubResource, &Texture);

	// Create texture view
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVD;
	ZeroMemory(&SRVD, sizeof(SRVD));
	SRVD.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SRVD.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVD.Texture2D.MipLevels = Texture2D.MipLevels;
	SRVD.Texture2D.MostDetailedMip = 0;
	Framework.Device->CreateShaderResourceView(Texture, &SRVD, &Image->Texture);
	Texture->Release();

	stbi_image_free(ImageData);
	ImagesInfo.push_back(Image);
	return Image;
}
