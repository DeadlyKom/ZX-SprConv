#include "Image.h"
#include "AppFramework.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb\stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STBIR_DEFAULT_FILTER_UPSAMPLE    STBIR_FILTER_BOX
#define STBIR_DEFAULT_FILTER_DOWNSAMPLE  STBIR_FILTER_BOX
#include "stb\stb_image_resize.h"

FImageBase& FImageBase::Get()
{
	static std::shared_ptr<FImageBase> Instance(new FImageBase());
	return *Instance.get();
}

FImageBase::~FImageBase()
{}

std::shared_ptr<FImage> FImageBase::Load(const std::string& Filename)
{
	std::shared_ptr<FImage> Image(new FImage());

	uint32_t Width, Height;
	// load from disk into a raw RGBA buffer
	uint8_t* ImageData = stbi_load(Filename.c_str(), (int*)&Width, (int*)&Height, NULL, 4);
	if (ImageData == nullptr)
	{
		return nullptr;
	}
	Image->Width = (float)Width;
	Image->Height = (float)Height;

	FAppFramework& Framework = FAppFramework::Get();

	// create texture
	D3D11_TEXTURE2D_DESC Texture2D;
	ZeroMemory(&Texture2D, sizeof(Texture2D));
	Texture2D.Width = Width;
	Texture2D.Height = Height;
	Texture2D.MipLevels = 1;
	Texture2D.ArraySize = 1;
	Texture2D.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	Texture2D.SampleDesc.Count = 1;
	Texture2D.Usage = D3D11_USAGE_DEFAULT;
	Texture2D.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	Texture2D.CPUAccessFlags = 0;

	ID3D11Texture2D* Texture = nullptr; 
	D3D11_SUBRESOURCE_DATA SubResource;
	SubResource.pSysMem = ImageData;
	SubResource.SysMemPitch = Width * 4;
	SubResource.SysMemSlicePitch = 0;
	Framework.Device->CreateTexture2D(&Texture2D, &SubResource, &Texture);

	// Create texture view
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVD;
	ZeroMemory(&SRVD, sizeof(SRVD));
	SRVD.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SRVD.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVD.Texture2D.MipLevels = Texture2D.MipLevels;
	SRVD.Texture2D.MostDetailedMip = 0;
	Framework.Device->CreateShaderResourceView(Texture, &SRVD, &Image->ShaderResourceView);
	Texture->Release();

	stbi_image_free(ImageData);
	ImagesInfo.push_back(Image);
	return Image;
}

uint8_t* FImageBase::LoadToMemory(const std::string& Filename, ImVec2& OutSize)
{
	uint32_t Width, Height;

	// load from disk into a raw RGBA buffer
	uint8_t* Data = stbi_load(Filename.c_str(), (int*)&Width, (int*)&Height, NULL, 4);
	OutSize = ImVec2(float(Width), float(Height));
	return Data;
}

void FImageBase::FreeToMemory(uint8_t* Data)
{
	stbi_image_free(Data);
}

bool FImageBase::ResizeRegion(const uint8_t* ImageData, const ImVec2& OriginalSize, const ImVec2& RequiredSize, uint8_t*& OutputImageData, const ImVec2& uv0, const ImVec2& uv1)
{
	if (stbir_resize_region((unsigned char*)ImageData,			(int)OriginalSize.x, (int)OriginalSize.y, 0,
							(unsigned char*)OutputImageData,	(int)RequiredSize.x, (int)RequiredSize.y, 0,
							STBIR_TYPE_UINT8,
							4, 3, 0,
							STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP,
							STBIR_FILTER_DEFAULT, STBIR_FILTER_DEFAULT,
							STBIR_COLORSPACE_SRGB,
							nullptr,
							uv0.x, uv0.y, uv1.x, uv1.y) == 0)
	{
		IM_ASSERT("issue resizing region texture\n");
		delete[] ImageData;
		return false;
	}
	return true;
}

std::shared_ptr<FImage> FImageBase::FromMemory(std::vector<char> Memory)
{
	std::shared_ptr<FImage> Image(new FImage());

	uint32_t Width, Height;
	// load from disk into a raw RGBA buffer
	uint8_t* ImageData = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(Memory.data()), (int)Memory.size(), (int*)&Width, (int*)&Height, NULL, 4);
	if (ImageData == nullptr)
	{
		return nullptr;
	}
	Image->Width = (float)Width;
	Image->Height = (float)Height;

	FAppFramework& Framework = FAppFramework::Get();

	// create texture
	D3D11_TEXTURE2D_DESC Texture2D;
	ZeroMemory(&Texture2D, sizeof(Texture2D));
	Texture2D.Width = Width;
	Texture2D.Height = Height;
	Texture2D.MipLevels = 1;
	Texture2D.ArraySize = 1;
	Texture2D.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	Texture2D.SampleDesc.Count = 1;
	Texture2D.Usage = D3D11_USAGE_DEFAULT;
	Texture2D.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	Texture2D.CPUAccessFlags = 0;

	ID3D11Texture2D* Texture = nullptr;
	D3D11_SUBRESOURCE_DATA SubResource;
	SubResource.pSysMem = ImageData;
	SubResource.SysMemPitch = Width * 4;
	SubResource.SysMemSlicePitch = 0;
	Framework.Device->CreateTexture2D(&Texture2D, &SubResource, &Texture);

	// Create texture view
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVD;
	ZeroMemory(&SRVD, sizeof(SRVD));
	SRVD.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SRVD.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVD.Texture2D.MipLevels = Texture2D.MipLevels;
	SRVD.Texture2D.MostDetailedMip = 0;
	Framework.Device->CreateShaderResourceView(Texture, &SRVD, &Image->ShaderResourceView);
	Texture->Release();

	stbi_image_free(ImageData);
	ImagesInfo.push_back(Image);
	return Image;
}

std::shared_ptr<FImage> FImageBase::CreateTexture(void* ImageData, const ImVec2& Size, UINT CPUAccessFlags /*= 0*/, D3D11_USAGE Usage /*= D3D11_USAGE::D3D11_USAGE_DEFAULT*/)
{
	std::shared_ptr<FImage> Image(new FImage());
	Image->CreateTexture(ImageData, Size, CPUAccessFlags, Usage);
	return Image;
}

FImage::FImage()
	: Size(-1.0, -1.0f)
	, CPUAccessFlags(ECPU_AccessFlag::NONE)
	, Usage(D3D11_USAGE::D3D11_USAGE_DEFAULT)
	, ShaderResourceView(nullptr)

{}

FImage::~FImage()
{
	Release();
}

bool FImage::GetImageInfo(const std::string& Filename, uint32_t& OutWidth, uint32_t& OutHeight)
{
	int Comp;
	return !!stbi_info(Filename.c_str(), (int*)&OutWidth, (int*)&OutHeight, &Comp);
}

void FImage::CreateTexture(void* ImageData, const ImVec2& InSize, UINT InCPUAccessFlags /*= 0*/, D3D11_USAGE InUsage /*= D3D11_USAGE::D3D11_USAGE_DEFAULT*/)
{
	Size = InSize;
	CPUAccessFlags = InCPUAccessFlags;
	Usage = InUsage;

	FAppFramework& Framework = FAppFramework::Get();

	// create texture
	D3D11_TEXTURE2D_DESC Texture2D;
	ZeroMemory(&Texture2D, sizeof(Texture2D));
	Texture2D.Width = (UINT)Size.x;
	Texture2D.Height = (UINT)Size.y;
	Texture2D.MipLevels = 1;
	Texture2D.ArraySize = 1;
	Texture2D.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	Texture2D.SampleDesc.Count = 1;
	Texture2D.Usage = Usage;
	Texture2D.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	Texture2D.CPUAccessFlags = (UINT)CPUAccessFlags;

	ID3D11Texture2D* Texture = nullptr;
	D3D11_SUBRESOURCE_DATA SubResource;
	SubResource.pSysMem = ImageData;
	SubResource.SysMemPitch = (UINT)Size.x * 4;
	SubResource.SysMemSlicePitch = 0;
	HRESULT Result = Framework.Device->CreateTexture2D(&Texture2D, &SubResource, &Texture);
	//check to make sure that texture is created correctly
	IM_ASSERT(SUCCEEDED(Result) && "issue creating texture\n");

	// Create texture view
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVD;
	ZeroMemory(&SRVD, sizeof(SRVD));
	SRVD.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SRVD.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVD.Texture2D.MipLevels = Texture2D.MipLevels;
	SRVD.Texture2D.MostDetailedMip = 0;
	Result = Framework.Device->CreateShaderResourceView(Texture, &SRVD, &ShaderResourceView);
	//check to make sure that resource view is created correctly
	IM_ASSERT(SUCCEEDED(Result) && "issue creating shaderResourceView \n");
	Texture->Release();
}

void FImage::Update(void* ImageData, const ImVec2& InSize)
{
	if (InSize == Size)
	{
		ID3D11Texture2D* Texture = nullptr;
		ID3D11Resource* TextureResource = nullptr;
		D3D11_MAPPED_SUBRESOURCE MappedResource;
		if (Lock(TextureResource, Texture, MappedResource))
		{
			memcpy(MappedResource.pData, ImageData, GetLength() * GetFormatSize());
			Unlock(TextureResource, Texture);
		}
		return;
	}
	Release();

	CreateTexture(ImageData, InSize, CPUAccessFlags, Usage);
}

void FImage::Resize(const void* InImageData, ImVec2 OriginalSize, ImVec2 RequiredSize)
{
	uint32_t* ImageData = new uint32_t[uint32_t(RequiredSize.x * RequiredSize.y * 4)];
	if (stbir_resize_uint8((unsigned char*)InImageData, (int)OriginalSize.x,(int)OriginalSize.y,	0,
						   (unsigned char*)ImageData,   (int)RequiredSize.x, (int)RequiredSize.y,	0,
						   4) == 0)
	{
		IM_ASSERT("issue resizing texture\n");
		delete[] ImageData;
		return;
	}
	Update(ImageData, RequiredSize);
	delete[] ImageData;
}

void FImage::ResizeRegion(const uint8_t* ImageData, const ImVec2& OriginalSize, const ImVec2& RequiredSize, const ImVec2& uv0, const ImVec2& uv1)
{
	uint8_t* ResizeImageData = new uint8_t[uint32_t(RequiredSize.x * RequiredSize.y * 4)];
	if (FImageBase::ResizeRegion(ImageData, OriginalSize, RequiredSize, ResizeImageData, uv0, uv1))
	{
		return;
	}
	Update(ResizeImageData, RequiredSize);
	delete[] ResizeImageData;
}

void FImage::Release()
{
	if (ShaderResourceView != nullptr)
	{
		ShaderResourceView->Release();
		ShaderResourceView = nullptr;
	}
}

bool FImage::Lock(ID3D11Resource*& TextureResource, ID3D11Texture2D*& Texture, D3D11_MAPPED_SUBRESOURCE& MappedResource)
{
	if (ShaderResourceView == nullptr)
	{
		return false;
	}

	ShaderResourceView->GetResource(&TextureResource);
	if (FAILED(TextureResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&Texture))))
	{
		return false;
	}

	return FAppFramework::Get().DeviceContext->Map(Texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource) == S_OK;
}

void FImage::Unlock(ID3D11Resource*& TextureResource, ID3D11Texture2D*& Texture)
{
	if (Texture != nullptr)
	{
		FAppFramework::Get().DeviceContext->Unmap(Texture, 0);
		Texture->Release();
	}

	if (TextureResource != nullptr)
	{
		TextureResource->Release();
	}
}
