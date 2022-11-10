#include "Viewer.h"
#include "Core\Window.h"

#include "Windows/Sprite.h"
#include "Windows/ImageList.h"
#include "Windows/Palette.h"

SViewer::SViewer()
{}

void SViewer::Initialize()
{
	Windows = { std::make_shared<SSprite>(),
				std::make_shared<SImageList>(),
				std::make_shared<SPalette>(), };

	for (int i = 0; i < Windows.size(); i++)
	{
		Windows.at(i)->Initialize();
	}
}

void SViewer::Shutdown()
{

}

void SViewer::Render()
{
	ImGui::DockSpaceOverViewport();

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Windows"))
		{
			for (int32_t i = 0; i < Windows.size(); i++)
			{
				if (Windows.at(i)->IsIncludeInWindows())
				{
					if (ImGui::MenuItem(Windows.at(i)->GetName().c_str(), 0, Windows.at(i)->IsOpen()))
					{
						if (Windows.at(i)->IsOpen())
						{
							Windows.at(i)->Close();
						}
						else
						{
							Windows.at(i)->Open();
						}
					}
				}
			}

			ImGui::EndMenu();
		}

		//if (ImGui::BeginMenu("Options"))
		//{
		//	ImGui::MenuItem("Only affects the UI. For example,", 0, false, false);
		//	ImGui::MenuItem("how often the Registers", 0, false, false);
		//	ImGui::MenuItem("Window updates values etc.", 0, false, false);
		//	if (ImGui::DragInt("FPS", &_TargetFPS, 1, 5, 60))
		//	{
		//		SetFPS(_TargetFPS);
		//	}
		//	ImGui::MenuItem("Default is 3.2mhz.", 0, false, false);
		//	ImGui::MenuItem("Beware: It will affect DELA/DELB.", 0, false, false);
		//	if (ImGui::DragFloat("CPU Clock", &cpu_speed, 0.1f, 0.1f, 10.0f, "%.1fmhz"))
		//	{
		//		if (cpu_speed < 0.1f)
		//			cpu_speed = 0.1f;
		//		if (cpu_speed > 10.0f)
		//			cpu_speed = 10.0f;
		//		Simulation::SetClock((int)(cpu_speed * 1000000), cpu_accuracy);
		//	}
		//	ImGui::MenuItem("CPU cycles are divided into steps.", 0, false, false);
		//	ImGui::MenuItem("Warning: Too many slows the clock speed.", 0, false, false);
		//	if (ImGui::DragInt("Clock Accuracy", &cpu_accuracy, 1, 10, 1000))
		//	{
		//		if (cpu_accuracy < 1)
		//			cpu_accuracy = 1;
		//		Simulation::SetClock((int)(cpu_speed * 1000000), cpu_accuracy);
		//	}
		//	ImGui::EndMenu();
		//}

		ImGui::EndMainMenuBar();
	}

	for (int32_t i = 0; i < Windows.size(); i++)
	{
		Windows.at(i)->Render();
	}
}
