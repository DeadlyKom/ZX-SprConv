#include "Viewer.h"
#include "Core\Window.h"

#include "Windows/Sprite.h"
#include "Windows/Tools.h"
#include "Windows/ImageList.h"
#include "Windows/Palette.h"
#include "Windows/BuildSprite.h"
#include "Windows/Sequencer.h"
#include "Windows/SetSprite.h"

SViewer::SViewer()
	: LastSelectedTool(EToolType::None)
{}

std::shared_ptr<SWindow> SViewer::GetWindow(EWindowsType Type)
{
	const std::map<EWindowsType, std::shared_ptr<SWindow>>::iterator& SearchIt = Windows.find(Type);
	return SearchIt != Windows.end() ? SearchIt->second : nullptr;
}

void SViewer::NativeInitialize(FNativeDataInitialize Data)
{
	Initialize();

	Windows = { { EWindowsType::ImageList,		std::make_shared<SImageList>()	},
				{ EWindowsType::Tools,			std::make_shared<STools>()		},
				{ EWindowsType::Sprite,			std::make_shared<SSprite>()		},
				{ EWindowsType::Palette,		std::make_shared<SPalette>()	},
				{ EWindowsType::BuildSprite,	std::make_shared<SBuildSprite>()},
				{ EWindowsType::Sequencer,		std::make_shared<SSequencer>()	},
				{ EWindowsType::SetSprite,		std::make_shared<SSetSprite>()	},
			  };
	for (std::pair<EWindowsType, std::shared_ptr<SWindow>> Window : Windows)
	{
		Data.Parent = shared_from_this();
		Window.second->NativeInitialize(Data);
	}
}

void SViewer::Initialize()
{
	FSprite NewSprite;
	NewSprite.NumFrame = 6;
	NewSprite.Size = ImVec2(64.0f, 64.0f);
	NewSprite.Pivot = ImVec2(32.0f, 32.0f);
	NewSprite.Name = "Demo";

	FSpriteLayer NewLayer1;
	NewLayer1.bVisible = true;
	NewLayer1.bLock = false;
	NewLayer1.Name = "Layer 1";
	NewSprite.Layers.push_back(NewLayer1);

	FSpriteLayer NewLayer2;
	NewLayer2.bVisible = true;
	NewLayer2.bLock = false;
	NewLayer2.Name = "Layer 2";
	NewSprite.Layers.push_back(NewLayer2);
	
	Sprites.push_back(NewSprite);
}

void SViewer::Render()
{
	ImGui::DockSpaceOverViewport();

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Windows"))
		{
			for (std::pair<EWindowsType, std::shared_ptr<SWindow>> Window : Windows)
			{
				if (Window.second->IsIncludeInWindows())
				{
					if (ImGui::MenuItem(Window.second->GetName().c_str(), 0, Window.second->IsOpen()))
					{
						if (Window.second->IsOpen())
						{
							Window.second->Close();
						}
						else
						{
							Window.second->Open();
						}
					}
				}
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	for (std::pair<EWindowsType, std::shared_ptr<SWindow>> Window : Windows)
	{
		Window.second->Render();
	}
}

void SViewer::Tick(float DeltaTime)
{
	for (std::pair<EWindowsType, std::shared_ptr<SWindow>> Window : Windows)
	{
		Window.second->Tick(DeltaTime);
	}
}

void SViewer::Destroy()
{
	for (std::pair<EWindowsType, std::shared_ptr<SWindow>> Window : Windows)
	{
		Window.second->Destroy();
	}
}

FSprite& SViewer::GetSelectedSprite()
{
	return Sprites[0];
}

bool SViewer::IsHandTool()
{
	return WindowCast<STools>(EWindowsType::Tools)->GetSelected() == EToolType::Hand;
}

bool SViewer::IsMarqueeTool()
{
	return WindowCast<STools>(EWindowsType::Tools)->GetSelected() == EToolType::Marquee;
}

void SViewer::HandlerInput()
{
	const ImGuiIO& IO = ImGui::GetIO();
	const bool Shift = IO.KeyShift;
	const bool Ctrl = IO.ConfigMacOSXBehaviors ? IO.KeySuper : IO.KeyCtrl;
	const bool Alt = IO.ConfigMacOSXBehaviors ? IO.KeyCtrl : IO.KeyAlt;
	
	//if (!ImGui::IsWindowFocused())
	//{
	//	return;
	//}

	if (ImGui::IsKeyPressed(ImGuiMod_Ctrl))
	{
		EToolType TmpLastSelectedTool = WindowCast<STools>(EWindowsType::Tools)->SetSelect(EToolType::Move);
		if (TmpLastSelectedTool != EToolType::Move)
		{
			LastSelectedTool = TmpLastSelectedTool;
		}
	}
	else if(ImGui::IsKeyReleased(ImGuiMod_Ctrl))
	{
		WindowCast<STools>(EWindowsType::Tools)->SetSelect(LastSelectedTool);
	}
	else if (IO.MouseDown[ImGuiMouseButton_Middle])
	{
		EToolType TmpLastSelectedTool = WindowCast<STools>(EWindowsType::Tools)->SetSelect(EToolType::Hand);
		if (TmpLastSelectedTool != EToolType::Hand)
		{
			LastSelectedTool = TmpLastSelectedTool;
		}
	}
	else if (IO.MouseReleased[ImGuiMouseButton_Middle])
	{
		WindowCast<STools>(EWindowsType::Tools)->SetSelect(LastSelectedTool);
	}

	// hot keys
	if (IO.KeysDown[ImGui::GetKeyIndex(ImGuiKey_M)])
	{
		WindowCast<STools>(EWindowsType::Tools)->SetSelect(EToolType::Marquee);
	}
	else if (IO.KeysDown[ImGui::GetKeyIndex(ImGuiKey_B)])
	{
		WindowCast<STools>(EWindowsType::Tools)->SetSelect(EToolType::Pan);
	}
	else if (IO.KeysDown[ImGui::GetKeyIndex(ImGuiKey_E)])
	{
		WindowCast<STools>(EWindowsType::Tools)->SetSelect(EToolType::Eraser);
	}
}
