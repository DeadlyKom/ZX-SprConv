#include "Viewer.h"
#include "Core\Window.h"
#include "Core\Utils.h"

#include "Windows/Sprite.h"
#include "Windows/Tools.h"
#include "Windows/ImageList.h"
#include "Windows/Palette.h"
#include "Windows/BuildSprite.h"
#include "Windows/Sequencer.h"
#include "Windows/SetSprite.h"

#include "Viewer\Windows\FileDialog.h"
namespace
{
	const char* MenuFileName = "File";
	const char* MenuQuitName = "Quit";
}

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
	NewSprite.NumFrame = 1;
	NewSprite.Size = ImVec2(64.0f, 64.0f);
	NewSprite.Pivot = ImVec2(32.0f, 32.0f);
	NewSprite.Name = "Demo";

	FSpriteLayer NewLayer1;
	NewLayer1.bVisible = true;
	NewLayer1.bLock = false;
	NewLayer1.Name = "Layer 1";
	NewSprite.Layers.push_back(NewLayer1);

	//FSpriteLayer NewLayer2;
	//NewLayer2.bVisible = true;
	//NewLayer2.bLock = false;
	//NewLayer2.Name = "Layer 2";
	//NewSprite.Layers.push_back(NewLayer2);
	
	Sprites.push_back(NewSprite);
}

void SViewer::Render()
{
	HandlerInput();

	ImGui::DockSpaceOverViewport();

	if (ImGui::BeginMainMenuBar())
	{
		ShowMenuFile();
		ShowMenuSprite();
		ShowMenuLayer();
		ShowMenuFrame();
		ShowMenuView();
		ShowWindows();

		ImGui::EndMainMenuBar();
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

void SViewer::ShowMenuFile()
{
	const ImGuiID QuitID = ImGui::GetCurrentWindow()->GetID(MenuQuitName);

	if (ImGui::BeginMenu(MenuFileName))
	{
		if (ImGui::MenuItem("New")) {}
		if (ImGui::MenuItem("Open", "Ctrl+O"))
		{
			const std::string OldPath = Files.empty() ? "" : Files.back().path().parent_path().string();
			FileDialogHandle = Utils::OpenWindowFileDialog("Select File", EDialogMode::Select, [this](std::filesystem::path FilePath) -> void
			{
				std::filesystem::directory_entry File(FilePath);
				if(File.exists())
				{
					Files.push_back(File);
					WindowCast<SImageList>(EWindowsType::ImageList)->OnSelectedImage.Broadcast(Files.back());
				}
				else
				{
					ImGui::LogText("Error");
				}
				Utils::CloseWindowFileDialog(FileDialogHandle);
			}, OldPath, "*.*, *.png, *.scr");
		}

		if (RecentFiles.size() > 0)
		{
			if (ImGui::BeginMenu("Open Recent"))
			{
				for (const FRecentFiles& RecentFile : RecentFiles)
				{
					ImGui::MenuItem(RecentFile.VisibleName.c_str());
				}

				ImGui::Separator();
				if (ImGui::MenuItem("Clear Recent Files"))
				{
					RecentFiles.clear();
				}

				ImGui::EndMenu();
			}
		}
		ImGui::Separator();

		if (ImGui::MenuItem("Save", "Ctrl+S")) {}
		if (ImGui::MenuItem("Save As..")) {}
		ImGui::Separator();

		if (ImGui::MenuItem(MenuQuitName, "Alt+F4"))
		{
			if (ViewFlags.bDontAskMeNextTime_Quit)
			{
				Close();
			}
			else
			{
				ImGui::OpenPopup(QuitID);
			}
		}

		ImGui::EndMenu();
	}

	{
		// Always center this window when appearing
		const ImVec2 Center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(Center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (ImGui::BeginPopupModal(MenuQuitName, NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("All those beautiful files will be deleted.\nThis operation cannot be undone!\n\n");
			ImGui::Separator();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
			ImGui::Checkbox("Don't ask me next time", &ViewFlags.bDontAskMeNextTime_Quit);
			ImGui::PopStyleVar();

			if (ImGui::Button("OK", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
				Close();
			}
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}
}

void SViewer::ShowMenuSprite()
{
	if (ImGui::BeginMenu("Sprite"))
	{
		if (ImGui::MenuItem("New")) {}
		if (ImGui::MenuItem("Properties...", "Ctrl+P")) {}
		if (ImGui::BeginMenu("Color Mode"))
		{
			if (ImGui::MenuItem("RGB Color", NULL, true)) {}
			if (ImGui::MenuItem("Indexed", NULL, false)) {}

			ImGui::EndMenu();
		}
		ImGui::Separator();

		if (ImGui::MenuItem("Sprite Size...")) {}
		ImGui::EndMenu();
	}
}

void SViewer::ShowMenuLayer()
{

}

void SViewer::ShowMenuFrame()
{

}

void SViewer::ShowMenuView()
{
	if (ImGui::BeginMenu("View"))
	{
		if (ImGui::BeginMenu("Show"))
		{
			if (ImGui::MenuItem("Layer Edges", NULL, true)) {}
			if (ImGui::MenuItem("Attribute Grid", NULL, ViewFlags.bAttributeGrid))
			{
				ViewFlags.bAttributeGrid = !ViewFlags.bAttributeGrid;
			}
			if (ImGui::MenuItem("Pixel Grid", NULL, ViewFlags.bPixelGrid))
			{
				ViewFlags.bPixelGrid = !ViewFlags.bPixelGrid;
			}
			if (ImGui::MenuItem("Grid", NULL, ViewFlags.bGrid))
			{
				ViewFlags.bGrid = !ViewFlags.bGrid;
			}

			ImGui::EndMenu();
		}
		ImGui::Separator();

		if (ImGui::MenuItem("Grid Settings")) {}

		ImGui::EndMenu();
	}

}

void SViewer::ShowWindows()
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

	for (std::pair<EWindowsType, std::shared_ptr<SWindow>> Window : Windows)
	{
		Window.second->Render();
	}
}
