#include "Sprite.h"
#include "Core\Utils.h"
#include "Core\Image.h"
#include "Viewer\Viewer.h"
#include "Viewer\Windows\ImageList.h"
#include "Core\AppFramework.h"
#include "..\ZX-Convert\Resource.h"

namespace
{
	void DrawCallback(const ImDrawList* ParentList, const ImDrawCmd* CMD)
	{
		std::shared_ptr<SViewer> Viewer = FAppFramework::Get().GetViewer();
		std::shared_ptr<SSprite> Sprite = Viewer ? std::dynamic_pointer_cast<SSprite>(Viewer->GetWindow(EWindowsType::Sprite)) : nullptr;
		if (Sprite)
		{
			Sprite->OnDrawCallback(ParentList, CMD);
		}
	}
}

namespace Shader
{
	/* Constant buffer used in pixel shader. Size must be multiple of 16 bytes.
	 * Layout must match the layout in the pixel shader. */
	struct PIXEL_CONSTANT_BUFFER
	{
		float GridColor[4];
		float GridWidth[2];
		bool  AttributeGrid;
		float TimeCounter;
		float BackgroundColor[3];
		bool  ForceNearestSampling;
		float TextureSize[2];
		float Dummy[2];
	};
}

SSprite::SSprite()
	: Device(nullptr)
	, DeviceContext(nullptr)
	, PCB_Grid(nullptr)
	, PS_MarchingAnts(nullptr)
	, PS_Grid(nullptr)
	, PCB_MarchingAnts(nullptr)
	, MarchingAntsData(nullptr)
{}

void SSprite::NativeInitialize(FNativeDataInitialize Data)
{
	SWindow::NativeInitialize(Data);

	// save pointers to DirectX device and context
	Device = Data.Device;
	Device->AddRef();
	DeviceContext = Data.DeviceContext;
	DeviceContext->AddRef();

	//
	ID3DBlob* PixelShaderBlob;
	ID3DBlob* ErrorBlob;

	// 'grid' pixel shader
	{
		const std::string PS = FAppFramework::Get().LoadShaderResource(IDR_PS_GRID);
		if (FAILED(D3DCompile(PS.c_str(), PS.size(), NULL, NULL, NULL, "main", "ps_4_0", 0, 0, &PixelShaderBlob, &ErrorBlob)))
		{
			std::string Error((const char*)ErrorBlob->GetBufferPointer());
			IMGUI_DEBUG_LOG("ImGuiTexInspect pixel shader failed. Diagnostic:\n%s\n", Error.c_str());
			ErrorBlob->Release();
			return;
		}

		if (Device->CreatePixelShader(PixelShaderBlob->GetBufferPointer(), PixelShaderBlob->GetBufferSize(), NULL, &PS_Grid) != S_OK)
		{
			PixelShaderBlob->Release();
			return;
		}
		PixelShaderBlob->Release();

		// create pixel shader constant buffer
		{
			D3D11_BUFFER_DESC desc;
			desc.ByteWidth = sizeof(Shader::PIXEL_CONSTANT_BUFFER);
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			Device->CreateBuffer(&desc, NULL, &PCB_Grid);
		}
	}

	// 'marching ants' pixel shader
	{
		std::string PS = FAppFramework::Get().LoadShaderResource(IDR_PS_MARCHING_ANTS);
		if (FAILED(D3DCompile(PS.c_str(), PS.size(), NULL, NULL, NULL, "main", "ps_4_0", 0, 0, &PixelShaderBlob, &ErrorBlob)))
		{
			std::string Error((const char*)ErrorBlob->GetBufferPointer());
			IMGUI_DEBUG_LOG("ImGuiTexInspect pixel shader failed. Diagnostic:\n%s\n", Error.c_str());
			ErrorBlob->Release();
			return;
		}

		if (Device->CreatePixelShader(PixelShaderBlob->GetBufferPointer(), PixelShaderBlob->GetBufferSize(), NULL, &PS_MarchingAnts) != S_OK)
		{
			PixelShaderBlob->Release();
			return;
		}
		PixelShaderBlob->Release();

		// create pixel shader constant buffer
		{
			D3D11_BUFFER_DESC desc;
			desc.ByteWidth = sizeof(Shader::PIXEL_CONSTANT_BUFFER);
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			Device->CreateBuffer(&desc, NULL, &PCB_MarchingAnts);
		}
	}
}

void SSprite::Initialize()
{
	bIncludeInWindows = true;
	Name = "Sprite Editor";

	std::shared_ptr<SViewer> Viewer = std::dynamic_pointer_cast<SViewer>(Parent);
	std::shared_ptr<SImageList> ImageList = Viewer ? std::dynamic_pointer_cast<SImageList>(Viewer->GetWindow(EWindowsType::ImageList)) : nullptr;
	if (ImageList)
	{
		ImageList->OnSelectedImage.AddSP(std::dynamic_pointer_cast<SSprite>(shared_from_this()), &SSprite::OnSelectedFileImage);
	}

	// ToDo debug
	{
		Image = Utils::LoadImage("C:\\Work\\Sprites\\Menu\\Change Mission\\interact - 7.png");
		//MarchingAnts = Utils::LoadImage("C:\\Work\\Sprites\\Menu\\Change Mission\\interact - 7_MA.png");
		if (Image != nullptr)
		{
			MarchingAntsData = new uint32_t[Image->GetLength()];
			ZeroMemory(MarchingAntsData, Image->GetLength() * Image->GetFormatSize());
			CreateTextureMA();
		}
	}
}

void SSprite::Render()
{
	if (!IsOpen())
	{
		Close();
		return;
	}

	if (Image == nullptr)
	{
		return;
	}

	ImVec2 Size(0.0f, 0.0f);
	ImGui::Begin("Sprite Editor", &bOpen, ImGuiWindowFlags_HorizontalScrollbar);
	ImGui::Checkbox("Attribute Grid", &AttributeGrid);

	// calculate panel size
	const float BorderWidth = 1.0f;
	ImVec2 ContentRegionAvail = ImGui::GetContentRegionAvail();
	ImVec2 AvailablePanelSize = ContentRegionAvail - ImVec2(BorderWidth, BorderWidth) * 2.0f;

	RoundImagePosition();

	ImVec2 TextureSizePixels = Scale * Image->Size;					// size whole texture would appear on screen
	ImVec2 viewSizeUV = AvailablePanelSize / TextureSizePixels;		// cropped size in terms of UV
	ImVec2 uv0 = ImagePosition - viewSizeUV * 0.5f;
	ImVec2 uv1 = ImagePosition + viewSizeUV * 0.5f;

	ImVec2 DrawImageOffset{ BorderWidth, BorderWidth };
	ImVec2 viewSize = AvailablePanelSize;

	{
		/* Don't crop the texture to UV [0,1] range.  What you see outside this
		 * range will depend on API and texture properties */
		if (TextureSizePixels.x < AvailablePanelSize.x)
		{
			// not big enough to horizontally fill view
			viewSize.x = ImFloor(TextureSizePixels.x);
			DrawImageOffset.x += ImFloor((AvailablePanelSize.x - TextureSizePixels.x) * 0.5f);
			uv0.x = 0.0f;
			uv1.x = 1.0f;
			viewSizeUV.x = 1.0f;
			ImagePosition.x = 0.5f;
		}
		if (TextureSizePixels.y < AvailablePanelSize.y)
		{
			// not big enough to vertically fill view
			viewSize.y = ImFloor(TextureSizePixels.y);
			DrawImageOffset.y += ImFloor((AvailablePanelSize.y - TextureSizePixels.y) * 0.5f);
			uv0.y = 0.0f;
			uv1.y = 1.0f;
			viewSizeUV.y = 1.0f;
			ImagePosition.y = 0.5f;
		}
	}

	ViewSize = viewSize;
	ViewSizeUV = viewSizeUV;

	{
		ChangeScale();

		// see comment above
		ImGui::GetCurrentWindow()->ScrollMax.y = 1.0f;

		// keep track of size of area that we draw for borders later
		PanelTopLeftPixel = ImGui::GetCursorScreenPos();
		ImGui::SetCursorPos(ImGui::GetCursorPos() + DrawImageOffset);
		ViewTopLeftPixel = ImGui::GetCursorScreenPos();

		UpdateShader();

		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImRect bb(window->DC.CursorPos, window->DC.CursorPos + viewSize);

		// callback for using our own image shader 
		ImGui::GetWindowDrawList()->AddCallback(DrawCallback, Image->GetShaderResourceView());
		ImGui::GetWindowDrawList()->AddImage(Image->GetShaderResourceView(), bb.Min, bb.Max, uv0, uv1);

		if (MarchingAnts != nullptr)
		{
			// callback for using our own image shader 
			ImGui::GetWindowDrawList()->AddCallback(DrawCallback, MarchingAnts->GetShaderResourceView());
			ImGui::GetWindowDrawList()->AddImage(MarchingAnts->GetShaderResourceView(), bb.Min, bb.Max, uv0, uv1);
		}

		// reset callback for using our own image shader 
		ImGui::GetWindowDrawList()->AddCallback(ImDrawCallback_ResetRenderState, nullptr);

		const ImGuiIO& IO = ImGui::GetIO();

		TexelsToPixels = GetTexelsToPixels(ViewTopLeftPixel, viewSize, uv0, viewSizeUV, Image->Size);
		PixelsToTexels = TexelsToPixels.Inverse();

		ImVec2 MousePosition = ImGui::GetMousePos();
		ImVec2 MousePositionTexel = PixelsToTexels * MousePosition;
		ImVec2 MouseUV = MousePositionTexel / Image->Size;
		MousePositionTexel.x = Math::Modulus(MousePositionTexel.x, Image->Size.x);
		MousePositionTexel.y = Math::Modulus(MousePositionTexel.y, Image->Size.y);

		const bool Hovered = ImGui::IsWindowHovered();

		// dragging
		{
			// start drag
			if (!bDragging && Hovered && IO.MouseClicked[ImGuiMouseButton_Left])
			{
				bDragging = true;
			}
			// carry on dragging
			else if (bDragging)
			{
				ImVec2 uvDelta = IO.MouseDelta * viewSizeUV / viewSize;
				ImagePosition -= uvDelta;
				RoundImagePosition();
			}

			// end drag
			if (bDragging && (IO.MouseReleased[ImGuiMouseButton_Left] || !IO.MouseDown[ImGuiMouseButton_Left]))
			{
				bDragging = false;
			}
		}

		if (Hovered && IO.MouseWheel != 0.0f)
		{
			float LocalScale = Scale.y;
			float PrevScale = LocalScale;

			bool keepTexelSizeRegular = LocalScale > MinimumGridSize;
			if (IO.MouseWheel > 0.0f)
			{
				LocalScale *= ZoomRate;
				if (keepTexelSizeRegular)
				{
					// it looks nicer when all the grid cells are the same size
					// so keep scale integer when zoomed in
					LocalScale = ImCeil(LocalScale);
				}
			}
			else
			{
				LocalScale /= ZoomRate;
				if (keepTexelSizeRegular)
				{
					// see comment above. We're doing a floor this time to make
					// sure the scale always changes when scrolling
					LocalScale = Math::ImFloorSigned(LocalScale);
				}
			}
			/* to make it easy to get back to 1:1 size we ensure that we stop
			 * here without going straight past it*/
			if ((PrevScale < 1.0f && LocalScale > 1.0f) || (PrevScale > 1.0f && LocalScale < 1.0f))
			{
				LocalScale = 1.0f;
			}
			SetScale(ImVec2(PixelAspectRatio * LocalScale, LocalScale));
			SetImagePosition(ImagePosition + (MouseUV - ImagePosition) * (1.0f - PrevScale / LocalScale));
		}
	}

	{
		ImGui::Begin("Debug");

		ImGui::Text("PixelAspectRatio : %f", PixelAspectRatio);
		ImGui::Text("MinimumGridSize : %f", MinimumGridSize);
		ImGui::Text("ZoomRate : %f", ZoomRate);
		ImGui::Text("bDragging : %i", bDragging);

		ImGui::Text("ContentRegionAvail : (%f, %f)", ContentRegionAvail.x, ContentRegionAvail.y);
		ImGui::Text("ImagePosition : (%f, %f)", ImagePosition.x, ImagePosition.y);
		ImGui::Text("Scale : (%f, %f)", Scale.x, Scale.y);
		ImGui::Text("PanelTopLeftPixel : (%f, %f)", PanelTopLeftPixel.x, PanelTopLeftPixel.y);
		ImGui::Text("PanelSize : (%f, %f)", PanelSize.x, PanelSize.y);
		ImGui::Text("ViewTopLeftPixel : (%f, %f)", ViewTopLeftPixel.x, ViewTopLeftPixel.y);
		ImGui::Text("ViewSize : (%f, %f)", ViewSize.x, ViewSize.y);
		ImGui::Text("ViewSizeUV : (%f, %f)", ViewSizeUV.x, ViewSizeUV.y);
		ImGui::Text("UV0 : (%f, %f)", uv0.x, uv0.y);
		ImGui::Text("UV1 : (%f, %f)", uv1.x, uv1.y);
		
		ImGui::End();
	}

	ImGui::End();
}

void SSprite::Tick(float DeltaTime)
{
	TimeCounter += DeltaTime;
}

void SSprite::Destroy()
{
	if (Device)
	{
		Device->Release();
		Device = nullptr;
	}
	if (DeviceContext)
	{
		DeviceContext->Release();
		DeviceContext = nullptr;
	}
	if (PCB_Grid)
	{
		PCB_Grid->Release();
		PCB_Grid = nullptr;
	}
	if (PS_Grid)
	{
		PS_Grid->Release();
		PS_Grid = nullptr;
	}
	if (PCB_MarchingAnts)
	{
		PCB_MarchingAnts->Release();
		PCB_MarchingAnts = nullptr;
	}
	if (PS_MarchingAnts)
	{
		PS_MarchingAnts->Release();
		PS_MarchingAnts = nullptr;
	}
	if (MarchingAntsData != nullptr)
	{
		delete[] MarchingAntsData;
	}
}

void SSprite::OnDrawCallback(const ImDrawList* ParentList, const ImDrawCmd* CMD)
{
	if (Image == nullptr || DeviceContext == nullptr)
	{
		return;
	}

	if (CMD->UserCallbackData == Image->GetShaderResourceView())
	{
		if (PS_Grid == nullptr || PCB_Grid == nullptr)
		{
			return;
		}

		// map the pixel shader constant buffer and fill values
		{
			D3D11_MAPPED_SUBRESOURCE MappedResource;
			if (DeviceContext->Map(PCB_Grid, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource) != S_OK)
			{
				return;
			}
			// transfer shader options from shaderOptions to our backend specific pixel shader constant buffer
			Shader::PIXEL_CONSTANT_BUFFER* ConstantBuffer = (Shader::PIXEL_CONSTANT_BUFFER*)MappedResource.pData;
			memcpy(ConstantBuffer->GridColor, &GridColor, sizeof(GridColor));
			memcpy(ConstantBuffer->GridWidth, &GridWidth, sizeof(GridWidth));
			memcpy(ConstantBuffer->BackgroundColor, &BackgroundColor, sizeof(float) * 3);
			memcpy(ConstantBuffer->TextureSize, &Image->Size, sizeof(Image->Size));
			ConstantBuffer->AttributeGrid = AttributeGrid;
			ConstantBuffer->ForceNearestSampling = ForceNearestSampling;
			DeviceContext->Unmap(PCB_Grid, 0);
		}
		// Activate shader and buffer
		DeviceContext->PSSetShader(PS_Grid, NULL, 0);
		DeviceContext->PSSetConstantBuffers(0, 1, &PCB_Grid);
	}
	else if (CMD->UserCallbackData == MarchingAnts->GetShaderResourceView())
	{
		if (PS_MarchingAnts == nullptr || PCB_MarchingAnts == nullptr)
		{
			return;
		}

		// map the pixel shader constant buffer and fill values
		{
			D3D11_MAPPED_SUBRESOURCE MappedResource;
			if (DeviceContext->Map(PCB_MarchingAnts, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource) != S_OK)
			{
				return;
			}
			// transfer shader options from shaderOptions to our backend specific pixel shader constant buffer
			Shader::PIXEL_CONSTANT_BUFFER* ConstantBuffer = (Shader::PIXEL_CONSTANT_BUFFER*)MappedResource.pData;
			memcpy(ConstantBuffer->GridColor, &GridColor, sizeof(GridColor));
			memcpy(ConstantBuffer->GridWidth, &GridWidth, sizeof(GridWidth));
			memcpy(ConstantBuffer->BackgroundColor, &BackgroundColor, sizeof(float) * 3);
			memcpy(ConstantBuffer->TextureSize, &Image->Size, sizeof(Image->Size));
			ConstantBuffer->AttributeGrid = AttributeGrid;
			ConstantBuffer->TimeCounter = TimeCounter;
			ConstantBuffer->ForceNearestSampling = ForceNearestSampling;
			DeviceContext->Unmap(PCB_MarchingAnts, 0);
		}
		// Activate shader and buffer
		DeviceContext->PSSetShader(PS_MarchingAnts, NULL, 0);
		DeviceContext->PSSetConstantBuffers(0, 1, &PCB_MarchingAnts);
	}
}

void SSprite::UpdateShader()
{
	if (/*HasFlag(inspector->Flags, InspectorFlags_NoGrid) == false &&*/ Scale.y > MinimumGridSize)
	{
		// enable grid in shader
		GridColor.w = 1.0f;
		SetScale(Math::Round(Scale.y));
	}
	else
	{
		// disable grid in shader
		GridColor.w = 0.0f;
	}

	ForceNearestSampling = (Scale.x > 1.0f || Scale.y > 1.0f);
}

void SSprite::SetScale(ImVec2 NewScale)
{
	NewScale = ImClamp(NewScale, ScaleMin, ScaleMax);
	ViewSizeUV *= Scale / NewScale;
	Scale = NewScale;

	// only force nearest sampling if zoomed in
	ForceNearestSampling = (Scale.x > 1.0f || Scale.y > 1.0f);
	GridWidth = ImVec2(1.0f / Scale.x, 1.0f / Scale.y);
}

void SSprite::SetScale(float scaleY)
{
	SetScale(ImVec2(scaleY * PixelAspectRatio, scaleY));
}

void SSprite::SetImagePosition(ImVec2 NewPosition)
{
	ImagePosition = NewPosition;
	RoundImagePosition();
}

void SSprite::RoundImagePosition()
{
	if (Image == nullptr)
	{
		return;
	}
	/* When ShowWrap mode is disabled the limits are a bit more strict. We
	 * try to keep it so that the user cannot pan past the edge of the
	 * texture at all.*/
	ImVec2 AbsViewSizeUV = Math::Abs(ViewSizeUV);
	ImagePosition = ImMax(ImagePosition - AbsViewSizeUV * 0.5f, ImVec2(0.0f, 0.0f)) + AbsViewSizeUV * 0.5f;
	ImagePosition = ImMin(ImagePosition + AbsViewSizeUV * 0.5f, ImVec2(1.0f, 1.0f)) - AbsViewSizeUV * 0.5f;

	/* If inspector->scale is 1 then we should ensure that pixels are aligned
	 * with texel centers to get pixel-perfect texture rendering*/
	ImVec2 TopLeftSubTexel = ImagePosition * Scale * Image->Size - ViewSize * 0.5f;

	if (Scale.x >= 1.0f)
	{
		TopLeftSubTexel.x = Math::Round(TopLeftSubTexel.x);
	}
	if (Scale.y >= 1.0f)
	{
		TopLeftSubTexel.y = Math::Round(TopLeftSubTexel.y);
	}
	ImagePosition = (TopLeftSubTexel + ViewSize * 0.5f) / (Scale * Image->Size);
}

void SSprite::ChangeScale()
{
	if (Scale == OldScale)
	{
		return;
	}
	OldScale = Scale;

	UpdateTextureMA();
}

void SSprite::CreateTextureMA()
{
	if (MarchingAnts == nullptr)
	{
		const uint32_t Width = (uint32_t)Image->Width;
		const uint32_t Height = (uint32_t)Image->Height;
		
		// fill
		ImVec2 Size = { 32, 32 };
		ImVec2 Position = { 64, 32 };
		for (uint32_t y = (uint32_t)Position.y; y < (uint32_t)(Position.y + Size.y); ++y)
		{
			for (uint32_t x = (uint32_t)Position.x; x < (uint32_t)(Position.x + Size.x); ++x)
			{
				MarchingAntsData[y * Width + x] = ImColor(1.0f, 1.0f, 1.0f, 1.0f);
			}
		}

		//
		uint32_t* ImageData = new uint32_t[Width * Height];
		for (uint32_t y = 0; y < Height; ++y)
		{
			for (uint32_t x = 0; x < Width; ++x)
			{
				ImU32& Color = MarchingAntsData[y * Width + x];
				ImageData[y * Width + x] = (ImU32)Color;
			}
		}

		MarchingAnts = FImageBase::Get().CreateTexture(ImageData, Image->Size, D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE, D3D11_USAGE::D3D11_USAGE_DYNAMIC);
		delete[] ImageData;
	}
}

void SSprite::UpdateTextureMA()
{
	if (MarchingAnts == nullptr)
	{
		return;
	}

	MarchingAnts->Resize(MarchingAntsData, Image->Size, Image->Size * Scale);
}

Transform2D SSprite::GetTexelsToPixels(ImVec2 screenTopLeft, ImVec2 screenViewSize, ImVec2 uvTopLeft, ImVec2 uvViewSize, ImVec2 textureSize)
{
	ImVec2 uvToPixel = screenViewSize / uvViewSize;

	Transform2D transform;
	transform.Scale = uvToPixel / textureSize;
	transform.Translate.x = screenTopLeft.x - uvTopLeft.x * uvToPixel.x;
	transform.Translate.y = screenTopLeft.y - uvTopLeft.y * uvToPixel.y;
	return transform;
}

void SSprite::OnSelectedFileImage(const std::filesystem::directory_entry& Path)
{
	if (Image != nullptr)
	{
		Image->Release();
		Image = nullptr;
	}
	if (MarchingAnts != nullptr)
	{
		MarchingAnts->Release();
		MarchingAnts = nullptr;
	}
	if (MarchingAntsData != nullptr)
	{
		delete[] MarchingAntsData;
		MarchingAntsData = nullptr;
	}

	Image = Utils::LoadImage(Path.path().string());
	if (Image != nullptr)
	{
		MarchingAntsData = new uint32_t[Image->GetLength()];
		ZeroMemory(MarchingAntsData, Image->GetLength() * Image->GetFormatSize());
		CreateTextureMA();
	}

	Scale = OldScale = { 1.0f, 1.0f };
}
