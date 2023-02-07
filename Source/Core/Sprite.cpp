#include "Sprite.h"
#include "Core\Utils.h"
#include "Core\AppFramework.h"
#include "Viewer\Windows\SpriteConstructor.h"

namespace
{
	void DrawCallback(const ImDrawList* ParentList, const ImDrawCmd* CMD)
	{
		std::shared_ptr<SViewer> Viewer = FAppFramework::Get().GetViewer();
		std::shared_ptr<SSpriteConstructor> SpriteConstructor = Viewer ? std::dynamic_pointer_cast<SSpriteConstructor>(Viewer->GetWindow(EWindowsType::SpriteConstructor)) : nullptr;
		if (SpriteConstructor)
		{
			SpriteConstructor->OnDrawCallback(ParentList, CMD);
		}
	}

	bool Button(const char* StringID, std::shared_ptr<FImage>& ImageEmpty, FSprite* Sprite, uint32_t FrameNum, const ImVec2& VisibleSize, const ImVec4& BackgroundColor, const ImVec4& TintColor, const ImVec4& SelectedColor)
	{
		ImGuiWindow* Window = ImGui::GetCurrentWindow();
		if (Window->SkipItems || Sprite == nullptr)
		{
			return false;
		}

		const ImGuiID ID = Window->GetID(StringID);
		const ImGuiStyle& Style = ImGui::GetStyle();
		const ImVec2 LabelSize = ImGui::CalcTextSize(StringID, NULL, true);

		const ImVec2 Padding = Style.FramePadding;
		const ImRect Rect(Window->DC.CursorPos, Window->DC.CursorPos + VisibleSize + Padding * 2.0f);
		ImGui::ItemSize(Rect);
		if (!ImGui::ItemAdd(Rect, ID))
		{
			return false;
		}

		bool bHovered, bHeld;
		const bool bPressed = ImGui::ButtonBehavior(Rect, ID, &bHovered, &bHeld);

		const float SpriteMin = ImMin(Sprite->Size.x, Sprite->Size.y);
		const float SpriteMax = ImMax(Sprite->Size.x, Sprite->Size.y);

		// render
		ImVec2 p0 = Rect.Min + Padding;
		ImVec2 p1 = Rect.Max - Padding;

		ImGui::RenderNavHighlight(Rect, ID);
		const ImU32 Color = ImGui::GetColorU32((bHeld && bHovered) ? ImGuiCol_ButtonActive : bHovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
		ImGui::RenderFrame(Rect.Min, Rect.Max, Color, true, ImClamp((float)ImMin(Padding.x, Padding.y), 0.0f, Style.FrameRounding));
		if (BackgroundColor.w > 0.0f)
		{
			Window->DrawList->AddRectFilled(p0, p1, ImGui::GetColorU32(BackgroundColor));
		}
		
		const ImVec2 Scale = VisibleSize / SpriteMax;
		ImVec2 NewPadding((SpriteMax - Sprite->Size.x) * 0.5f, (SpriteMax - Sprite->Size.y) * 0.5f);
		p0 += NewPadding * Scale;
		p1 -= NewPadding * Scale;

		ImGui::PushClipRect(p0, p1, true);

		// callback for using our own image shader 
		Window->DrawList->AddCallback(DrawCallback, (void*)ImageEmpty.get());
		Window->DrawList->AddImage(ImageEmpty->GetShaderResourceView(), p0, p0 + ImageEmpty->Size * Scale, ImVec2(0.0f, 0.0f), ImVec2(1.0, 1.0f), ImGui::GetColorU32(TintColor));

		for (FSpriteLayer& Layer : Sprite->Layers)
		{
			FSpriteFrame* SpriteFrame = Layer.GetSpritesBlocks(FrameNum);
			if (SpriteFrame == nullptr)
			{
				continue;
			}

			for (const FSpriteBlock& Block : SpriteFrame->Blocks)
			{
				if (!Block.IsValid())
				{
					continue;
				}

				const ImVec2 StartImage = p0 + Sprite->Pivot * Scale + Block.Offset * Scale;
				const ImVec2 EndImage = StartImage + Block.ImageSprite->Size * Scale;

				// callback for using our own image shader 
				Window->DrawList->AddCallback(DrawCallback, (void*)Block.ImageSprite.get());
				Window->DrawList->AddImage(Block.ImageSprite->GetShaderResourceView(), StartImage, EndImage, ImVec2(0.0f, 0.0f), ImVec2(1.0, 1.0f), ImGui::GetColorU32(TintColor));
			}
		}
		ImGui::PopClipRect();

		// reset callback for using our own image shader 
		Window->DrawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);

		return bPressed;
	};
}

FSpriteBlock::FSpriteBlock()
	: Offset(0.0f, 0.0f)
	, Marquee(0.0f, 0.0f, 0.0f, 0.0f)
	, Filename("")
	, ImageSprite(nullptr)
{}

void FSpriteBlock::Initialize()
{
	if (ImageSprite == nullptr)
	{
		ImageSprite = std::make_shared<FImage>();
	}

	if (!ImageSprite->IsValid())
	{
		ImVec2 Size;
		uint8_t* ResizeImageData = new uint8_t[uint32_t(Marquee.GetArea()) * 4];
		// load from disk into a raw RGBA buffer
		uint8_t* ImageData = Utils::LoadImageToMemory(Filename, Size);
		ImVec2 SizeInv = ImVec2(1.0f, 1.0f) / Size;

		Utils::ResizeRegion(ImageData, Size, Marquee.GetSize(), ResizeImageData, Marquee.Min * SizeInv, Marquee.Max * SizeInv);
		ImageSprite->CreateTexture(ResizeImageData, Marquee.GetSize());
		Utils::FreeImageToMemory(ImageData);
		delete[] ResizeImageData;
	}
}

void FSpriteBlock::Release()
{
	if (ImageSprite != nullptr)
	{
		ImageSprite->Release();
		ImageSprite = nullptr;
	}
}

FSpriteLayer::FSpriteLayer()
	: bVisible(false)
	, bLock(false)
	, bEmpty(true)
	, Name("")
{}

void FSpriteLayer::Release()
{
	for (FSpriteFrame& Frame : SpriteFrame)
	{
		for (FSpriteBlock& SpriteBlock : Frame.Blocks)
		{
			SpriteBlock.Release();
		}
	}
	SpriteFrame.clear();
}

FSpriteFrame& FSpriteLayer::AddFrame()
{
	return SpriteFrame.emplace_back();
}

FSpriteFrame* FSpriteLayer::GetSpritesBlocks(uint32_t FrameNum)
{
	return SpriteFrame.size() > FrameNum ? &SpriteFrame[FrameNum] : nullptr;
}

bool FSpriteLayer::AddSpriteBlock(FSpriteBlock& NewSpriteBlock, uint32_t FrameNum)
{
	if (!IsValidFrame(FrameNum))
	{
		return false;
	}

	SpriteFrame[FrameNum].Blocks.push_back(NewSpriteBlock);
	return true;
}

FSprite::FSprite()
	: NumFrame(0)
	, Size(-1.0f, -1.0f)
	, Pivot(0.0f, 0.0f)
	, ColorMode(EColorMode::Unknow)
	, Name("")
	, ImageSprite(nullptr)
	, bAnimation(false)
	, SelectedLayer(0)
	, SelectedFrame(0)
{}

void FSprite::Initialize()
{
	if (ImageSprite == nullptr)
	{
		ImageSprite = std::make_shared<FImage>();
	}

	if (!ImageSprite->IsValid())
	{
		//ImageSprite->CreateTexture();
	}
}

void FSprite::Release()
{
	if (ImageSprite != nullptr)
	{
		ImageSprite->Release();
		ImageSprite = nullptr;
	}

	for (FSpriteLayer& SpriteLayer : Layers)
	{
		SpriteLayer.Release();
	}
	Layers.clear();
}

void FSprite::AddFrame()
{
	for (FSpriteLayer& Layer : Layers)
	{
		Layer.AddFrame();
	}
}

FSpriteFrame* FSprite::GetFrame(uint32_t LayerNum, uint32_t FrameNum)
{
	return IsValidLayer(LayerNum) ? Layers[LayerNum].GetSpritesBlocks(FrameNum) : nullptr;
}

FSpriteLayer& FSprite::AddLayer()
{
	return Layers.emplace_back();
}

bool FSprite::Draw(const char* StringID, std::shared_ptr<FImage>& ImageEmpty, const ImVec2& VisibleSize, uint32_t FrameNum /*= 0*/)
{
	if (!IsValid())
	{
		return false;
	}
	const ImVec4 BackgroundColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	const ImVec4 TintColor = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);
	const ImVec4 SelectedColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

	return Button(StringID, ImageEmpty, this, FrameNum, VisibleSize, BackgroundColor, TintColor, SelectedColor);
}

std::string FSprite::ColotModeToString(EColorMode Mode)
{
	switch (Mode)
	{
	case EColorMode::ZX:		return "ZX";		break;
	case EColorMode::Indexed:	return "Indexed";	break;
	case EColorMode::RGB:		return "RGBA";		break;
	case EColorMode::Unknow:
	default:					return "";			break;
	}
}
