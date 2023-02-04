#include "Sequencer.h"
#include "Core\Sprite.h"
#include "Core\Utils.h"
#include "Core\Image.h"

void SSequencer::Initialize()
{
	bIncludeInWindows = true;
	Name = "Sequencer";

	ImageVisibleEnable = Utils::LoadImageFromResource(IDB_SEQUENCER_VISIBLE_ENABLE, TEXT("PNG"));
	ImageVisibleDisable = Utils::LoadImageFromResource(IDB_SEQUENCER_VISIBLE_DISABLE, TEXT("PNG"));

	ImageLock = Utils::LoadImageFromResource(IDB_SEQUENCER_LOCK, TEXT("PNG"));
	ImageUnlock = Utils::LoadImageFromResource(IDB_SEQUENCER_UNLOCK, TEXT("PNG"));
	
	ImageEmpty = Utils::LoadImageFromResource(IDB_LAYER_EMPTY, TEXT("PNG"));
	ImageFill = Utils::LoadImageFromResource(IDB_LAYER_FILL, TEXT("PNG"));
}

enum MyItemColumnID
{
	ItemColumnID_Visible,
	ItemColumnID_Lock,
	ItemColumnID_LayerName,
};

void SSequencer::Render()
{
	if (!IsOpen())
	{
		Close();
		return;
	}

	ImGui::Begin("Sequencer", &bOpen);

	RenderControlButtons();
	RenderSequencer();

	ImGui::End();
}

void SSequencer::RenderControlButtons()
{

}

void SSequencer::RenderSequencer()
{
	FSprite* Sprite = GetParent()->GetSelectedSprite();
	if (Sprite == nullptr || !Sprite->IsValid())
	{
		return;
	}

	const ImGuiTableFlags Flags =
		ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable
		| ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersH | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_BordersInnerH
		| ImGuiTableFlags_Borders
		| ImGuiTableFlags_SizingFixedFit;

	const float TextHeight = ImGui::GetTextLineHeightWithSpacing();
	const int FreezeColums = 3;
	const int FreezeRows = 2;
	const float InnerWidthWithScroll = 0.0f;
	const bool OuterSizeEnabled = true;
	const float RowMinHeight = 0.0f;

	const ImVec2 OuterSizeValue = ImVec2(38.0f * (Sprite->NumFrame + 3) + 100.0f, TextHeight * (Sprite->Layers.size() + 1) * 1.7f);

	auto ButtonExLambda = [](const char* StringID, const FImage& Image, const ImVec4& BackgroundColor, const ImVec4& TintColor, const ImVec4& SelectedColor) -> bool
	{
		ImGuiWindow* Window = ImGui::GetCurrentWindow();
		if (Window->SkipItems)
		{
			return false;
		}

		const ImGuiID ID = Window->GetID(StringID);
		ImGuiStyle& Style = ImGui::GetStyle();
		const ImVec2 Padding = Style.FramePadding;
		const ImRect bb(Window->DC.CursorPos, Window->DC.CursorPos + Image.Size + Padding * 2.0f);
		ImGui::ItemSize(bb);
		if (!ImGui::ItemAdd(bb, ID))
		{
			return false;
		}

		bool bHovered, bHeld;
		bool bPressed = ImGui::ButtonBehavior(bb, ID, &bHovered, &bHeld);

		// align
		ImVec2 Align(0.0f, 0.0f);
		Align.x = (Window->WorkRect.GetSize().x - Image.Size.x) * 0.5f;
		ImVec2 Rect(Window->DC.CursorMaxPos - Window->DC.CursorPosPrevLine);
		Align.y = (Rect.y - Image.Size.y) * 0.5f;

		// Render
		ImGui::RenderNavHighlight(bb, ID);
		if (!(Window->Flags & ImGuiWindowFlags_NoBackground))
		{
			const ImU32 Color = ImGui::GetColorU32((bHeld && bHovered) ? ImGuiCol_ButtonActive : bHovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
			ImGui::RenderFrame(bb.Min + Align, bb.Max + Align, Color, true, ImClamp((float)ImMin(Padding.x, Padding.y), 0.0f, Style.FrameRounding));
		}
		if (BackgroundColor.w > 0.0f)
		{
			Window->DrawList->AddRectFilled(bb.Min + Padding + Align, bb.Max - Padding + Align, ImGui::GetColorU32(BackgroundColor));
		}
		Window->DrawList->AddImage(Image.GetShaderResourceView(), bb.Min + Padding + Align, bb.Max - Padding + Align, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), !bHovered ? ImGui::GetColorU32(TintColor) : ImGui::GetColorU32(SelectedColor));

		return bPressed;
	};

	const float inner_width_to_use = (Flags & ImGuiTableFlags_ScrollX) ? InnerWidthWithScroll : 0.0f;
	if (ImGui::BeginTable("Sequencer", 3 + Sprite->NumFrame, Flags, OuterSizeEnabled ? OuterSizeValue : ImVec2(0, 0), inner_width_to_use))
	{
		//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
		//ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.0f, 0.0f));
		ImGui::GetCurrentWindow()->Flags |= ImGuiWindowFlags_NoBackground;

		ImGui::TableSetupColumn("Visible", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoHide, 32.0f, ItemColumnID_Visible);
		ImGui::TableSetupColumn("Lock", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoHide, 32.0f, ItemColumnID_Lock);
		ImGui::TableSetupColumn("Layer", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoHide, 100.0f, ItemColumnID_LayerName);

		for (uint32_t FrameIndex = 0; FrameIndex < Sprite->NumFrame; ++FrameIndex)
		{
			ImGui::TableSetupColumn("Frame##%i", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoHide, 32.0f, ItemColumnID_Visible);
		}

		ImGui::TableSetupScrollFreeze(FreezeColums, FreezeRows);
		//ImGui::TableHeadersRow();

		ImGui::TableNextRow(ImGuiTableRowFlags_None, RowMinHeight);

		const ImVec4 BackgroundColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
		const ImVec4 SelectedColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		const ImVec4 TintColor = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);

		{
			ImGui::TableSetColumnIndex(0);
			std::shared_ptr<FImage> ImageVisible = bVisible ? ImageVisibleEnable : ImageVisibleDisable;
			if (ButtonExLambda("Sequencer##BaseVisible", *ImageVisible, BackgroundColor, TintColor, SelectedColor))
			{
				bVisible = !bVisible;
				for (uint32_t RowIndex = 0; RowIndex < Sprite->Layers.size(); ++RowIndex)
				{
					FSpriteLayer& SpriteLayer = Sprite->Layers[RowIndex];
					SpriteLayer.bVisible = bVisible;
				}
			}

			ImGui::TableSetColumnIndex(1);
			std::shared_ptr<FImage> ImageLocking = bLock ? ImageLock : ImageUnlock;
			if (ButtonExLambda("Sequencer##BaseLock", *ImageLocking, BackgroundColor, TintColor, SelectedColor))
			{
				bLock = !bLock;
				for (uint32_t RowIndex = 0; RowIndex < Sprite->Layers.size(); ++RowIndex)
				{
					FSpriteLayer& SpriteLayer = Sprite->Layers[RowIndex];
					SpriteLayer.bLock = bLock;
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

			FSpriteLayer& SpriteLayer = Sprite->Layers[RowIndex];

			ImGui::TableSetColumnIndex(0);
			const std::string LayerVisibleName = Utils::Format("Sequencer##Visible%i", RowIndex);
			std::shared_ptr<FImage> ImageVisible = SpriteLayer.bVisible ? ImageVisibleEnable : ImageVisibleDisable;
			if (ButtonExLambda(LayerVisibleName.c_str(), *ImageVisible, BackgroundColor, TintColor, SelectedColor))
			{
				SpriteLayer.bVisible = !SpriteLayer.bVisible;
			}

			ImGui::TableSetColumnIndex(1);
			const std::string LayerLockName = Utils::Format("Sequencer##Lock%i", RowIndex);
			std::shared_ptr<FImage> ImageLocking = SpriteLayer.bLock ? ImageLock : ImageUnlock;
			if (ButtonExLambda(LayerLockName.c_str(), *ImageLocking, BackgroundColor, TintColor, SelectedColor))
			{
				SpriteLayer.bLock = !SpriteLayer.bLock;
			}

			ImGui::TableSetColumnIndex(2);
			ImGui::TextUnformatted(SpriteLayer.Name.c_str());

			for (uint32_t FrameIndex = 0; FrameIndex < Sprite->NumFrame; ++FrameIndex)
			{
				ImGui::TableNextColumn();

				std::shared_ptr<FImage> ImageFrame = SpriteLayer.bEmpty ? ImageEmpty : ImageFill;
				const std::string LayerFrameName = Utils::Format("Sequencer##Frame%i", FrameIndex);
				ButtonExLambda(LayerFrameName.c_str(), *ImageFrame, BackgroundColor, TintColor, SelectedColor);
			}
		}
		ImGui::PopStyleVar();
		ImGui::EndTable();
	}
}
