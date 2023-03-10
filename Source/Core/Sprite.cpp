#include "Sprite.h"
#include "Core\Utils.h"
#include "Core\AppFramework.h"
#include "Viewer\Windows\SpriteConstructor.h"

namespace
{
	int32_t BlockCounter = 0;
	void DrawCallback(const ImDrawList* ParentList, const ImDrawCmd* CMD)
	{
		std::shared_ptr<SViewer> Viewer = FAppFramework::Get().GetViewer();
		std::shared_ptr<SSpriteConstructor> SpriteConstructor = Viewer ? std::dynamic_pointer_cast<SSpriteConstructor>(Viewer->GetWindow(EWindowsType::SpriteConstructor)) : nullptr;
		if (SpriteConstructor)
		{
			SpriteConstructor->OnDrawCallback(ParentList, CMD);
		}
	}

	bool Button(const char* StringID, std::shared_ptr<FImage>& ImageEmpty, std::shared_ptr<FSprite> Sprite, uint32_t FrameNum, const ImVec2& VisibleSize, const ImVec4& BackgroundColor, const ImVec4& TintColor, const ImVec4& SelectedColor)
	{
		ImGuiWindow* Window = ImGui::GetCurrentWindow();
		if (Window->SkipItems || !Sprite)
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
		Window->DrawList->AddImage(ImageEmpty->GetShaderResourceView(), p0, p0 + ImageEmpty->Size * Scale, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), ImGui::GetColorU32(TintColor));

		for (std::shared_ptr<FSpriteLayer>& Layer : Sprite->Layers)
		{
			if (!Layer->bVisible)
			{
				continue;
			}

			const std::shared_ptr<FSpriteFrame>& SpriteFrame = Layer->GetSpritesFrame(FrameNum);
			if (!SpriteFrame)
			{
				continue;
			}

			for (uint32_t Index = 0; Index < SpriteFrame->bVisibleBlocks.size(); ++Index)
			{
				if (!!SpriteFrame->bVisibleBlocks[Index] == false)
				{
					continue;
				}

				const std::shared_ptr<FSpriteBlock>& SpriteBlock = Layer->Blocks[Index];
				const ImVec2 StartImage = p0 + Sprite->Pivot * Scale + SpriteBlock->Offset * Scale;
				const ImVec2 EndImage = StartImage + SpriteBlock->ImageSprite->Size * Scale;

				// callback for using our own image shader 
				Window->DrawList->AddCallback(DrawCallback, (void*)SpriteBlock->ImageSprite.get());
				Window->DrawList->AddImage(SpriteBlock->ImageSprite->GetShaderResourceView(), StartImage, EndImage, ImVec2(0.0f, 0.0f), ImVec2(1.0, 1.0f), ImGui::GetColorU32(TintColor));
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
	, bVisible(false)
	, bLock(false)
	, Filename("")
	, ImageSprite(nullptr)
{
	Name = Utils::Format("Bit %i", ++BlockCounter);
}

void FSpriteBlock::Initialize()
{
	if (!ImageSprite)
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
	if (ImageSprite)
	{
		ImageSprite->Release();
		ImageSprite.reset();
	}
}

FSpriteLayer::FSpriteLayer()
	: bVisible(false)
	, bLock(false)
	, Name("")
{}

void FSpriteLayer::Release()
{
	for (std::shared_ptr<FSpriteBlock>& Block : Blocks)
	{
		Block->Release();
	}
	Blocks.clear();
	Frames.clear();
}

std::shared_ptr<FSpriteFrame> FSpriteLayer::GetSpritesFrame(uint32_t FrameNum)
{
	return IsValidFrame(FrameNum) ? Frames[FrameNum] : nullptr;
}

std::shared_ptr<FSpriteFrame> FSpriteLayer::AddFrame()
{
	Frames.push_back(std::make_shared<FSpriteFrame>());

	std::shared_ptr<FSpriteFrame> Frame = Frames.back();
	for (uint32_t Block = 0; Block < Blocks.size(); ++Block)
	{
		Frame->bVisibleBlocks.emplace_back(true);
	}
	
	return Frame;
}

void FSpriteLayer::AddSpriteBlock(std::shared_ptr<FSpriteBlock> NewSpriteBlock)
{
	Blocks.push_back(NewSpriteBlock);

	for (std::shared_ptr<FSpriteFrame> Frame : Frames)
	{
		Frame->bVisibleBlocks.emplace_back(true);
	}
}

void FSpriteLayer::RemoveSpriteBlock(std::shared_ptr<FSpriteBlock> RemoveSpriteBlock)
{
	size_t RemoveIndex = 0;

	// remove sprite block
	for (; RemoveIndex < Blocks.size(); ++RemoveIndex)
	{
		std::shared_ptr<FSpriteBlock>& Block = Blocks[RemoveIndex];
		if (Block == RemoveSpriteBlock)
		{
			Block->Release();
			Blocks.erase(Blocks.begin() + RemoveIndex);
			break;
		}
	}

	if (RemoveIndex > Blocks.size())
	{
		LOG_ERROR("%s : error remove sprite block", __FUNCTION__);
		return;
	}

	// remove sprite block in all frames
	for (std::shared_ptr<FSpriteFrame> Frame : Frames)
	{
		Frame->bVisibleBlocks.erase(Frame->bVisibleBlocks.begin() + RemoveIndex);
	}
}

void FSpriteLayer::SwapSpriteBlocks(uint32_t IndexA, uint32_t IndexB)
{
	// swap all frames
	for (std::shared_ptr<FSpriteFrame>& Frame : Frames)
	{
		std::swap(Frame->bVisibleBlocks[IndexA], Frame->bVisibleBlocks[IndexB]);
	}

	// swap blocks
	std::swap(Blocks[IndexA], Blocks[IndexB]);
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
	if (!ImageSprite)
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

	for (std::shared_ptr<FSpriteLayer>& SpriteLayer : Layers)
	{
		SpriteLayer->Release();
	}
	Layers.clear();
}

void FSprite::AddFrame()
{
	for (std::shared_ptr<FSpriteLayer>& Layer : Layers)
	{
		Layer->AddFrame();
	}

	NumFrame++;
}

std::shared_ptr<FSpriteFrame> FSprite::GetFrame(uint32_t LayerNum, uint32_t FrameNum)
{
	return IsValidLayer(LayerNum) ? Layers[LayerNum]->GetSpritesFrame(FrameNum) : nullptr;
}

std::shared_ptr<FSpriteLayer> FSprite::AddLayer()
{
	Layers.push_back(std::make_shared<FSpriteLayer>());
	return Layers.back();
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

	return Button(StringID, ImageEmpty, shared_from_this(), FrameNum, VisibleSize, BackgroundColor, TintColor, SelectedColor);
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
