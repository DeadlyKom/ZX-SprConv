#include "Sequencer.h"

void SSequencer::Initialize()
{
	bIncludeInWindows = true;
	Name = "Sequencer";
}

void SSequencer::Render()
{
	if (!IsOpen())
	{
		Close();
		return;
	}

	ImGui::Begin("Sequencer", &bOpen);
	ImGui::End();
}
