#include "SetSprite.h"

void SSetSprite::Initialize()
{
	bIncludeInWindows = true;
	Name = "Set Sprite";
}

void SSetSprite::Render()
{
	if (!IsOpen())
	{
		Close();
		return;
	}

	ImGui::Begin("Set Sprite", &bOpen);
	ImGui::End();
}
