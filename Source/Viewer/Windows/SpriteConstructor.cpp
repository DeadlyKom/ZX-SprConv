#include "SpriteConstructor.h"

void SSpriteConstructor::Initialize()
{
	bIncludeInWindows = true;
	Name = "Sprite Constructor";
}

void SSpriteConstructor::Render()
{
	if (!IsOpen())
	{
		Close();
		return;
	}

	ImGui::Begin("SpriteConstructor", &bOpen);

	RenderSpriteList();

	ImGui::End();
}

void SSpriteConstructor::RenderSpriteList()
{

}
