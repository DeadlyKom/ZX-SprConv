#include "ImageList.h"
#include "Core\Utils.h"

void SImageList::Initialize()
{
	bIncludeInWindows = true;
	Name = "Image List";
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

	ImGui::BeginChild("Files");
	ImGui::Columns(4);
	ImGui::Separator();
	////static float initial_spacing_column_0 = 230.0f;
	////if (initial_spacing_column_0 > 0) {
	////	ImGui::SetColumnWidth(0, initial_spacing_column_0);
	////	initial_spacing_column_0 = 0.0f;
	////}
	////static float initial_spacing_column_1 = 80.0f;
	////if (initial_spacing_column_1 > 0) {
	////	ImGui::SetColumnWidth(1, initial_spacing_column_1);
	////	initial_spacing_column_1 = 0.0f;
	////}
	////static float initial_spacing_column_2 = 80.0f;
	////if (initial_spacing_column_2 > 0) {
	////	ImGui::SetColumnWidth(2, initial_spacing_column_2);
	////	initial_spacing_column_2 = 0.0f;
	////}
	if (ImGui::Selectable("File Name")) {
	}
	ImGui::NextColumn();
	if (ImGui::Selectable("Size")) {
	}
	ImGui::NextColumn();
	if (ImGui::Selectable("Type")) {
	}
	ImGui::NextColumn();
	if (ImGui::Selectable("Date")) {
	}
	ImGui::Separator();
	ImGui::EndChild();
	ImGui::End();
}

void SImageList::ShowMenuFiles()
{
	if (ImGui::MenuItem("Open", NULL) && !FileDialogHandle.IsValid())
	{
		FileDialogHandle = Utils::OpenWindowFileDialog("Select File", EDialogMode::Select, [this](std::string FilePath) -> void
		{
			Utils::CloseWindowFileDialog(FileDialogHandle);
		}, "", "*.*, *.png, *.scr");
	}
	ImGui::EndMenu();
}
