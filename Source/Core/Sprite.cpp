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

	bool Button(const char* StringID, FSprite* Sprite, uint32_t FrameNum, const ImVec2& VisibleSize, const ImVec4& BackgroundColor, const ImVec4& TintColor, const ImVec4& SelectedColor)
	{
		ImGuiWindow* Window = ImGui::GetCurrentWindow();
		if (Window->SkipItems)
		{
			return false;
		}

		const ImGuiID ID = Window->GetID(StringID);
		const ImGuiStyle& Style = ImGui::GetStyle();
		const ImVec2 LabelSize = ImGui::CalcTextSize(StringID, NULL, true);

		const ImVec2 Padding = Style.FramePadding;
		const ImRect bb(Window->DC.CursorPos, Window->DC.CursorPos + VisibleSize + Padding * 2.0f);
		ImGui::ItemSize(bb);
		if (!ImGui::ItemAdd(bb, ID))
		{
			return false;
		}

		bool bHovered, bHeld;
		const bool bPressed = ImGui::ButtonBehavior(bb, ID, &bHovered, &bHeld);

		// render
		const ImVec2 p0 = bb.Min + Padding;
		const ImVec2 p1 = bb.Max - Padding;

		ImGui::RenderNavHighlight(bb, ID);
		const ImU32 Color = ImGui::GetColorU32((bHeld && bHovered) ? ImGuiCol_ButtonActive : bHovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
		ImGui::RenderFrame(bb.Min, bb.Max, Color, true, ImClamp((float)ImMin(Padding.x, Padding.y), 0.0f, Style.FrameRounding));
		if (BackgroundColor.w > 0.0f)
		{
			Window->DrawList->AddRectFilled(p0, p1, ImGui::GetColorU32(BackgroundColor));
		}

		ImGui::PushClipRect(p0, p1, true);
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

				const ImVec2 Scale = VisibleSize / Sprite->Size;
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

void FSpriteLayer::AddSpriteBlock(FSpriteBlock& NewSpriteBlock, uint32_t FrameNum)
{
	if (!IsValidFrame(FrameNum))
	{
		return;
	}

	SpriteFrame[FrameNum].Blocks.push_back(NewSpriteBlock);
}

FSprite::FSprite()
	: NumFrame(0)
	, Size(-1.0f, -1.0f)
	, Pivot(0.0f, 0.0f)
	, ColorMode(EColorMode::Unknow)
	, Name("")
	, ImageSprite(nullptr)
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

void FSprite::Draw(const char* StringID, const ImVec2& VisibleSize, uint32_t FrameNum /*= 0*/)
{
	if (!IsValid())
	{
		return;
	}
	const ImVec4 BackgroundColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	const ImVec4 TintColor = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);
	const ImVec4 SelectedColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

	Button(StringID, this, FrameNum, VisibleSize, BackgroundColor, TintColor, SelectedColor);
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
