#include "Viewer.h"
#include "Core\Window.h"
#include "Core\Utils.h"

#include "Windows/SpriteEditor.h"
#include "Windows/Tools.h"
#include "Windows/ImageList.h"
#include "Windows/Palette.h"
#include "Windows/SpriteConstructor.h"
#include "Windows/Sequencer.h"
#include "Windows/Property.h"

#include "Viewer\Windows\FileDialog.h"

namespace
{
	const char* MenuFileName = "File";
	const char* MenuQuitName = "Quit";

	const char* CreateSpriteName = "Create Sprite";
}

SViewer::SViewer()
	: LastSelectedTool(EToolType::None)
	, bCreateSpriteFirstOpen(false)
	, CurrentSprite(-1)
	, SpriteCounter(0)
	, LayersCounter(0)
{}

std::shared_ptr<SWindow> SViewer::GetWindow(EWindowsType Type)
{
	const std::map<EWindowsType, std::shared_ptr<SWindow>>::iterator& SearchIt = Windows.find(Type);
	return SearchIt != Windows.end() ? SearchIt->second : nullptr;
}

void SViewer::NativeInitialize(FNativeDataInitialize Data)
{
	Initialize();

	Windows = { { EWindowsType::ImageList,			std::make_shared<SImageList>()			},
				{ EWindowsType::Tools,				std::make_shared<STools>()				},
				{ EWindowsType::SpriteEditor,		std::make_shared<SSpriteEditor>()		},
				{ EWindowsType::Palette,			std::make_shared<SPalette>()			},
				{ EWindowsType::SpriteConstructor,	std::make_shared<SSpriteConstructor>()	},
				{ EWindowsType::Sequencer,			std::make_shared<SSequencer>()			},
				{ EWindowsType::Property,			std::make_shared<SProperty>()			},
			  };
	for (std::pair<EWindowsType, std::shared_ptr<SWindow>> Window : Windows)
	{
		Data.Parent = shared_from_this();
		Window.second->NativeInitialize(Data);
	}
}

void SViewer::Initialize()
{
	//FSprite NewSprite;
	//NewSprite.NumFrame = 1;
	//NewSprite.Size = ImVec2(64.0f, 64.0f);
	//NewSprite.Pivot = ImVec2(32.0f, 32.0f);
	//NewSprite.Name = "Demo";

	//FSpriteLayer NewLayer1;
	//NewLayer1.bVisible = true;
	//NewLayer1.bLock = false;
	//NewLayer1.Name = "Layer 1";
	//NewSprite.Layers.push_back(NewLayer1);

	////FSpriteLayer NewLayer2;
	////NewLayer2.bVisible = true;
	////NewLayer2.bLock = false;
	////NewLayer2.Name = "Layer 2";
	////NewSprite.Layers.push_back(NewLayer2);
	//
	//Sprites.push_back(NewSprite);

	ImageRGBA = Utils::LoadImageFromResource(IDB_COLOR_MODE_RGBA, TEXT("PNG"));
	ImageIndexed = Utils::LoadImageFromResource(IDB_COLOR_MODE_INDEXED, TEXT("PNG"));
	ImageZX = Utils::LoadImageFromResource(IDB_COLOR_MODE_ZX, TEXT("PNG"));

	if (true)
	{
		OpenFile_Callback("C:\\Work\\Sprites\\Menu\\Change Mission\\interact - 7.png");
		OpenFile_Callback("C:\\Work\\Sprites\\Tiles\\Pack_1.2.png");
		OpenFile_Callback("C:\\Work\\Sprites\\Tank\\1.png");
		OpenFile_Callback("C:\\Work\\Sprites\\Tank\\Up.png");
		OpenFile_Callback("C:\\Work\\Sprites\\Tank\\Down.png");
		OpenFile_Callback("C:\\Work\\Sprites\\Solder_dead\\Solder.png");
		OpenFile_Callback("C:\\Work\\Sprites\\Mothership\\PNG\\Tier 2-3 & A-B Land 1.png");
		OpenFile_Callback("C:\\Work\\Sprites\\Mothership\\PNG\\Tier 2-3 & A-B Land 2.png");
		OpenFile_Callback("C:\\Work\\Sprites\\Mothership\\PNG\\Tier 2-3 & A-B Land 3.png");
		OpenFile_Callback("C:\\Work\\Sprites\\Mothership\\PNG\\Tier 2-3 & A-B Land 4.png");
		OpenFile_Callback("C:\\Work\\Sprites\\Raven my.png");
	}
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

FSprite* SViewer::GetSelectedSprite()
{
	const int32_t Index = CurrentSprite >= 0 ? (Sprites.size() > CurrentSprite ? CurrentSprite : -1) : -1;
	return Index >= 0 ? &Sprites[Index] : nullptr;
}

bool SViewer::IsHandTool()
{
	return WindowCast<STools>(EWindowsType::Tools)->GetSelected() == EToolType::Hand;
}

bool SViewer::IsMarqueeTool()
{
	return WindowCast<STools>(EWindowsType::Tools)->GetSelected() == EToolType::Marquee;
}

int SViewer::TextEditNumberCallback(ImGuiInputTextCallbackData* Data)
{
	switch (Data->EventFlag)
	{
	case ImGuiInputTextFlags_CallbackCharFilter:
		if (Data->EventChar < '0' || Data->EventChar > '9')
		{
			return 1;
		}
		break;
	case ImGuiInputTextFlags_CallbackEdit:
		float Value = 0;
		if (strlen(Data->Buf) > 1)
		{
			Value = float(std::stoi(Data->Buf));
		}
		*(float*)Data->UserData = Value;
		break;
	}
	return 0;
}

void SViewer::OpenFile_Callback(std::filesystem::path FilePath)
{
	std::filesystem::directory_entry File(FilePath);
	if (File.exists())
	{
		// поиск одинаковых файлов
		bool bPresent = false;
		for (std::vector<std::filesystem::directory_entry>::iterator FileIt = Files.begin(); FileIt != Files.end(); ++FileIt)
		{
			if (FileIt->path() == File.path())
			{
				bPresent = true;
				break;
			}
		}

		if (!bPresent)
		{
			Files.push_back(File);
			SImageList* ImageList = WindowCast<SImageList>(EWindowsType::ImageList);
			if (ImageList != nullptr)
			{
				ImageList->OnSelectedImage.Broadcast(Files.back());
			}
		}
	}
	else
	{
		ImGui::LogText("Error");
	}
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
		/*if (ImGui::BeginMenu("New"))
		{
			if (ImGui::MenuItem("Sprite", "Ctrl+N"))
			{
				ImGui::OpenPopup(CreateSpriteID);
			}
			if (ImGui::MenuItem("Sprite Preset", "Ctrl+Shift+N")) {}

			ImGui::EndMenu();
		}*/

		if (ImGui::MenuItem("Open", "Ctrl+O"))
		{
			const std::string OldPath = Files.empty() ? "" : Files.back().path().parent_path().string();
			FileDialogHandle = Utils::OpenWindowFileDialog("Select File", EDialogMode::Select,
			[this](std::filesystem::path FilePath) -> void
			{
				OpenFile_Callback(FilePath);
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
		if (WindowQuitModal()) {}
		else if (WindowCreateSpriteModal()) {}
	}
}

void SViewer::ShowMenuSprite()
{
	const ImGuiID CreateSpriteID = ImGui::GetCurrentWindow()->GetID(CreateSpriteName);
	if (ImGui::BeginMenu("Sprite"))
	{
		if (ImGui::MenuItem("New", "Ctrl+N"))
		{
			ImGui::OpenPopup(CreateSpriteID);
		}
		if (ImGui::MenuItem("Properties...", "Ctrl+P")) {}
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

bool SViewer::WindowQuitModal()
{
	// Always center this window when appearing
	const ImVec2 Center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(Center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	const bool bVisible = ImGui::BeginPopupModal(MenuQuitName, NULL, ImGuiWindowFlags_AlwaysAutoResize);
	if (bVisible)
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
	return bVisible;
}

bool SViewer::WindowCreateSpriteModal()
{
	// Always center this window when appearing
	const ImVec2 Center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(Center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	auto ButtonLambda = [](const char* StringID, ImTextureID TextureID, const ImVec2& ImageSize, const ImVec2& Size, bool bSelectedCondition, const ImVec4& BackgroundColor, const ImVec4& TintColor, const ImVec4& SelectedColor, const ImVec4& TextColor) -> bool
	{
		ImGuiWindow* Window = ImGui::GetCurrentWindow();
		if (Window->SkipItems)
		{
			return false;
		}

		const ImGuiID ID = Window->GetID(StringID);
		const ImGuiStyle& Style = ImGui::GetStyle();
		const ImVec2 LabelSize = ImGui::CalcTextSize(StringID, NULL, true);
		const ImVec2 NewSize = ImGui::CalcItemSize(Size, ImMax(ImageSize.x, LabelSize.x) + Style.FramePadding.x * 2.0f, ImageSize.y + LabelSize.y + Style.FramePadding.y * 2.0f);

		const ImVec2 Padding = Style.FramePadding;
		const ImRect bb(Window->DC.CursorPos, Window->DC.CursorPos + NewSize + Padding * 2.0f);
		ImGui::ItemSize(bb);
		if (!ImGui::ItemAdd(bb, ID))
		{
			return false;
		}

		bool bHovered, bHeld;
		bool bPressed = ImGui::ButtonBehavior(bb, ID, &bHovered, &bHeld);

		// Render
		//ImGui::RenderNavHighlight(bb, ID);
		//if (!(Window->Flags & ImGuiWindowFlags_NoBackground))
		//{
		//	const ImU32 Color = ImGui::GetColorU32((bHeld && bHovered) ? ImGuiCol_ButtonActive : bHovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
		//	ImGui::RenderFrame(bb.Min + Align, bb.Max + Align, Color, true, ImClamp((float)ImMin(Padding.x, Padding.y), 0.0f, Style.FrameRounding));
		//}
		//if (BackgroundColor.w > 0.0f)
		//{
		//	Window->DrawList->AddRectFilled(bb.Min + Padding + Align, bb.Max - Padding + Align, ImGui::GetColorU32(BackgroundColor));
		//}

		if (bSelectedCondition)
		{
			ImGui::RenderTextClipped(bb.Min + Style.FramePadding, bb.Max - Style.FramePadding, StringID, NULL, &LabelSize, Style.ButtonTextAlign, &bb);
		}
		Window->DrawList->AddImage(TextureID, bb.Min + Padding , bb.Max - Padding, ImVec2(0.0, 0.0f), ImVec2(1.0, 1.0f), (bSelectedCondition | !bHovered) ? ImGui::GetColorU32(TintColor) : ImGui::GetColorU32(SelectedColor));
		//;
		return bPressed;
	};

	const bool bVisible = ImGui::BeginPopupModal(CreateSpriteName, NULL, ImGuiWindowFlags_AlwaysAutoResize);
	if (bVisible)
	{
		if (!bCreateSpriteFirstOpen)
		{
			bCreateSpriteFirstOpen = true;

			CreateSpriteSize = ImVec2(32, 32);
			CreateSpritePivot = CreateSpriteSize * 0.5f;

			sprintf(CreateSpriteNameBuffer, Utils::Format("Sprite %i", ++SpriteCounter).c_str());
			sprintf(CreateSpriteWidthBuffer, "%i\n", int(CreateSpriteSize.x));
			sprintf(CreateSpriteHeightBuffer, "%i\n", int(CreateSpriteSize.y));
		}

		const float TextWidth = ImGui::CalcTextSize("A").x;
		const float TextHeight = ImGui::GetTextLineHeightWithSpacing();

		ImGui::Dummy(ImVec2(0.0f, TextHeight * 0.5f));
		ImGui::InputTextEx("Name ", NULL, CreateSpriteNameBuffer, IM_ARRAYSIZE(CreateSpriteWidthBuffer), ImVec2(TextWidth * 20.0f, TextHeight), ImGuiInputTextFlags_None, &TextEditNumberCallback, (void*)this);

		ImGui::Dummy(ImVec2(0.0f, TextHeight * 0.5f));
		ImGui::Text("Size :");
		ImGui::Separator();

		const ImGuiInputTextFlags InputNumberTextFlags = ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CallbackCharFilter | ImGuiInputTextFlags_CallbackEdit;
		ImGui::InputTextEx("Width ", NULL, CreateSpriteWidthBuffer, IM_ARRAYSIZE(CreateSpriteWidthBuffer), ImVec2(TextWidth * 10.0f, TextHeight), InputNumberTextFlags, &TextEditNumberCallback, (void*)&CreateSpriteSize.x);
		ImGui::InputTextEx("Input ", NULL, CreateSpriteHeightBuffer, IM_ARRAYSIZE(CreateSpriteHeightBuffer), ImVec2(TextWidth * 10.0f, TextHeight), InputNumberTextFlags, &TextEditNumberCallback, (void*)&CreateSpriteSize.y);

		ImGui::Dummy(ImVec2(0.0f, TextHeight * 1.0f));
		ImGui::Text("Pivot :");
		ImGui::Separator();

		CreateSpritePivot = ImClamp(CreateSpritePivot, ImVec2(0.0f, 0.0f), ImMax(CreateSpriteSize - ImVec2(1.0f, 1.0f), ImVec2(0.0f, 0.0f)));
		ImGui::SliderFloat("X ", &CreateSpritePivot.x, 0, ImClamp(CreateSpriteSize.x, 0.0f, CreateSpriteSize.x - 1.0f), "%.0f");
		ImGui::SliderFloat("Y ", &CreateSpritePivot.y, 0, ImClamp(CreateSpriteSize.y, 0.0f, CreateSpriteSize.y - 1.0f), "%.0f");
		ImGui::Dummy(ImVec2(0.0f, TextHeight * 1.0f));

		std::string CreateSpriteColorModeName = FSprite::ColotModeToString(CreateSpriteColorMode);
		ImGui::Text(Utils::Format("Color Mode : %s", CreateSpriteColorModeName.c_str()).c_str());
		ImGui::Separator();

		const ImVec4 BackgroundColor(0.0f, 0.0f, 0.0f, 1.0f);
		const ImVec4 TintColor(1.0f, 1.0f, 1.0f, 0.5f);
		const ImVec4 SelectedColor(1.0f, 1.0f, 1.0f, 1.0f);
		const ImVec4 TextColor(0.0f, 0.0f, 0.0f, 1.0f);
		const ImVec2 ButtonSize(64.0, 64.0f);

		if (ButtonLambda("RGBA", ImageRGBA->GetShaderResourceView(), ImageRGBA->Size, ButtonSize, CreateSpriteColorMode == EColorMode::RGB, BackgroundColor, TintColor, SelectedColor, TextColor))
		{
			if (CreateSpriteColorMode == EColorMode::RGB)
			{
				CreateSpriteColorMode = EColorMode::Unknow;
			}
			else
			{
				CreateSpriteColorMode = EColorMode::RGB;
			}
		}
		ImGui::SameLine();
		if (ButtonLambda("INDEXED", ImageIndexed->GetShaderResourceView(), ImageIndexed->Size, ButtonSize, CreateSpriteColorMode == EColorMode::Indexed, BackgroundColor, TintColor, SelectedColor, TextColor))
		{
			if (CreateSpriteColorMode == EColorMode::Indexed)
			{
				CreateSpriteColorMode = EColorMode::Unknow;
			}
			else
			{
				CreateSpriteColorMode = EColorMode::Indexed;
			}
		}
		ImGui::SameLine();
		if (ButtonLambda("ZX", ImageZX->GetShaderResourceView(), ImageZX->Size, ButtonSize, CreateSpriteColorMode == EColorMode::ZX, BackgroundColor, TintColor, SelectedColor, TextColor))
		{
			if (CreateSpriteColorMode == EColorMode::ZX)
			{
				CreateSpriteColorMode = EColorMode::Unknow;
			}
			else
			{
				CreateSpriteColorMode = EColorMode::ZX;
			}
		}

		ImGui::Dummy(ImVec2(0.0f, TextHeight * 1.0f));

		if (ImGui::ButtonEx("OK", ImVec2(TextWidth * 11.0f, TextHeight * 1.5f)))
		{
			FSprite NewSprite;
			NewSprite.NumFrame = 1;
			NewSprite.Size = CreateSpriteSize;
			NewSprite.Pivot = CreateSpritePivot;
			NewSprite.ColorMode = CreateSpriteColorMode;
			NewSprite.Name = CreateSpriteNameBuffer;

			if (AddSprite(NewSprite))
			{
				ImGui::CloseCurrentPopup();
				bCreateSpriteFirstOpen = false;
			}
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(TextWidth * 11.0f, TextHeight * 1.5f)))
		{
			ImGui::CloseCurrentPopup();
			bCreateSpriteFirstOpen = false;
		}
		ImGui::EndPopup();
	}
	return bVisible;
}

bool SViewer::AddSprite(FSprite& NewSprite)
{
	if (!NewSprite.IsValid())
	{
		return false;
	}

	FSpriteLayer NewLayer;
	NewLayer.bVisible = true;
	NewLayer.bLock = false;
	NewLayer.Name = Utils::Format("Layer %i", ++LayersCounter);
	NewSprite.Layers.push_back(NewLayer);

	Sprites.push_back(NewSprite);

	if (CurrentSprite < 0)
	{
		CurrentSprite++;
	}

	return true;
}
