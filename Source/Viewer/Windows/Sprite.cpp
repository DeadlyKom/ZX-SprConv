#include "Sprite.h"

void SSprite::Initialize()
{
	bIncludeInWindows = true;
	Name = "Sprite Editor";
}

void SSprite::Render()
{
	if (!IsOpen())
	{
		Close();
		return;
	}

	ImGui::Begin("Sprite Editor", &bOpen);
	ImGui::End();
}
