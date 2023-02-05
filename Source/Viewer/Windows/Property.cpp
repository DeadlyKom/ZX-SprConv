#include "Property.h"

void SProperty::Initialize()
{
	bIncludeInWindows = true;
	Name = "Property";
}

void SProperty::Render()
{
	if (!IsOpen())
	{
		Close();
		return;
	}

	ImGui::Begin("Property", &bOpen);
	ImGui::End();
}
