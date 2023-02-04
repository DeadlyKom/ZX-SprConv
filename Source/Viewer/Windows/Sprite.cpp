#include "Sprite.h"
#include "Core\Utils.h"
#include "Core\Image.h"
#include "Viewer\Viewer.h"
#include "Viewer\Windows\ImageList.h"
#include "Core\AppFramework.h"

#define ATTRIBUTE_GRID          1 << 0
#define GRID					1 << 1
#define PIXEL_GRID              1 << 2
#define FORCE_NEAREST_SAMPLING  1 << 31

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
		int	  Flags;
		float TimeCounter;
		float BackgroundColor[3];
		int   Dummy;
		float TextureSize[2];
		float GridSize[2];
	};

	void* LINE_ID = (void*)0x10FFFFFF;
}

SSprite::SSprite()
	: Device(nullptr)
	, DeviceContext(nullptr)
	, PCB_Grid(nullptr)
	, PS_Grid(nullptr)
	, PS_LineMarchingAnts(nullptr)
	, PCB_MarchingAnts(nullptr)
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

	// 'line marching ants' pixel shader
	{
		std::string PS = FAppFramework::Get().LoadShaderResource(IDR_PS_MA_LINE);
		if (FAILED(D3DCompile(PS.c_str(), PS.size(), NULL, NULL, NULL, "main", "ps_4_0", 0, 0, &PixelShaderBlob, &ErrorBlob)))
		{
			std::string Error((const char*)ErrorBlob->GetBufferPointer());
			IMGUI_DEBUG_LOG("ImGuiTexInspect pixel shader failed. Diagnostic:\n%s\n", Error.c_str());
			ErrorBlob->Release();
			return;
		}

		if (Device->CreatePixelShader(PixelShaderBlob->GetBufferPointer(), PixelShaderBlob->GetBufferSize(), NULL, &PS_LineMarchingAnts) != S_OK)
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

	std::shared_ptr<SImageList> ImageList = GetWindow<SImageList>(EWindowsType::ImageList);
	if (ImageList)
	{
		ImageList->OnSelectedImage.AddSP(std::dynamic_pointer_cast<SSprite>(shared_from_this()), &SSprite::OnSelectedFileImage);
	}

	//// ToDo debug
	//{
	//	Image = Utils::LoadImage("C:\\Work\\Sprites\\Menu\\Change Mission\\interact - 7.png");
	//}
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

	//if (true)
	//{
	//	ImGui::Checkbox("Attribute Grid", &GetParent()->GetViewFlags().bAttributeGrid);
	//	ImGui::Checkbox("Grid", &GetParent()->GetViewFlags().bGrid);
	//	ImGui::Checkbox("Pixel Grid", &GetParent()->GetViewFlags().bPixelGrid);
	//	ImGui::DragInt2("Grid", GetParent()->GetViewFlags().GridSize, 0.3f, 1, 256);
	//}

	// calculate panel size
	const float BorderWidth = 1.0f;
	ImVec2 ContentRegionAvail = ImGui::GetContentRegionAvail();
	ImVec2 AvailablePanelSize = ContentRegionAvail - ImVec2(BorderWidth, BorderWidth) * 2.0f;

	RoundImagePosition();

	TextureSizePixels = Scale * Image->Size;					// size whole texture would appear on screen
	ImVec2 viewSizeUV = AvailablePanelSize / TextureSizePixels;		// cropped size in terms of UV
	uv0 = ImagePosition - viewSizeUV * 0.5f;
	uv1 = ImagePosition + viewSizeUV * 0.5f;

	ImVec2 DrawImageOffset{ BorderWidth, BorderWidth };
	ImVec2 viewSize = AvailablePanelSize;

	{
		/* don't crop the texture to UV [0,1] range.  What you see outside this
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

		ImGuiWindow* Window = ImGui::GetCurrentWindow();
		ImRect bb(Window->DC.CursorPos, Window->DC.CursorPos + viewSize);

		// callback for using our own image shader 
		ImGui::GetWindowDrawList()->AddCallback(DrawCallback, Image->GetShaderResourceView());
		ImGui::GetWindowDrawList()->AddImage(Image->GetShaderResourceView(), bb.Min, bb.Max, uv0, uv1);

		std::shared_ptr<SViewer> Viewer = GetParent();
		if (Viewer && Viewer->IsMarqueeTool())
		{
			InputMarquee();
		}
		if (bMarqueeVisible)
		{
			DrawMarquee(bb);
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
		//std::shared_ptr<SViewer> Viewer = GetParent();
		if (Viewer)
		{
			bDragging = Viewer->IsHandTool();
			if (bDragging)
			{
				ImVec2 uvDelta = IO.MouseDelta * viewSizeUV / viewSize;
				ImagePosition -= uvDelta;
				RoundImagePosition();
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

	if (false)
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
		ImGui::Text("StartMarqueePosition : (%f, %f)", StartMarqueePosition.x, StartMarqueePosition.y);
		ImGui::Text("EndMarqueePosition : (%f, %f)", EndMarqueePosition.x, EndMarqueePosition.y);

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
	if (PS_LineMarchingAnts)
	{
		PS_LineMarchingAnts->Release();
		PS_LineMarchingAnts = nullptr;
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

			{
				uint32_t Flags = 0;
				FViewFlags& ViewFlags = GetParent()->GetViewFlags();
				if (ViewFlags.bAttributeGrid)
				{
					Flags |= ATTRIBUTE_GRID;
				}
				if (ViewFlags.bGrid)
				{
					Flags |= GRID;
				}
				if (ViewFlags.bPixelGrid)
				{
					Flags |= PIXEL_GRID;
				}
				if (bForceNearestSampling)
				{
					Flags |= FORCE_NEAREST_SAMPLING;
				}
				ConstantBuffer->Flags = Flags;

				ConstantBuffer->GridSize[0] = float(ViewFlags.GridSize[0]);
				ConstantBuffer->GridSize[1] = float(ViewFlags.GridSize[1]);
			}
			DeviceContext->Unmap(PCB_Grid, 0);
		}
		// activate shader and buffer
		DeviceContext->PSSetShader(PS_Grid, NULL, 0);
		DeviceContext->PSSetConstantBuffers(0, 1, &PCB_Grid);
	}
	else if (CMD->UserCallbackData == Shader::LINE_ID)
	{
		if (PS_LineMarchingAnts == nullptr || PCB_MarchingAnts == nullptr)
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
			ConstantBuffer->TimeCounter = TimeCounter;
			DeviceContext->Unmap(PCB_MarchingAnts, 0);
		}
		// Activate shader and buffer
		DeviceContext->PSSetShader(PS_LineMarchingAnts, NULL, 0);
		DeviceContext->PSSetConstantBuffers(0, 1, &PCB_MarchingAnts);
	}
}

void SSprite::UpdateShader()
{
	if (Scale.y > MinimumGridSize)
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

	bForceNearestSampling = (Scale.x > 1.0f || Scale.y > 1.0f);
}

void SSprite::SetScale(ImVec2 NewScale)
{
	NewScale = ImClamp(NewScale, ScaleMin, ScaleMax);
	ViewSizeUV *= Scale / NewScale;
	Scale = NewScale;

	// only force nearest sampling if zoomed in
	bForceNearestSampling = (Scale.x > 1.0f || Scale.y > 1.0f);
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
	/* when ShowWrap mode is disabled the limits are a bit more strict. We
	 * try to keep it so that the user cannot pan past the edge of the
	 * texture at all.*/
	ImVec2 AbsViewSizeUV = Math::Abs(ViewSizeUV);
	ImagePosition = ImMax(ImagePosition - AbsViewSizeUV * 0.5f, ImVec2(0.0f, 0.0f)) + AbsViewSizeUV * 0.5f;
	ImagePosition = ImMin(ImagePosition + AbsViewSizeUV * 0.5f, ImVec2(1.0f, 1.0f)) - AbsViewSizeUV * 0.5f;

	/* if inspector->scale is 1 then we should ensure that pixels are aligned
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

ImVec2 SSprite::ConverPositionToPixel(const ImVec2& Position)
{
	const ImVec2 ImageSizeInv = ImVec2(1.0f, 1.0f) / Image->Size;
	return ImFloor((Position - ViewTopLeftPixel + uv0 / ImageSizeInv * Scale) / Scale);
}

void SSprite::InputMarquee()
{
	if (!ImGui::IsWindowFocused())
	{
		return;
	}

	const ImGuiIO& IO = ImGui::GetIO();
	const bool Shift = IO.KeyShift;
	const bool Ctrl = IO.ConfigMacOSXBehaviors ? IO.KeySuper : IO.KeyCtrl;
	const bool Alt = IO.ConfigMacOSXBehaviors ? IO.KeyCtrl : IO.KeyAlt;

	if (!bMarqueeActive && ImGui::IsKeyPressed(ImGuiKey_MouseLeft))
	{
		StartMarqueePosition = ConverPositionToPixel(ImGui::GetMousePos());
		StartMarqueePosition = ImMax(StartMarqueePosition, ImVec2(0.0f, 0.0f));
		StartMarqueePosition = ImMin(StartMarqueePosition, Image->Size * Scale);

		EndMarqueePosition = StartMarqueePosition;
		bMarqueeVisible = true;
		bMarqueeActive = true;
	}
	else if (ImGui::IsKeyReleased(ImGuiKey_MouseLeft))
	{
	}
	else if (ImGui::IsKeyDown(ImGuiKey_MouseLeft))
	{
		EndMarqueePosition = ConverPositionToPixel(ImGui::GetMousePos() + Scale);
		EndMarqueePosition = ImMax(EndMarqueePosition, ImVec2(0.0f, 0.0f));
		EndMarqueePosition = ImMin(EndMarqueePosition, Image->Size * Scale);
	}
	else
	{
		bMarqueeActive = false;
	}
}

void SSprite::DrawMarquee(const ImRect& Window)
{
	ImVec2 Start = StartMarqueePosition;
	ImVec2 End = EndMarqueePosition;
	
	// clamp
	{
		const ImVec2 ImageSizeInv = ImVec2(1.0f, 1.0f) / Image->Size;
		ImVec2 A = ImFloor(uv0 / ImageSizeInv);
		Start = (Start - A) * Scale;
		End = (End - A) * Scale;

		Start = ImMax(Start, ImVec2(0.0f, 0.0f));
		Start = ImMin(Start, Image->Size * Scale);
		End = ImMax(End, ImVec2(0.0f, 0.0f));
		End = ImMin(End, Image->Size * Scale);
	}

	ImVec2 TopLeftSubTexel = ImagePosition * Scale * Image->Size - ViewSize * 0.5f;
	ImVec2 TopLeftPixel = ViewTopLeftPixel - (TopLeftSubTexel - ImFloor(TopLeftSubTexel / Scale) * Scale);

	ImGui::GetWindowDrawList()->_FringeScale = 0.1f;
	ImGui::GetWindowDrawList()->AddCallback(DrawCallback, Shader::LINE_ID);
	ImGui::GetWindowDrawList()->AddRect(TopLeftPixel + Start, TopLeftPixel + End, ImGui::GetColorU32(ImGuiCol_Button), 0, 0, 0.001f);
}

void SSprite::OnSelectedFileImage(const std::filesystem::directory_entry& Path)
{
	if (Image != nullptr)
	{
		Image->Release();
		Image = nullptr;
	}

	Image = Utils::LoadImage(Path.path().string());
	Scale = OldScale = { 1.0f, 1.0f };
}
