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

	ImGui::Begin("Tools", &bOpen, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);

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
	if (ButtonLambda("Tools##Marquee", ImageMarquee, Selected == EToolType::Marquee, AvailWidth))
	{
		Selected = EToolType::Marquee;
	}
	if (ButtonLambda("Tools##Pan", ImagePan, Selected == EToolType::Pan, AvailWidth))
	{
		Selected = EToolType::Pan;
	}
	if (ButtonLambda("Tools##Eraser", ImageEraser, Selected == EToolType::Eraser, AvailWidth))
	{
		Selected = EToolType::Eraser;
	}
	if (ButtonLambda("Tools##Hand", ImageHand, Selected == EToolType::Hand, AvailWidth))
	{
		Selected = EToolType::Hand;
	}
	if (ButtonLambda("Tools##Move", ImageMove, Selected == EToolType::Move, AvailWidth))
	{
		Selected = EToolType::Move;
	}

	ImGui::End();
}

EToolType STools::SetSelect(EToolType NewSelecte)
{
	EToolType Last = Selected;
	Selected = NewSelecte;
	return Last;
}
