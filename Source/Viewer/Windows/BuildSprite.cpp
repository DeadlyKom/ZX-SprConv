#include "BuildSprite.h"

void SBuildSprite::Initialize()
{
	bIncludeInWindows = true;
	Name = "Build Sprite";
}

void SBuildSprite::Render()
{
	if (!IsOpen())
	{
		Close();
		return;
	}

	ImGui::Begin("Build Sprite", &bOpen);
	ImGui::End();
}
