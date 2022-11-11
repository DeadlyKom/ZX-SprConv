#pragma once

#include <string>
#include <functional>
#include <filesystem>
#include "Core\Delegates.h"
#include "Core\Window.h"
#include "imgui.h"

enum class EFileSortOrder
{
	Unknown,
	Up,
	Down,
};

enum class EDialogMode
{
	Select,
	Open,
	Save
};

enum class EDialogStage
{
	Unknown,
	Close,
	Selected,
};

class SFileDialog : public SWindow
{
public:
	static SFileDialog& Get();
	static DelegateHandle OpenWindow(std::string& FileDialogName, EDialogMode Mode, std::function<void(std::filesystem::path)>& OnCallback, const std::filesystem::path& Path, const std::string& FilterTypes);
	static void CloseWindow(DelegateHandle& Handle);

	virtual void Initialize() override;
	virtual void Render() override;
	virtual void Open() override;
	virtual void Close() override;

private:
	void SetFilterTypes(const std::string& InFilterTypes);
	void Release();

	void ReadDirectory(const std::string& Path);
	void ShowDirectory(const ImVec2& Size);
	void ShowFiles(const ImVec2& Size);
	void ShowFilterDropBox();

	void ApplyFilterTypes();

	EFileSortOrder FlipFlopOrder(EFileSortOrder CurrentOrder)
	{
		NameSortOrder = SizeSortOrder = TypeSortOrder = DateSortOrder = EFileSortOrder::Unknown;
		return CurrentOrder == EFileSortOrder::Down ? EFileSortOrder::Up : EFileSortOrder::Down;
	}

	ImFont* Font;

	bool bAllFiles;

	EDialogMode Mode;
	EDialogStage Stage;

	EFileSortOrder NameSortOrder;
	EFileSortOrder SizeSortOrder;
	EFileSortOrder TypeSortOrder;
	EFileSortOrder DateSortOrder;

	int32_t FileSelectIndex;
	int32_t FolderSelectIndex;
	int32_t SelectedFilterIndex;

	std::string FileDialogName;
	std::string CurrentFile;
	std::string CurrentFolder;
	std::string CurrentPath;

	std::function<void(std::filesystem::path)> CloseCallback;
	std::vector<std::string> FilterTypes;
	std::vector<std::filesystem::directory_entry> Files;
	std::vector<std::filesystem::directory_entry> Folders;
};
