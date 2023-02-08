#include "Sequencer.h"
#include "Core\Sprite.h"
#include "Core\Utils.h"
#include "Core\Image.h"

namespace
{
	enum MyItemColumnID
	{
		ItemColumnID_Visible,
		ItemColumnID_Lock,
		ItemColumnID_LayerName,
	};

	const ImVec4 BackgroundColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	const ImVec4 SelectedColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	const ImVec4 TintColor = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);

	const char* PopupBlockMenuName = "Popup Block Menu";
}

void SSequencer::Initialize()
{
	bIncludeInWindows = true;
	Name = "Sequencer";

	ImageFirstButton = Utils::LoadImageFromResource(IDB_SEQUENCER_FIRST_BUTTON, TEXT("PNG"));
	ImagePreviousButton = Utils::LoadImageFromResource(IDB_SEQUENCER_PREVIOUS_BUTTON, TEXT("PNG"));
	ImagePlayButton = Utils::LoadImageFromResource(IDB_SEQUENCER_PLAY_BUTTON, TEXT("PNG"));
	ImageNextButton = Utils::LoadImageFromResource(IDB_SEQUENCER_NEXT_BUTTON, TEXT("PNG"));
	ImageLastButton = Utils::LoadImageFromResource(IDB_SEQUENCER_LAST_BUTTON, TEXT("PNG"));

	ImageVisibleEnable = Utils::LoadImageFromResource(IDB_SEQUENCER_VISIBLE_ENABLE, TEXT("PNG"));
	ImageVisibleDisable = Utils::LoadImageFromResource(IDB_SEQUENCER_VISIBLE_DISABLE, TEXT("PNG"));

	ImageLock = Utils::LoadImageFromResource(IDB_SEQUENCER_LOCK, TEXT("PNG"));
	ImageUnlock = Utils::LoadImageFromResource(IDB_SEQUENCER_UNLOCK, TEXT("PNG"));
	
	ImageEmpty = Utils::LoadImageFromResource(IDB_LAYER_EMPTY, TEXT("PNG"));
	ImageFill = Utils::LoadImageFromResource(IDB_LAYER_FILL, TEXT("PNG"));
}

void SSequencer::Render()
{
	if (!IsOpen())
	{
		Close();
		return;
	}

	ImGui::Begin("Sequencer", &bOpen);

	RenderControlButtons();
	const float TextHeight = ImGui::GetTextLineHeightWithSpacing();
	ImGui::Dummy(ImVec2(0.0f, TextHeight * 0.5f));
	RenderSequencer();

	ImGui::End();
}

void SSequencer::RenderControlButtons()
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1.0f, 1.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1.0f, 1.0f));

	const float DefaultWidth = ImGui::GetContentRegionAvail().x;
	auto ButtonLambda = [DefaultWidth](const char* ID, std::shared_ptr<FImage> Image, bool bSelectedCondition, float& AvailWidth, bool bLast = false) -> bool
	{
		if (!Image->IsValid())
		{
			return false;
		}

		const ImVec2 Padding = ImGui::GetStyle().FramePadding;
		const ImVec4 BackgroundColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
		const ImVec4 TintColor = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);
		const ImVec4 SelectedColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		const float Width = Image->Size.x + Padding.x * 2.0f;

		const bool bResult = ImGui::ImageButton(ID, Image->GetShaderResourceView(), Image->Size, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), BackgroundColor, bSelectedCondition ? SelectedColor : TintColor);
		AvailWidth -= Width;
		if (!bLast && AvailWidth > Width)
		{
			ImGui::SameLine();
		}
		else if (AvailWidth < Width)
		{
			AvailWidth = DefaultWidth;
		}

		return bResult;
	};

	float AvailWidth = DefaultWidth;
	if (ButtonLambda("Sequencer##First", ImageFirstButton, Selected == ESequencerControl::First, AvailWidth))
	{
		Selected = ESequencerControl::First;
	}
	if (ButtonLambda("Sequencer##Previous", ImagePreviousButton, Selected == ESequencerControl::Previous, AvailWidth))
	{
		Selected = ESequencerControl::Previous;
	}
	if (ButtonLambda("Sequencer##Play", ImagePlayButton, Selected == ESequencerControl::Play, AvailWidth))
	{
		Selected = ESequencerControl::Play;
	}
	if (ButtonLambda("Sequencer##Next", ImageNextButton, Selected == ESequencerControl::Next, AvailWidth))
	{
		Selected = ESequencerControl::Next;
	}
	if (ButtonLambda("Sequencer##Last", ImageLastButton, Selected == ESequencerControl::Last, AvailWidth, true))
	{
		Selected = ESequencerControl::Last;
	}
	ImGui::PopStyleVar(2);
}

void SSequencer::RenderSequencer()
{
	std::shared_ptr<FSprite> Sprite = GetParent()->GetSelectedSprite();
	if (!Sprite || !Sprite->IsValid())
	{
		return;
	}

	const ImGuiTableFlags Flags =
		ImGuiTableFlags_Resizable /*| ImGuiTableFlags_Reorderable *//*| ImGuiTableFlags_Hideable*/
		//| ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersH | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_BordersInnerH
		//| ImGuiTableFlags_Borders
		| ImGuiTableFlags_RowBg
		| ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_BordersOuterH
		/*| ImGuiTableFlags_SizingFixedFit*/;

	const float TextHeight = ImGui::GetTextLineHeightWithSpacing();
	const int FreezeColums = 3;
	const int FreezeRows = 2;
	const float InnerWidthWithScroll = 0.0f;
	const bool OuterSizeEnabled = false;
	const float RowMinHeight = 0.0f;

	const ImVec2 OuterSizeValue = ImVec2(38.0f * (Sprite->NumFrame + 3) + 100.0f, TextHeight * (Sprite->Layers.size() + 1) * 1.7f);

	const float inner_width_to_use = (Flags & ImGuiTableFlags_ScrollX) ? InnerWidthWithScroll : 0.0f;
	if (ImGui::BeginTable("Sequencer", 3 + Sprite->NumFrame, Flags, OuterSizeEnabled ? OuterSizeValue : ImVec2(0, 0), inner_width_to_use))
	{
		//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 2.0f));
		//ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.0f, 2.0f));
		ImGui::GetCurrentWindow()->Flags |= ImGuiWindowFlags_NoBackground;

		ImGui::TableSetupColumn("Visible", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoHide, 32.0f, ItemColumnID_Visible);
		ImGui::TableSetupColumn("Lock", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoHide, 32.0f, ItemColumnID_Lock);
		ImGui::TableSetupColumn("Layer", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed /*| ImGuiTableColumnFlags_NoResize*/ | ImGuiTableColumnFlags_NoHide, 100.0f, ItemColumnID_LayerName);

		for (uint32_t FrameIndex = 0; FrameIndex < Sprite->NumFrame; ++FrameIndex)
		{
			ImGui::TableSetupColumn("Frame##%i", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoHide, 32.0f, ItemColumnID_Visible);
		}

		ImGui::TableSetupScrollFreeze(FreezeColums, FreezeRows);
		//ImGui::TableHeadersRow();

		ImGui::TableNextRow(ImGuiTableRowFlags_None, RowMinHeight);

		{
			ImGui::TableSetColumnIndex(0);
			std::shared_ptr<FImage> ImageVisible = bVisible ? ImageVisibleEnable : ImageVisibleDisable;
			if (DrawButton("Sequencer##BaseVisible", *ImageVisible, BackgroundColor, TintColor, SelectedColor))
			{
				bVisible = !bVisible;
				for (uint32_t RowIndex = 0; RowIndex < Sprite->Layers.size(); ++RowIndex)
				{
					std::shared_ptr<FSpriteLayer>& SpriteLayer = Sprite->Layers[RowIndex];
					SpriteLayer->bVisible = bVisible;
				}
			}

			ImGui::TableSetColumnIndex(1);
			std::shared_ptr<FImage> ImageLocking = bLock ? ImageLock : ImageUnlock;
			if (DrawButton("Sequencer##BaseLock", *ImageLocking, BackgroundColor, TintColor, SelectedColor))
			{
				bLock = !bLock;
				for (uint32_t RowIndex = 0; RowIndex < Sprite->Layers.size(); ++RowIndex)
				{
					std::shared_ptr<FSpriteLayer>& SpriteLayer = Sprite->Layers[RowIndex];
					SpriteLayer->bLock = bLock;
				}
			}

			ImGui::TableNextColumn();
			for (uint32_t FrameIndex = 0; FrameIndex < Sprite->NumFrame; ++FrameIndex)
			{
				ImGui::TableNextColumn();
				std::string NumFrameText = Utils::Format("%i", FrameIndex + 1);
				ImGui::TextUnformatted(NumFrameText.c_str());
			}
		}

		for (uint32_t RowIndex = 0; RowIndex < Sprite->Layers.size(); ++RowIndex)
		{
			ImGui::TableNextRow(ImGuiTableRowFlags_None, RowMinHeight);
			DrawLayer(Sprite->Layers[RowIndex], RowIndex, Sprite->NumFrame);
		}

		ImGui::PopStyleVar();
		ImGui::EndTable();
	}
}

void SSequencer::RenderPopupBlockMenu()
{
	if (ImGui::BeginPopup(PopupBlockMenuName))
	{
		if (ImGui::MenuItem("Properties..."))
		{
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Delete"))
		{
			RemoveSpriteBlok();
		}

		ImGui::EndPopup();
	}
}

void SSequencer::ClickedSpriteBlok(std::shared_ptr<FSpriteBlock> Block)
{

}

void SSequencer::RemoveSpriteBlok()
{
	if (CachedSelectedSpriteLeyer.expired() || CachedSelectedSpriteBlock.expired())
	{
		return;
	}

	std::shared_ptr<FSpriteLayer> SelectedSpriteLeyer = CachedSelectedSpriteLeyer.lock();
	for (size_t Index = 0; Index < SelectedSpriteLeyer->Blocks.size(); ++Index)
	{
		std::shared_ptr<FSpriteBlock> Block = SelectedSpriteLeyer->Blocks[Index];
		if (Block == CachedSelectedSpriteBlock.lock())
		{
			Block->Release();
			CachedSelectedSpriteBlock.reset();
			SelectedSpriteLeyer->Blocks.erase(SelectedSpriteLeyer->Blocks.begin() + Index);
			return;
		}
	}
}

bool SSequencer::DrawButton(const char* StringID, const FImage& Image, const ImVec4& BackgroundColor, const ImVec4& TintColor, const ImVec4& SelectedColor)
{
	ImGuiWindow* Window = ImGui::GetCurrentWindow();
	if (Window->SkipItems)
	{
		return false;
	}

	ImVec2 ImageSize = Image.Size * 0.6f;

	const ImGuiID ID = Window->GetID(StringID);
	ImGuiStyle& Style = ImGui::GetStyle();
	const ImVec2 Padding = Style.FramePadding;

	// align
	ImVec2 Align(0.0f, 0.0f);

	Align.x = (Window->WorkRect.GetSize().x - ImageSize.x) * 0.5f;
	Align.y = (ImGui::TableGetHeaderRowHeight() - ImageSize.y) * 0.5f;

	const ImRect bb(Window->DC.CursorPos + Align, Window->DC.CursorPos + ImageSize + Padding * 2.0f + Align);
	ImGui::ItemSize(bb);
	if (!ImGui::ItemAdd(bb, ID))
	{
		return false;
	}

	bool bHovered, bHeld;
	bool bPressed = ImGui::ButtonBehavior(bb, ID, &bHovered, &bHeld);

	// Render
	ImGui::RenderNavHighlight(bb, ID);
	if (!(Window->Flags & ImGuiWindowFlags_NoBackground))
	{
		const ImU32 Color = ImGui::GetColorU32((bHeld && bHovered) ? ImGuiCol_ButtonActive : bHovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
		ImGui::RenderFrame(bb.Min, bb.Max, Color, true, ImClamp((float)ImMin(Padding.x, Padding.y), 0.0f, Style.FrameRounding));
	}
	if (BackgroundColor.w > 0.0f)
	{
		Window->DrawList->AddRectFilled(bb.Min + Padding, bb.Max - Padding, ImGui::GetColorU32(BackgroundColor));
	}
	Window->DrawList->AddImage(Image.GetShaderResourceView(), bb.Min + Padding , bb.Max - Padding, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), !bHovered ? ImGui::GetColorU32(TintColor) : ImGui::GetColorU32(SelectedColor));

	return bPressed;
}

void SSequencer::DrawLayer(std::shared_ptr<FSpriteLayer> SpriteLayer, uint32_t NumLayer, uint32_t NumFrame)
{
	if (!SpriteLayer)
	{
		return;
	}

	ImGui::TableSetColumnIndex(0);
	const std::string LayerVisibleName = Utils::Format("Sequencer##Visible%i", NumLayer);
	std::shared_ptr<FImage> ImageVisible = SpriteLayer->bVisible ? ImageVisibleEnable : ImageVisibleDisable;
	if (DrawButton(LayerVisibleName.c_str(), *ImageVisible, BackgroundColor, TintColor, SelectedColor))
	{
		SpriteLayer->bVisible = !SpriteLayer->bVisible;
	}

	ImGui::TableSetColumnIndex(1);
	const std::string LayerLockName = Utils::Format("Sequencer##Lock%i", NumLayer);
	std::shared_ptr<FImage> ImageLocking = SpriteLayer->bLock ? ImageLock : ImageUnlock;
	if (DrawButton(LayerLockName.c_str(), *ImageLocking, BackgroundColor, TintColor, SelectedColor))
	{
		SpriteLayer->bLock = !SpriteLayer->bLock;
	}

	ImGui::TableSetColumnIndex(2);
	ImGui::AlignTextToFramePadding();
	DrawFrames(SpriteLayer, NumLayer, NumFrame);
}

void SSequencer::DrawFrames(std::shared_ptr<FSpriteLayer> SpriteLayer, uint32_t NumLayer, uint32_t NumFrame)
{
	if (!SpriteLayer)
	{
		return;
	}

	if (SpriteLayer->Blocks.empty())
	{
		ImGui::TextUnformatted(SpriteLayer->Name.c_str());
	}
	else
	{
		const bool bVisible = ImGui::TreeNodeEx(SpriteLayer->Name.c_str(), ImGuiTreeNodeFlags_SpanFullWidth);
		if (bVisible)
		{
			for (std::shared_ptr<FSpriteBlock> Block : SpriteLayer->Blocks)
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(2);

				if (ImGui::TreeNodeEx(Block->Name.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth))
				{
					if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
					{
						ClickedSpriteBlok(Block);
					}
					else if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
					{
						CachedSelectedSpriteLeyer = SpriteLayer;
						CachedSelectedSpriteBlock = Block;
						ImGui::OpenPopup(PopupBlockMenuName);
					}

					for (uint32_t FrameIndex = 0; FrameIndex < NumFrame; ++FrameIndex)
					{
						ImGui::TableNextColumn();

						std::shared_ptr<FSpriteFrame> Frame = SpriteLayer->GetSpritesFrame(FrameIndex);
						if (!Frame)
						{
							continue;
						}

						// поиск совпадения
						bool bEmpty = true;
						for (std::weak_ptr<FSpriteBlock>& FrameBlock : Frame->Blocks)
						{
							if (FrameBlock.expired() || FrameBlock.lock() != Block)
							{
								continue;
							}

							bEmpty = false;
							break;
						}

						std::shared_ptr<FImage> ImageFrame = bEmpty ? ImageEmpty : ImageFill;
						const std::string LayerFrameName = Utils::Format("Sequencer##Layer%iFrame%i", NumLayer, FrameIndex);
						DrawButton(LayerFrameName.c_str(), *ImageFrame, BackgroundColor, TintColor, SelectedColor);
					}
				}
			}
			RenderPopupBlockMenu();
			ImGui::TreePop();
		}
	}
}
