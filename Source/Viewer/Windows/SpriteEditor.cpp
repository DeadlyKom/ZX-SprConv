#include "SpriteEditor.h"
#include "Core\Utils.h"
#include "Core\Image.h"
#include "Core\AppFramework.h"

#include "Viewer\Viewer.h"
#include "Viewer\Windows\Tools.h"
#include "Viewer\Windows\ImageList.h"

#define ATTRIBUTE_GRID          1 << 0
#define GRID					1 << 1
#define PIXEL_GRID              1 << 2
#define FORCE_NEAREST_SAMPLING  1 << 31

namespace
{
	void DrawCallback(const ImDrawList* ParentList, const ImDrawCmd* CMD)
	{
		std::shared_ptr<SViewer> Viewer = FAppFramework::Get().GetViewer();
		std::shared_ptr<SSpriteEditor> SpriteEditor = Viewer ? std::dynamic_pointer_cast<SSpriteEditor>(Viewer->GetWindow(EWindowsType::SpriteEditor)) : nullptr;
		if (SpriteEditor)
		{
			SpriteEditor->OnDrawCallback(ParentList, CMD);
		}
	}

	const char* PopupMenuName = "PopupMenuSprite";
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
		int   Dummy_0;
		float TextureSize[2];
		float GridSize[2];
		float GridOffset[2];

		float Dummy[46];
	};

	void* LINE_ID = (void*)0x10FFFFFF;
}

SSpriteEditor::SSpriteEditor()
	// directX
	: Device(nullptr)
	, DeviceContext(nullptr)
	, PS_Grid(nullptr)
	, PS_LineMarchingAnts(nullptr)
	, PCB_Grid(nullptr)
	, PCB_MarchingAnts(nullptr)
	
	// shader variable
	, TimeCounter(0.0f)
	, bForceNearestSampling(true)							// if true fragment shader will always sample from texel centers
	, GridWidth(0.0f, 0.0f)									// width in UV coords of grid line
	, GridSize(0.0f, 0.0f)
	, GridOffset(0.0f, 0.0f)
	, GridColor(0.025f, 0.025f, 0.15f, 0.0f)
	, BackgroundColor(0.0f, 1.0f, 0.0f, 0.0f)				// color used for alpha blending
	
	// scale
	, ZoomRate(2.0f)										// how fast mouse wheel affects zoom
	, Scale(4.0f, 4.0f)										// 1 pixel is 1 texel
	, OldScale(4.0f, 4.0f)
	, ScaleMin(1.0f / 32.0f, 1.0f / 32.0f)
	, ScaleMax(32.0f, 32.0f)
	, PixelAspectRatio(1.0f)								// values other than 1 not supported yet
	, MinimumGridSize(4.0f)									// don't draw the grid if lines would be closer than MinimumGridSize pixels

	// view state	
	, ImagePosition(0.5f, 0.5f)								// the UV value at the center of the current view
	, PanelTopLeftPixel(0.0f, 0.0)							// top left of view in ImGui pixel coordinates
	, PanelSize(0.0f, 0.0f)									// size of area allocated to drawing the image in pixels.
	, ViewTopLeftPixel(0.0f, 0.0f)							// position in ImGui pixel coordinates
	, ViewSize(0.0f, 0.0f)									// rendered size of current image. This could be smaller than panel size if user has zoomed out.
	, ViewSizeUV(0.0f, 0.0f)								// visible region of the texture in UV coordinates

	// texture
	, UV(0.0f, 0.0f, 0.0f, 0.0f)
	, TextureSizePixels(0.0f, 0.0f)

	// conversion transforms to go back and forth between screen pixels  (what ImGui considers screen pixels) and texels
	, TexelsToPixels()
	, PixelsToTexels()

	, bDragging(false)										// is user currently dragging to pan view
	, Image(nullptr)
	
	// popup
	, bPopupMenu(false)

	// marquee
	, bMarqueeActive(false)
	, bMarqueeVisible(false)
	, bMouseInsideMarquee(false)
	, MarqueeRect(0.0f, 0.0f, 0.0f, 0.0f)
{}

void SSpriteEditor::NativeInitialize(FNativeDataInitialize Data)
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
			LOG_ERROR("ImGuiTexInspect pixel shader failed. Diagnostic:\n%s", Error.c_str());
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
			LOG_ERROR("ImGuiTexInspect pixel shader failed. Diagnostic:\n%s", Error.c_str());
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

void SSpriteEditor::Initialize()
{
	bIncludeInWindows = true;
	Name = "Sprite Editor";

	std::shared_ptr<SImageList> ImageList = GetWindow<SImageList>(EWindowsType::ImageList);
	if (ImageList)
	{
		ImageList->OnSelectedImage.AddSP(std::dynamic_pointer_cast<SSpriteEditor>(shared_from_this()), &SSpriteEditor::OnSelectedFileImage);
	}
}

void SSpriteEditor::Render()
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

	ImGui::Begin("SpriteEditor", &bOpen, ImGuiWindowFlags_HorizontalScrollbar);

	HandleMouseInputs();
	HandleKeyboardInputs();
	HandleMarqueeInput();
	ImGui::PushAllowKeyboardFocus(true);
	RenderEditorSprite();
	RenderPopupMenu();
	ImGui::PopAllowKeyboardFocus();

	ImGui::End();

	//if (true)
	//{
	//	ImGui::Checkbox("Attribute Grid", &GetParent()->GetViewFlags().bAttributeGrid);
	//	ImGui::Checkbox("Grid", &GetParent()->GetViewFlags().bGrid);
	//	ImGui::Checkbox("Pixel Grid", &GetParent()->GetViewFlags().bPixelGrid);
	//	ImGui::DragInt2("Grid", GetParent()->GetViewFlags().GridSize, 0.3f, 1, 256);
	//}

	//if (false)
	//{
	//	ImGui::Begin("Debug");
	//	ImGui::Text("PixelAspectRatio : %f", PixelAspectRatio);
	//	ImGui::Text("MinimumGridSize : %f", MinimumGridSize);
	//	ImGui::Text("ZoomRate : %f", ZoomRate);
	//	ImGui::Text("bDragging : %i", bDragging);
	//	ImGui::Text("ContentRegionAvail : (%f, %f)", ContentRegionAvail.x, ContentRegionAvail.y);
	//	ImGui::Text("ImagePosition : (%f, %f)", ImagePosition.x, ImagePosition.y);
	//	ImGui::Text("Scale : (%f, %f)", Scale.x, Scale.y);
	//	ImGui::Text("PanelTopLeftPixel : (%f, %f)", PanelTopLeftPixel.x, PanelTopLeftPixel.y);
	//	ImGui::Text("PanelSize : (%f, %f)", PanelSize.x, PanelSize.y);
	//	ImGui::Text("ViewTopLeftPixel : (%f, %f)", ViewTopLeftPixel.x, ViewTopLeftPixel.y);
	//	ImGui::Text("ViewSize : (%f, %f)", ViewSize.x, ViewSize.y);
	//	ImGui::Text("ViewSizeUV : (%f, %f)", ViewSizeUV.x, ViewSizeUV.y);
	//	ImGui::Text("UV0 : (%f, %f)", UV.Min.x, UV.Min.y);
	//	ImGui::Text("UV1 : (%f, %f)", UV.Max.x, UV.Max.y);
	//	ImGui::Text("StartMarqueePosition : (%f, %f)", StartMarqueePosition.x, StartMarqueePosition.y);
	//	ImGui::Text("EndMarqueePosition : (%f, %f)", EndMarqueePosition.x, EndMarqueePosition.y);	
	//	ImGui::End();
	//}
}

void SSpriteEditor::Tick(float DeltaTime)
{
	TimeCounter += DeltaTime;
}

void SSpriteEditor::Destroy()
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

void SSpriteEditor::OnDrawCallback(const ImDrawList* ParentList, const ImDrawCmd* CMD)
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

				ConstantBuffer->GridSize[0] = ViewFlags.GridSettingSize.x;
				ConstantBuffer->GridSize[1] = ViewFlags.GridSettingSize.y;
				ConstantBuffer->GridOffset[0] = ViewFlags.GridSettingOffset.x;
				ConstantBuffer->GridOffset[1] = ViewFlags.GridSettingOffset.y;
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

void SSpriteEditor::RenderEditorSprite()
{
	// update shader variable
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

	ImGuiWindow* Window = ImGui::GetCurrentWindow();

	// see comment above
	Window->ScrollMax.y = 1.0f;

	// keep track of size of area that we draw for borders later
	PanelTopLeftPixel = ImGui::GetCursorScreenPos();
	ImGui::SetCursorPos(ImGui::GetCursorPos() + CalculatePanelSize());
	ViewTopLeftPixel = ImGui::GetCursorScreenPos();
	ImRect Rect(Window->DC.CursorPos, Window->DC.CursorPos + ViewSize);

	// callback for using our own image shader 
	ImGui::GetWindowDrawList()->AddCallback(DrawCallback, Image->GetShaderResourceView());
	ImGui::GetWindowDrawList()->AddImage(Image->GetShaderResourceView(), Rect.Min, Rect.Max, UV.Min, UV.Max);

	if (bMarqueeVisible)
	{
		DrawMarquee(Rect);
	}

	// reset callback for using our own image shader 
	ImGui::GetWindowDrawList()->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
}

void SSpriteEditor::DrawMarquee(const ImRect& Window)
{
	ImRect TmpMarqueeRect = MarqueeRect;

	// clamp
	const ImVec2 ImageSizeInv = ImVec2(1.0f, 1.0f) / Image->Size;
	const ImVec2 Floor = ImFloor(UV.Min / ImageSizeInv);
	TmpMarqueeRect.Min = (TmpMarqueeRect.Min - Floor) * Scale;
	TmpMarqueeRect.Max = (TmpMarqueeRect.Max - Floor) * Scale;

	TmpMarqueeRect.Min = ImMin(ImMax(TmpMarqueeRect.Min, ImVec2(0.0f, 0.0f)), Image->Size * Scale);
	TmpMarqueeRect.Max = ImMin(ImMax(TmpMarqueeRect.Max, ImVec2(0.0f, 0.0f)), Image->Size * Scale);

	const ImVec2 TopLeftSubTexel = ImagePosition * Scale * Image->Size - ViewSize * 0.5f;
	const ImVec2 TopLeftPixel = ViewTopLeftPixel - (TopLeftSubTexel - ImFloor(TopLeftSubTexel / Scale) * Scale);

	ImGui::GetWindowDrawList()->_FringeScale = 0.1f;
	ImGui::GetWindowDrawList()->AddCallback(DrawCallback, Shader::LINE_ID);
	ImGui::GetWindowDrawList()->AddRect(TopLeftPixel + TmpMarqueeRect.Min, TopLeftPixel + TmpMarqueeRect.Max, ImGui::GetColorU32(ImGuiCol_Button), 0, 0, 0.001f);
}

void SSpriteEditor::RenderPopupMenu()
{
	if (ImGui::BeginPopup(PopupMenuName))
	{
		bPopupMenu = true;
		if (bMouseInsideMarquee && ImGui::MenuItem("Add to"))
		{
			GetParent()->AddSpriteBlock(ImageFilePath, MarqueeRect);
		}

		ImGui::EndPopup();
	}
}

void SSpriteEditor::RoundImagePosition()
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

void SSpriteEditor::SetScale(float scaleY)
{
	SetScale(ImVec2(scaleY * PixelAspectRatio, scaleY));
}

void SSpriteEditor::SetScale(ImVec2 NewScale)
{
	NewScale = ImClamp(NewScale, ScaleMin, ScaleMax);
	ViewSizeUV *= Scale / NewScale;
	Scale = NewScale;

	// only force nearest sampling if zoomed in
	bForceNearestSampling = (Scale.x > 1.0f || Scale.y > 1.0f);
	GridWidth = ImVec2(1.0f / Scale.x, 1.0f / Scale.y);
}

void SSpriteEditor::SetImagePosition(ImVec2 NewPosition)
{
	ImagePosition = NewPosition;
	RoundImagePosition();
}

ImVec2 SSpriteEditor::CalculatePanelSize()
{
	// calculate panel size
	const float BorderWidth = 1.0f;
	ImVec2 ContentRegionAvail = ImGui::GetContentRegionAvail();
	ImVec2 AvailablePanelSize = ContentRegionAvail - ImVec2(BorderWidth, BorderWidth) * 2.0f;

	RoundImagePosition();

	TextureSizePixels = Image->Size * Scale;
	ViewSizeUV = AvailablePanelSize / TextureSizePixels;
	UV.Min = ImagePosition - ViewSizeUV * 0.5f;
	UV.Max = ImagePosition + ViewSizeUV * 0.5f;

	ImVec2 DrawImageOffset(BorderWidth, BorderWidth);
	ViewSize = AvailablePanelSize;

	// don't crop the texture to UV [0,1] range.  What you see outside this range will depend on API and texture properties
	if (TextureSizePixels.x < AvailablePanelSize.x)
	{
		// not big enough to horizontally fill view
		ViewSize.x = ImFloor(TextureSizePixels.x);
		DrawImageOffset.x += ImFloor((AvailablePanelSize.x - TextureSizePixels.x) * 0.5f);
		UV.Min.x = 0.0f;
		UV.Max.x = 1.0f;
		ViewSizeUV.x = 1.0f;
		ImagePosition.x = 0.5f;
	}
	if (TextureSizePixels.y < AvailablePanelSize.y)
	{
		// not big enough to vertically fill view
		ViewSize.y = ImFloor(TextureSizePixels.y);
		DrawImageOffset.y += ImFloor((AvailablePanelSize.y - TextureSizePixels.y) * 0.5f);
		UV.Min.y = 0.0f;
		UV.Max.y = 1.0f;
		ViewSizeUV.y = 1.0f;
		ImagePosition.y = 0.5f;
	}

	return DrawImageOffset;
}

ImVec2 SSpriteEditor::ConverPositionToPixel(const ImVec2& Position)
{
	const ImVec2 ImageSizeInv = ImVec2(1.0f, 1.0f) / Image->Size;
	return ImFloor((Position - ViewTopLeftPixel + UV.Min / ImageSizeInv * Scale) / Scale);
}

Transform2D SSpriteEditor::GetTexelsToPixels(const ImVec2& ScreenTopLeft, const ImVec2& ScreenViewSize, const ImVec2& UVTopLeft, const ImVec2& UVViewSize, const ImVec2& TextureSize)
{
	const ImVec2 UVToPixel = ScreenViewSize / UVViewSize;
	Transform2D Transform;
	Transform.Scale = UVToPixel / TextureSize;
	Transform.Translate.x = ScreenTopLeft.x - UVTopLeft.x * UVToPixel.x;
	Transform.Translate.y = ScreenTopLeft.y - UVTopLeft.y * UVToPixel.y;
	return Transform;
}

void SSpriteEditor::HandleKeyboardInputs()
{
	ImGuiIO& IO = ImGui::GetIO();
	const bool bHovered = ImGui::IsWindowHovered();

	const bool Shift = IO.KeyShift;
	const bool Ctrl = IO.ConfigMacOSXBehaviors ? IO.KeySuper : IO.KeyCtrl;
	const bool Alt = IO.ConfigMacOSXBehaviors ? IO.KeyCtrl : IO.KeyAlt;

	if (!ImGui::IsWindowFocused())
	{
		return;
	}
}

void SSpriteEditor::HandleMouseInputs()
{
	// change scale
	if (Scale != OldScale)
	{
		OldScale = Scale;
	}

	// scale
	TexelsToPixels = GetTexelsToPixels(ViewTopLeftPixel, ViewSize, UV.Min, ViewSizeUV, Image->Size);
	PixelsToTexels = TexelsToPixels.Inverse();

	const ImVec2 MousePosition = ImGui::GetMousePos();
	ImVec2 MousePositionTexel = PixelsToTexels * MousePosition;
	const ImVec2 MouseUV = MousePositionTexel / Image->Size;
	//MousePositionTexel.x = Math::Modulus(MousePositionTexel.x, Image->Size.x);
	//MousePositionTexel.y = Math::Modulus(MousePositionTexel.y, Image->Size.y);

	ImGuiIO& IO = ImGui::GetIO();
	const bool bHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);
	const bool Shift = IO.KeyShift;
	const bool Ctrl = IO.ConfigMacOSXBehaviors ? IO.KeySuper : IO.KeyCtrl;
	const bool Alt = IO.ConfigMacOSXBehaviors ? IO.KeyCtrl : IO.KeyAlt;

	std::shared_ptr<SViewer> Viewer = GetParent();
	if (bHovered && !Ctrl && !Shift && !Alt && IO.MouseWheel != 0.0f)
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
	else if (!bDragging && IO.MouseDown[ImGuiMouseButton_Middle])
	{
		if (Viewer)
		{
			bDragging = Viewer->TrySetTool(EToolType::Hand);
		}
	}
	else if (IO.MouseReleased[ImGuiMouseButton_Middle])
	{
		bDragging = false;

		if (Viewer)
		{
			Viewer->ResetTool();
		}
	}
	else if (bHovered && Image != nullptr && IO.MouseReleased[ImGuiMouseButton_Right])
	{
		FSprite* Sprite = GetParent()->GetSelectedSprite();
		if (Sprite != nullptr)
		{
			bMouseInsideMarquee = MarqueeRect.Contains(ConverPositionToPixel(ImGui::GetMousePos()));
			ImGui::OpenPopup(PopupMenuName);
		}
	}

	// dragging
	if (bDragging)
	{
		ImVec2 uvDelta = IO.MouseDelta * ViewSizeUV / ViewSize;
		ImagePosition -= uvDelta;
		RoundImagePosition();
	}
}

void SSpriteEditor::HandleMarqueeInput()
{
	std::shared_ptr<SViewer> Viewer = GetParent();
	if (Viewer == nullptr || !Viewer->IsMarqueeTool())
	{
		return;
	}

	const ImGuiIO& IO = ImGui::GetIO();
	const bool bHovered = ImGui::IsWindowHovered();

	const bool Shift = IO.KeyShift;
	const bool Ctrl = IO.ConfigMacOSXBehaviors ? IO.KeySuper : IO.KeyCtrl;
	const bool Alt = IO.ConfigMacOSXBehaviors ? IO.KeyCtrl : IO.KeyAlt;

	if (!bPopupMenu && bHovered && !Ctrl && !Shift && !Alt && !bMarqueeActive && ImGui::IsKeyDown(ImGuiKey_MouseLeft))
	{
		MarqueeRect.Min = ImMin(ImMax(ConverPositionToPixel(ImGui::GetMousePos()), ImVec2(0.0f, 0.0f)), Image->Size * Scale);
		MarqueeRect.Max = MarqueeRect.Min;

		bMarqueeVisible = true;
		bMarqueeActive = true;
	}
	else if (!bPopupMenu && bHovered && !Ctrl && !Shift && !Alt && bMarqueeActive && ImGui::IsKeyDown(ImGuiKey_MouseLeft))
	{
		MarqueeRect.Max = ImMin(ImMax(ConverPositionToPixel(ImGui::GetMousePos() + Scale), ImVec2(0.0f, 0.0f)), Image->Size * Scale);
	}
	else if (bHovered && ImGui::IsKeyReleased(ImGuiKey_MouseLeft))
	{
		bMarqueeActive = false;
	}
	// защита от сброса выделения при клике мимо всплывающего меню
	else if (bPopupMenu && bHovered && !Ctrl && !Shift && !Alt && bMarqueeVisible && ImGui::IsKeyPressed(ImGuiKey_MouseLeft))
	{
		bPopupMenu = false;
	}
}

void SSpriteEditor::OnSelectedFileImage(const std::filesystem::directory_entry& FilePath)
{
	if (Image != nullptr)
	{
		Image->Release();
		Image = nullptr;
	}

	ImageFilePath = FilePath;
	Image = Utils::LoadImage(FilePath.path().string());
	Scale = OldScale = { 1.0f, 1.0f };
}
