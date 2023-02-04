#include "ImageList.h"
#include "Viewer\Viewer.h"
#include "Core\Utils.h"

void SImageList::Initialize()
{
	bIncludeInWindows = true;
	Name = "Image List";
	FileSelectIndex = -1;
}

void SImageList::Render()
{
	if (!IsOpen())
	{
		Close();
		return;
	}

	ImGui::Begin("Image List", &bOpen);

	//ImVec2 FilesSize;
	//{
	//	ImGuiStyle& Style = ImGui::GetStyle();
	//	ImVec2 Size = ImGui::GetWindowSize();
	//
	//	float HeightItem = ImGui::CalcTextSize("").y;
	//	float HeightButton = HeightItem * 5.0f;
	//
	//	FilesSize.x = Size.x * 1.0f - Style.WindowPadding.x * 2;
	//	FilesSize.y = Size.y * 1.0f - Style.WindowPadding.y * 2;
	//}
	//
	//ImGui::BeginChild("Files", FilesSize);
	//ImGui::Columns(3);
	//
	//if (IsWindowAppearing())
	//{
	//	ImGui::SetColumnWidth(0, FilesSize.x * 0.7f);
	//	ImGui::SetColumnWidth(1, FilesSize.x * 0.13f);
	//}
	//
	//ImGui::Separator();	
	//if (ImGui::Selectable("File Name"))
	//{
	//}
	//ImGui::NextColumn();
	//if (ImGui::Selectable("Type"))
	//{
	//}
	//ImGui::NextColumn();
	//if (ImGui::Selectable("Size"))
	//{
	//}
	//ImGui::NextColumn();
	//ImGui::Separator();
	//
	//std::vector<std::filesystem::directory_entry>& Files = GetParent()->Files;
	//for (int32_t i = 0; i < Files.size(); ++i)
	//{
	//	if (ImGui::Selectable(Files[i].path().filename().string().c_str(), i == FileSelectIndex, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(ImGui::GetWindowContentRegionWidth(), 0)))
	//	{
	//		FileSelectIndex = i;
	//		OnSelectedImage.Broadcast(Files[i]);
	//	}
	//	ImGui::NextColumn();
	//	ImGui::TextUnformatted(Files[i].path().extension().string().c_str());
	//	ImGui::NextColumn();
	//	ImGui::TextUnformatted(std::to_string(Files[i].file_size()).c_str());
	//	ImGui::NextColumn();
	//}
	//
	//ImGui::EndChild();

	if (ImGui::BeginChild("ImageListPanel"))
	{
		HandleKeyboardInputs();
		ImGui::PushAllowKeyboardFocus(true);

		const std::vector<std::filesystem::directory_entry>& Files = GetParent()->Files;
		for (int32_t Index = 0; Index < Files.size(); ++Index)
		{
			if (ImGui::Selectable(Files[Index].path().stem().string().c_str(), Index == FileSelectIndex, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(ImGui::GetWindowContentRegionWidth(), 0)))
			{
				FileSelectIndex = Index;
				OnSelectedImage.Broadcast(Files[Index]);
			}

			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				uint32_t Width, Height;
				FImage::GetImageInfo(Files[Index].path().string(), Width, Height);

				const std::string ImageMetadataText = Utils::Format("%ix%i", Width, Height);
				ImGui::TextUnformatted(ImageMetadataText.c_str());

				//ImGui::TextUnformatted(Files[Index].path().extension().string().c_str());
				//ImGui::TextUnformatted(std::to_string(Files[Index].file_size()).c_str());
				ImGui::EndTooltip();
			}
		}
		ImGui::PopAllowKeyboardFocus();
		ImGui::EndChild();
	}
	ImGui::End();
}

void SImageList::HandleKeyboardInputs()
{
	ImGuiIO& IO = ImGui::GetIO();
	const bool Shift = IO.KeyShift;
	const bool Ctrl = IO.ConfigMacOSXBehaviors ? IO.KeySuper : IO.KeyCtrl;
	const bool Alt = IO.ConfigMacOSXBehaviors ? IO.KeyCtrl : IO.KeyAlt;

	if (!ImGui::IsWindowFocused())
	{
		return;
	}

	if (!Ctrl && !Shift && !Alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete)))
	{
		if (FileSelectIndex >= 0)
		{
			DeleteFile(FileSelectIndex);
		}
	}
}

void SImageList::DeleteFile(int32_t& RemoveIndex)
{
	int32_t Index = 0;
	std::vector<std::filesystem::directory_entry>& Files = GetParent()->Files;
	for (std::vector<std::filesystem::directory_entry>::iterator FileIt = Files.begin(); FileIt != Files.end(); ++FileIt, ++Index)
	{
		if (RemoveIndex == Index)
		{
			Files.erase(FileIt);
			break;
		}
	}

	if (Files.size() <= RemoveIndex)
	{
		--RemoveIndex;
	}
	else
	{
		RemoveIndex = -1;
	}
}
