#include "Palette.h"

void SPalette::Initialize()
{
	bIncludeInWindows = true;
	Name = "Palette";
}

void SPalette::Render()
{
	if (!IsOpen())
	{
		Close();
		return;
	}

	ImGui::Begin("Palette", &bOpen);
	ImGui::End();
}
