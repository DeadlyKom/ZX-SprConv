#include "Sequencer.h"

#include "Core\Sprite.h"
#include "Core\Utils.h"
#include "Core\Image.h"

#include "Viewer\Windows\Property.h"

namespace
{
	enum ItemColumnID
	{
		ItemColumnID_Visible,
		ItemColumnID_Lock,
		ItemColumnID_LayerName,
		ItemColumnID_Frame,
	};

	const ImVec4 BackgroundColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	const ImVec4 SelectedColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	const ImVec4 TintColor = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);

	const char* PopupBlockMenuName = "Popup Block Menu";
}

SSequencer::SSequencer()
	: bBaseVisible(true)
	, bBaseLock(false)
	, Selected(ESequencerControl::None)

	// drag and drop
	, DragStartIndex(INDEX_NONE)
	, DragEndIndex(INDEX_NONE)
{}

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

	ImGui::Begin("Sequencer", &bOpen, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

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
		ImGuiTableFlags_Resizable
		| ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY
		| ImGuiTableFlags_RowBg
		| ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_BordersOuterH
		;

	const float TextHeight = ImGui::GetTextLineHeightWithSpacing();
	const int32_t ColumCount = 3 + Sprite->NumFrame;
	if (ImGui::BeginTable("Sequencer", ColumCount, Flags))
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
			const std::string NumFrameText = Utils::Format("Frame##%i", FrameIndex + 1);
			ImGui::TableSetupColumn(NumFrameText.c_str(), ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoHide, 32.0f, ItemColumnID_Frame);
		}

		ImGui::TableSetupScrollFreeze(ColumCount, 1);
		ImGui::TableNextRow(ImGuiTableRowFlags_None, 0.0f);

		{
			ImGui::TableSetColumnIndex(0);
			std::shared_ptr<FImage> ImageVisible = bBaseVisible ? ImageVisibleEnable : ImageVisibleDisable;
			if (DrawButton("Sequencer##BaseVisible", *ImageVisible, BackgroundColor, TintColor, SelectedColor))
			{
				bBaseVisible = !bBaseVisible;
				for (uint32_t RowIndex = 0; RowIndex < Sprite->Layers.size(); ++RowIndex)
				{
					std::shared_ptr<FSpriteLayer>& SpriteLayer = Sprite->Layers[RowIndex];
					SpriteLayer->bVisible = bBaseVisible;
				}
			}

			ImGui::TableSetColumnIndex(1);
			std::shared_ptr<FImage> ImageLocking = bBaseLock ? ImageLock : ImageUnlock;
			if (DrawButton("Sequencer##BaseLock", *ImageLocking, BackgroundColor, TintColor, SelectedColor))
			{
				bBaseLock = !bBaseLock;
				for (uint32_t RowIndex = 0; RowIndex < Sprite->Layers.size(); ++RowIndex)
				{
					std::shared_ptr<FSpriteLayer>& SpriteLayer = Sprite->Layers[RowIndex];
					SpriteLayer->bLock = bBaseLock;
				}
			}

			ImGui::TableSetColumnIndex(2);
			for (uint32_t FrameIndex = 0; FrameIndex < Sprite->NumFrame; ++FrameIndex)
			{
				ImGui::TableNextColumn();

				const std::string NumFrameText = Utils::Format("%i", FrameIndex + 1);
				ImGui::TextUnformatted(NumFrameText.c_str());
			}
		}

		for (uint32_t RowIndex = 0; RowIndex < Sprite->Layers.size(); ++RowIndex)
		{
			ImGui::TableNextRow(ImGuiTableRowFlags_None, 0.0f);
			DrawLayer(Sprite, Sprite->Layers[RowIndex], RowIndex, Sprite->NumFrame);
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
			PropertiesSpriteBlock();
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
	if (CachedSelectedSprite.expired() || CachedSelectedSpriteBlock.expired())
	{
		return;
	}
	GetWindow<SProperty>(EWindowsType::Property)->SetProperty(EPropertyType::SpriteBlock, CachedSelectedSprite, CachedSelectedSpriteBlock);
}

void SSequencer::PropertiesSpriteBlock()
{
	if (CachedSelectedSprite.expired() || CachedSelectedSpriteBlock.expired())
	{
		return;
	}
	GetWindow<SProperty>(EWindowsType::Property)->SetProperty(EPropertyType::SpriteBlock, CachedSelectedSprite, CachedSelectedSpriteBlock);
}

void SSequencer::RemoveSpriteBlok()
{
	if (CachedSelectedSpriteLeyer.expired() || CachedSelectedSpriteBlock.expired())
	{
		return;
	}

	std::shared_ptr<FSpriteLayer> SelectedSpriteLeyer = CachedSelectedSpriteLeyer.lock();
	SelectedSpriteLeyer->RemoveSpriteBlock(CachedSelectedSpriteBlock.lock());
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

void SSequencer::DrawLayer(std::shared_ptr<FSprite> Sprite, std::shared_ptr<FSpriteLayer> SpriteLayer, uint32_t NumLayer, uint32_t NumFrame)
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
	DrawFrames(Sprite, SpriteLayer, NumLayer, NumFrame);
}

void SSequencer::DrawFrames(std::shared_ptr<FSprite> Sprite, std::shared_ptr<FSpriteLayer> SpriteLayer, uint32_t NumLayer, uint32_t NumFrame)
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
			const uint32_t BlocksNum = static_cast<uint32_t>(SpriteLayer->Blocks.size());
			for (uint32_t Index = 0; Index < BlocksNum; ++Index)
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(2);

				std::shared_ptr<FSpriteBlock>& Block = SpriteLayer->Blocks[Index];
				const std::string StringID = Utils::Format("%s##%i", Block->Name.c_str(), Index);
				if (ImGui::TreeNodeEx(StringID.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth))
				{
					// start drag
					if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
					{
						DragBlock = Block;
						DragStartIndex = Index;
					}
					// dragging
					else if (DragBlock && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
					{
						DragEndIndex = Index;
					}
					// end drag
					else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
					{
						DragBlock.reset();
						DragStartIndex = INDEX_NONE;
						DragEndIndex = INDEX_NONE;
					}
					// pop up menu
					else if(ImGui::IsItemClicked(ImGuiMouseButton_Right))
					{
						CachedSelectedSprite = Sprite;
						CachedSelectedSpriteLeyer = SpriteLayer;
						CachedSelectedSpriteBlock = Block;
						ImGui::OpenPopup(PopupBlockMenuName);
					}

					// draw frames
					for (uint32_t FrameIndex = 0; FrameIndex < NumFrame; ++FrameIndex)
					{
						ImGui::TableNextColumn();

						std::shared_ptr<FSpriteFrame> SpriteFrame = SpriteLayer->GetSpritesFrame(FrameIndex);
						if (!SpriteFrame)
						{
							continue;
						}

						std::shared_ptr<FImage> ImageFrame = SpriteFrame->bVisibleBlocks[Index] ? ImageFill : ImageEmpty;
						const std::string LayerFrameName = Utils::Format("Sequencer##Layer%iFrame%i", NumLayer, FrameIndex);
						DrawButton(LayerFrameName.c_str(), *ImageFrame, BackgroundColor, TintColor, SelectedColor);
					}
				}
			}

			// drag and drop operation
			const bool bDragging = ImGui::GetMouseDragDelta().y != 0.0f;
			const bool bValidDragDelta = DragStartIndex >= 0 && DragStartIndex < BlocksNum && DragEndIndex >= 0 && DragEndIndex < BlocksNum && DragStartIndex - DragEndIndex != 0;
			if (bDragging && bValidDragDelta)
			{
				if (SpriteLayer->Blocks[DragEndIndex] != DragBlock)
				{
					// swap
					SpriteLayer->SwapSpriteBlocks(DragStartIndex, DragEndIndex);
					DragStartIndex = DragEndIndex;
				}
			}

			RenderPopupBlockMenu();
			ImGui::TreePop();
		}
	}
}
