#include "Tools.h"
#include "Core\Utils.h"
#include "Core\Image.h"

void STools::Initialize()
{
	bIncludeInWindows = true;
	Name = "Tools";

	ImageMarquee = Utils::LoadImageFromResource(IDB_TOOL_MARQUEE, TEXT("PNG"));
	ImagePan = Utils::LoadImageFromResource(IDB_TOOL_PAN, TEXT("PNG"));
	ImageEraser = Utils::LoadImageFromResource(IDB_TOOL_ERASER, TEXT("PNG"));
	ImageHand = Utils::LoadImageFromResource(IDB_TOOL_HAND, TEXT("PNG"));
	ImageMove = Utils::LoadImageFromResource(IDB_TOOL_MOVE, TEXT("PNG"));
}

void STools::Render()
{
	if (!IsOpen())
	{
		Close();
		return;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(50.0f, 65.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1.0f, 1.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1.0f, 1.0f));
	ImGui::Begin("Tools", &bOpen, ImGuiWindowFlags_NoScrollbar);

	const float DefaultWidth = ImGui::GetContentRegionAvail().x;
	auto ButtonLambda = [DefaultWidth](const char* ID, std::shared_ptr<FImage> Image, bool bSelectedCondition, float& AvailWidth) -> bool
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
		if (AvailWidth > Width)
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
	if (ButtonLambda("Tools##Marquee", ImageMarquee, IsEqualSelected(EToolType::Marquee), AvailWidth))
	{
		SetSelect(EToolType::Marquee);
	}
	if (ButtonLambda("Tools##Pan", ImagePan, IsEqualSelected(EToolType::Pan), AvailWidth))
	{
		SetSelect(EToolType::Pan);
	}
	if (ButtonLambda("Tools##Eraser", ImageEraser, IsEqualSelected(EToolType::Eraser), AvailWidth))
	{
		SetSelect(EToolType::Eraser);
	}
	if (ButtonLambda("Tools##Hand", ImageHand, IsEqualSelected(EToolType::Hand), AvailWidth))
	{
		SetSelect(EToolType::Hand);
	}
	if (ButtonLambda("Tools##Move", ImageMove, IsEqualSelected(EToolType::Move), AvailWidth))
	{
		SetSelect(EToolType::Move);
	}
	ImGui::End();
	ImGui::PopStyleVar(3);

	if (false)
	{
		switch (Selected)
		{
		case EToolType::None:
			ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
			break;
		case EToolType::Marquee:
			ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
			break;
		case EToolType::Pan:
			ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
			break;
		case EToolType::Eraser:
			ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
			break;
		case EToolType::Hand:
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
			break;
		case EToolType::Move:
			ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
			break;
		default:
			ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
			break;
		}
	}
}

EToolType STools::SetSelect(EToolType NewSelecte)
{
	EToolType Last = Selected;
	Selected = NewSelecte;
	return Last;
}
