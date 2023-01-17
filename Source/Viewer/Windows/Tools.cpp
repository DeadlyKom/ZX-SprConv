#include "Tools.h"
#include "Core\Utils.h"
#include "Core\Image.h"

void STools::Initialize()
{
	bIncludeInWindows = true;
	Name = "Tools";

	ImageMarquee = Utils::LoadImageFromResource(IDB_TOOL_MARQUEE, TEXT("PNG"));
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
	ImVec4 TintColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	ImGui::ImageButton("Tools##Marquee", ImageMarquee->GetShaderResourceView(), ImageMarquee->Size, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), BackgroundColor, TintColor);
	ImGui::End();
}
