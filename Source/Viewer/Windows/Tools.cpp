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

	ImGui::Begin("Tools", &bOpen);
	ImVec4 BackgroundColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	ImVec4 TintColor = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);
	ImVec4 SelectedColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	if (ImGui::ImageButton("Tools##Marquee", ImageMarquee->GetShaderResourceView(), ImageMarquee->Size, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), BackgroundColor, Selected == EToolType::Marquee ? SelectedColor : TintColor))
	{
		Selected = EToolType::Marquee;
	}
	else if (ImGui::ImageButton("Tools##Pan", ImagePan->GetShaderResourceView(), ImagePan->Size, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), BackgroundColor, Selected == EToolType::Pan ? SelectedColor : TintColor))
	{
		Selected = EToolType::Pan;
	}
	else if (ImGui::ImageButton("Tools##Eraser", ImageEraser->GetShaderResourceView(), ImageEraser->Size, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), BackgroundColor, Selected == EToolType::Eraser ? SelectedColor : TintColor))
	{
		Selected = EToolType::Eraser;
	}
	else if (ImGui::ImageButton("Tools##Hand", ImageHand->GetShaderResourceView(), ImageHand->Size, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), BackgroundColor, Selected == EToolType::Hand ? SelectedColor : TintColor))
	{
		Selected = EToolType::Hand;
	}
	else if (ImGui::ImageButton("Tools##Move", ImageMove->GetShaderResourceView(), ImageMove->Size, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), BackgroundColor, Selected == EToolType::Move ? SelectedColor : TintColor))
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
