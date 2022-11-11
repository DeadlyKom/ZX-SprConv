#include "ImageList.h"
#include "Core\Utils.h"

void SImageList::Initialize()
{
	bIncludeInWindows = true;
	Name = "Image List";
	FileSelectIndex = 0;
}

void SImageList::Render()
{
	if (!IsOpen())
	{
		Close();
		return;
	}

	ImGui::Begin("Image List", &bOpen, ImGuiWindowFlags_MenuBar);

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Files"))
		{
			ShowMenuFiles();
		}
		ImGui::EndMenuBar();
	}

	ImVec2 FilesSize;
	{
		ImGuiStyle& Style = ImGui::GetStyle();
		ImVec2 Size = ImGui::GetWindowSize();

		float HeightItem = ImGui::CalcTextSize("").y;
		float HeightButton = HeightItem * 5.0f;

		FilesSize.x = Size.x * 1.0f - Style.WindowPadding.x * 2;
		FilesSize.y = Size.y * 1.0f - Style.WindowPadding.y * 2;
	}

	ImGui::BeginChild("Files", FilesSize);
	ImGui::Columns(3);

	if (IsWindowAppearing())
	{
		ImGui::SetColumnWidth(0, FilesSize.x * 0.7f);
		ImGui::SetColumnWidth(1, FilesSize.x * 0.13f);
	}

	ImGui::Separator();	
	if (ImGui::Selectable("File Name"))
	{
	}
	ImGui::NextColumn();
	if (ImGui::Selectable("Type"))
	{
	}
	ImGui::NextColumn();
	if (ImGui::Selectable("Size"))
	{
	}
	ImGui::NextColumn();
	ImGui::Separator();

	for (int32_t i = 0; i < Files.size(); ++i)
	{
		if (ImGui::Selectable(Files[i].path().filename().string().c_str(), i == FileSelectIndex, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(ImGui::GetWindowContentRegionWidth(), 0)))
		{
			FileSelectIndex = i;
			OnSelectedImage.Broadcast(Files[i]);
		}
		ImGui::NextColumn();
		ImGui::TextUnformatted(Files[i].path().extension().string().c_str());
		ImGui::NextColumn();
		ImGui::TextUnformatted(std::to_string(Files[i].file_size()).c_str());
		ImGui::NextColumn();
	}

	ImGui::EndChild();
	ImGui::End();

	SWindow::Render();
}

void SImageList::ShowMenuFiles()
{
	if (ImGui::MenuItem("Open", NULL) && !FileDialogHandle.IsValid())
	{
		
		const std::string OldPath = Files.empty() ? "" : Files.back().path().parent_path().string();
		FileDialogHandle = Utils::OpenWindowFileDialog("Select File", EDialogMode::Select, [this](std::filesystem::path FilePath) -> void
		{
			std::filesystem::directory_entry File(FilePath);
			if(File.exists())
			{
				Files.push_back(File);
				OnSelectedImage.Broadcast(Files.back());
			}
			else
			{
				ImGui::LogText("Error");
			}
			Utils::CloseWindowFileDialog(FileDialogHandle);
		}, OldPath, "*.*, *.png, *.scr");
	}
	ImGui::EndMenu();
}
