#include "FileDialog.h"
#include "Core\AppFramework.h"

void SFileDialog::Initialize()
{
	CurrentPath = std::filesystem::current_path().string();
	ReadDirectory(CurrentPath);
}

void SFileDialog::Render()
{
	if (!IsOpen())
	{
		Close();
		return;
	}

	ImGui::Begin("Open", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking);
	ImGui::Text("%s", CurrentPath.c_str());
	ImGui::BeginChild("Directories", ImVec2(200, 300), true, ImGuiWindowFlags_HorizontalScrollbar);
	if (ImGui::Selectable("..", false, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(ImGui::GetContentRegionAvail().x, 0)))
	{
		if (ImGui::IsMouseDoubleClicked(0))
		{
			CurrentPath = std::filesystem::path(CurrentPath).parent_path().string();
		}
	}

	std::string file_dialog_current_file = "";
	std::string file_dialog_current_folder = "";
	int32_t file_dialog_file_select_index = 0;
	int32_t file_dialog_folder_select_index = 0;

	for (int i = 0; i < Folders.size(); ++i)
	{
		if (ImGui::Selectable(Folders[i].path().stem().string().c_str(), i == file_dialog_folder_select_index, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(ImGui::GetWindowContentRegionWidth(), 0)))
		{
			file_dialog_current_file = "";
			if (ImGui::IsMouseDoubleClicked(0))
			{
				CurrentPath = Folders[i].path().string();
				file_dialog_folder_select_index = 0;
				file_dialog_file_select_index = 0;
				ImGui::SetScrollHereY(0.0f);
				file_dialog_current_folder = "";
			}
			else
			{
				file_dialog_folder_select_index = i;
				file_dialog_current_folder = Folders[i].path().stem().string();
			}
		}
	}
	ImGui::EndChild();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 120);

	if (ImGui::Button("Cancel"))
	{
		Close();
		CloseCallback();
	}

	ImGui::SameLine();
	if (ImGui::Button("Select"))
	{
		Close();
		CloseCallback();
	}

	ImGui::End();
}

SFileDialog& SFileDialog::Get()
{
	static shared_ptr<SFileDialog> Instance(new SFileDialog());
	return *Instance.get();
}

void SFileDialog::Open()
{
	SWindow::Open();
	Initialize();
	Render();
}

uint32_t SFileDialog::OpenWindow(std::function<void()>& OnCallback, const std::string& Path)
{
	SFileDialog::Get().CloseCallback = OnCallback;
	return FAppFramework::Get().BindRender(bind(&SFileDialog::Render, SFileDialog::Get()));
}

void SFileDialog::CloseWindow(uint32_t Handle)
{
	FAppFramework::Get().UnbindRender(Handle);
}

void SFileDialog::ReadDirectory(const std::string& Path)
{
	for (auto& EntryIt : std::filesystem::directory_iterator(Path))
	{
		if (EntryIt.is_directory())
		{
			Folders.push_back(EntryIt);
		}
		else
		{
			Files.push_back(EntryIt);
		}
	}
}
