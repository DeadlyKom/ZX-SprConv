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
{}

std::shared_ptr<SWindow> SViewer::GetWindow(EWindowsType Type)
{
	const std::map<EWindowsType, std::shared_ptr<SWindow>>::iterator& SearchIt = Windows.find(Type);
	return SearchIt != Windows.end() ? SearchIt->second : nullptr;
}

void SViewer::NativeInitialize(FNativeDataInitialize Data)
{
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
{}

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
