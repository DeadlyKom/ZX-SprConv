#include <map>
#include <ctime>
#include <sstream>
#include "FileDialog.h"
#include "Core\AppFramework.h"
#include "Fonts\Fonts.h"

#pragma warning(disable : 4996)				//_CRT_SECURE_NO_WARNINGS

namespace FilterType
{
	const std::string ALL_FILES = "*.*";
	const std::string PNG_FILES = "*.png";
	const std::string SCR_FILES = "*.scr";
}

namespace
{
	std::map<std::string, std::string> KnownFilterTypes = 
	{ {FilterType::ALL_FILES, "All Files (*.*)"},
	  {FilterType::PNG_FILES, "PNG Files"},
	  {FilterType::SCR_FILES, "SCR Files (speccy screen)"} };
}

void SFileDialog::Initialize()
{
	CurrentPath = CurrentPath.empty() ? std::filesystem::current_path().string() : CurrentPath;
	ReadDirectory(CurrentPath);

	CurrentFile = "";
	CurrentFolder = "";
	FileSelectIndex = 0;
	FolderSelectIndex = 0;

	Font = FFonts::Get().GetFont(0);
	Stage = EDialogStage::Unknown;
}

void SFileDialog::Render()
{
	if (!IsOpen())
	{
		Close();
		return;
	}

	if (IsWindowAppearing())
	{
		ImGui::SetNextWindowSize(ImVec2(850.0f, 420.0f));
	}

	ImGui::PushFont(Font);

	ImGui::Begin(FileDialogName.c_str(), &bOpen, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse);
	
	ImVec2 DirectorySize;
	ImVec2 FilesSize;
	float LeftButton;

	{
		ImGuiStyle& Style = ImGui::GetStyle();
		ImVec2 Size = ImGui::GetWindowSize();

		float HeightItem = ImGui::CalcTextSize("").y;
		float HeightButton = HeightItem * 5.0f;

		DirectorySize.x = Size.x * 0.3f;
		DirectorySize.y = Size.y * 1.0f - Style.WindowPadding.y * 3 - HeightButton - HeightItem;

		FilesSize.x = Size.x * 0.7f - Style.WindowPadding.x * 3;
		FilesSize.y = Size.y * 1.0f - Style.WindowPadding.y * 3 - HeightButton - HeightItem;

		LeftButton = Size.x - Style.WindowPadding.x * 5.0f - 100.0f - ImGui::CalcTextSize("Select").x - ImGui::CalcTextSize("Cancel").x;
	}

	ImGui::Text("%s", CurrentPath.c_str());
	ShowDirectory(DirectorySize);
	ImGui::SameLine();
	ShowFiles(FilesSize);

	ImGui::SetCursorPosX(LeftButton);

	ShowFilterDropBox();

	ImGui::SameLine();
	
	if (!IsOpen())
	{
		Stage = EDialogStage::Close;
	}

	ImGui::BeginDisabled(CurrentFile.empty());
	if (ImGui::Button("Select"))
	{
		Stage = EDialogStage::Close;
	}
	ImGui::EndDisabled();

	ImGui::SameLine();
	if (ImGui::Button("Cancel"))
	{
		Stage = EDialogStage::Close;
	}

	ImGui::End();

	ImGui::PopFont();

	SWindow::Render();

	switch (Stage)
	{
	case EDialogStage::Unknown:
		break;

	case EDialogStage::Close:
		Close();
		CloseCallback("");
		break;

	case EDialogStage::Selected:
		Close();
		std::filesystem::path Path(CurrentPath);
		CloseCallback(Path / CurrentFile);
		break;
	}
}

SFileDialog& SFileDialog::Get()
{
	static std::shared_ptr<SFileDialog> Instance(new SFileDialog());
	return *Instance.get();
}

void SFileDialog::Open()
{
	SWindow::Open();
	Initialize();
}

void SFileDialog::Close()
{
	SWindow::Close();
	Release();
}

DelegateHandle SFileDialog::OpenWindow(std::string& FileDialogName, EDialogMode Mode, std::function<void(std::filesystem::path)>& OnCallback, const std::filesystem::path& Path, const std::string& FilterTypes)
{
	SFileDialog& FileDialog = SFileDialog::Get();
	FileDialog.FileDialogName = FileDialogName;
	FileDialog.Mode = Mode;
	FileDialog.CloseCallback = OnCallback;
	FileDialog.CurrentPath = Path.string();
	FileDialog.SetFilterTypes(FilterTypes);
	FileDialog.Open();
	return FAppFramework::Get().OnRender.AddRaw(&FileDialog, &SFileDialog::Render);
}

void SFileDialog::CloseWindow(DelegateHandle& Handle)
{
	FAppFramework::Get().OnRender.Remove(Handle);
}

void SFileDialog::SetFilterTypes(const std::string& InFilterTypes)
{
	SelectedFilterIndex = 0;
	FilterTypes.clear();

	if (InFilterTypes.empty())
	{
		bAllFiles = true;
		FilterTypes.push_back(FilterType::ALL_FILES);
	}
	else
	{
		bAllFiles = false;

		std::string FilterTypesLower(InFilterTypes);
		std::transform(FilterTypesLower.begin(), FilterTypesLower.end(), FilterTypesLower.begin(),
		[](uint8_t c) -> uint8_t
		{
			return std::tolower(c);
		});

		// separate filters
		std::string FilterType;
		std::istringstream FilterTypesStream(FilterTypesLower);
		while (std::getline(FilterTypesStream, FilterType, ','))
		{
			if (FilterType.empty())
			{
				continue;
			}

			FilterType.erase(remove(FilterType.begin(), FilterType.end(), ' '), FilterType.end());
			FilterTypes.push_back(FilterType);
		}
	}
}

void SFileDialog::Release()
{
	Files.clear();
	Folders.clear();
}

void SFileDialog::ReadDirectory(const std::string& Path)
{
	Files.clear();
	Folders.clear();

	try
	{
		std::error_code ErrorCode;
		for (std::filesystem::directory_entry FileIt : std::filesystem::directory_iterator(Path, std::filesystem::directory_options::skip_permission_denied, ErrorCode))
		{
			if (ErrorCode)
			{
				LOG_ERROR("Can't open file : %s", FileIt.path().string().c_str());
				continue;
			}

			if (FileIt.is_directory())
			{
				Folders.push_back(FileIt);
			}
			else
			{
				Files.push_back(FileIt);
			}
		}	
	}
	catch (const std::exception& Exception)
	{
		LOG_ERROR(Exception.what());
	}

	ApplyFilterTypes();
}

void SFileDialog::ShowDirectory(const ImVec2& Size)
{
	ImGui::BeginChild("Directories", Size, true, ImGuiWindowFlags_HorizontalScrollbar);
	if (ImGui::Selectable("..", false, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(ImGui::GetContentRegionAvail().x, 0)))
	{
		if (ImGui::IsMouseDoubleClicked(0))
		{
			CurrentFile = "";
			CurrentPath = std::filesystem::path(CurrentPath).parent_path().string();
			ReadDirectory(CurrentPath);
		}
	}

	for (int i = 0; i < Folders.size(); ++i)
	{
		std::wstring WFolderName = Folders[i].path().stem().wstring();
		std::string FolderName = Utils::Utf16ToUtf8(WFolderName);
		if (ImGui::Selectable(FolderName.c_str(), i == FolderSelectIndex, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(ImGui::GetWindowContentRegionWidth(), 0)))
		{
			CurrentFolder = Folders[i].path().stem().string();
			if (ImGui::IsMouseDoubleClicked(0))
			{
				CurrentFile = "";
				CurrentPath = Folders[i].path().string();
				FolderSelectIndex = 0;
				FileSelectIndex = 0;
				ImGui::SetScrollHereY(0.0f);
			}
			else
			{
				FolderSelectIndex = i;
			}
			ReadDirectory(CurrentPath);
		}
	}
	ImGui::EndChild();
}

void SFileDialog::ShowFiles(const ImVec2& Size)
{
	ImGui::BeginChild("Files", Size, true, ImGuiWindowFlags_HorizontalScrollbar);
	ImGui::Columns(4);

	if (IsWindowAppearing())
	{
		ImGui::SetColumnWidth(0, Size.x * 0.5f);
		ImGui::SetColumnWidth(1, Size.x * 0.1f);
		ImGui::SetColumnWidth(2, Size.x * 0.13f);
	}

	if (ImGui::Selectable("File"))
	{
		NameSortOrder = FlipFlopOrder(NameSortOrder);
		std::sort(Files.begin(), Files.end(), [=](const std::filesystem::directory_entry& A, const std::filesystem::directory_entry& B) -> bool
		{
			const std::string& NameA = A.path().filename().string();
			const std::string& NameB = B.path().filename().string();
			return NameSortOrder == EFileSortOrder::Down ? NameA > NameB : NameA < NameB;
		});
	}
	ImGui::NextColumn();
	if (ImGui::Selectable("Size"))
	{
		SizeSortOrder = FlipFlopOrder(SizeSortOrder);
		std::sort(Files.begin(), Files.end(), [=](const std::filesystem::directory_entry& A, const std::filesystem::directory_entry& B) -> bool
		{
			const size_t& NameA = A.file_size();
			const size_t& NameB = B.file_size();
			return SizeSortOrder == EFileSortOrder::Down ? NameA > NameB : NameA < NameB;
		});
	}
	ImGui::NextColumn();
	if (ImGui::Selectable("Type"))
	{
		TypeSortOrder = FlipFlopOrder(TypeSortOrder);
		std::sort(Files.begin(), Files.end(), [=](const std::filesystem::directory_entry& A, const std::filesystem::directory_entry& B) -> bool
		{
			const std::string& NameA = A.path().extension().string();
			const std::string& NameB = B.path().extension().string();
			return TypeSortOrder == EFileSortOrder::Down ? NameA > NameB : NameA < NameB;
		});
	}
	ImGui::NextColumn();
	if (ImGui::Selectable("Date"))
	{
		DateSortOrder = FlipFlopOrder(DateSortOrder);
		std::sort(Files.begin(), Files.end(), [=](const std::filesystem::directory_entry& A, const std::filesystem::directory_entry& B) -> bool
		{
			const std::filesystem::file_time_type& NameA = A.last_write_time();
			const std::filesystem::file_time_type& NameB = B.last_write_time();
			return DateSortOrder == EFileSortOrder::Down ? NameA > NameB : NameA < NameB;
		});
	}
	ImGui::NextColumn();
	ImGui::Separator();

	for (int32_t i = 0; i < Files.size(); ++i)
	{
		if (ImGui::Selectable(Files[i].path().filename().string().c_str(), i == FileSelectIndex, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(ImGui::GetWindowContentRegionWidth(), 0)))
		{
			FileSelectIndex = i;
			CurrentFile = Files[i].path().filename().string();

			if (ImGui::IsMouseDoubleClicked(0))
			{
				Stage = EDialogStage::Selected;
			}
		}
		ImGui::NextColumn();
		ImGui::TextUnformatted(std::to_string(Files[i].file_size()).c_str());
		ImGui::NextColumn();
		ImGui::TextUnformatted(Files[i].path().extension().string().c_str());
		ImGui::NextColumn();
		std::filesystem::file_time_type Time = Files[i].last_write_time();
		auto ST = std::chrono::time_point_cast<std::chrono::system_clock::duration>(Time - decltype(Time)::clock::now() + std::chrono::system_clock::now());
		std::time_t Clock = std::chrono::system_clock::to_time_t(ST);
		std::tm* TimeInfo = std::localtime(&Clock);
		std::string DateName = Utils::Format("%d/%d/%d %.2d:%.2d:%.2d", TimeInfo->tm_mday, TimeInfo->tm_mon+1, TimeInfo->tm_year+1900, TimeInfo->tm_hour, TimeInfo->tm_min, TimeInfo->tm_sec);
		ImGui::TextUnformatted(DateName.c_str());
		ImGui::NextColumn();
	}

	ImGui::EndChild();
}

void SFileDialog::ShowFilterDropBox()
{
	ImGui::PushItemWidth(100.0f);
	if (ImGui::BeginCombo("##FilterTypes", FilterTypes[SelectedFilterIndex].c_str()))
	{
		for (std::vector<std::string>::size_type Index = 0; Index < FilterTypes.size(); ++Index)
		{
			const std::map<std::string, std::string>::iterator& SearchIt = KnownFilterTypes.find(FilterTypes[Index]);
			const std::string LabelText = SearchIt != KnownFilterTypes.end() ? SearchIt->second : FilterTypes[Index];
			if (ImGui::Selectable(LabelText.c_str(), SelectedFilterIndex == Index))
			{
				SelectedFilterIndex = int32_t(Index);
				ReadDirectory(CurrentPath);
			}
		}
		ImGui::EndCombo();
	}
	ImGui::PopItemWidth();
}

void SFileDialog::ApplyFilterTypes()
{
	if (bAllFiles)
	{
		return;
	}

	// filtration files
	const std::string& Filter = FilterTypes[SelectedFilterIndex];
	{
		const size_t DotPosition = Filter.find_last_of('.');
		if (DotPosition == std::string::npos)
		{
			return;
		}
		const std::string LeftFilter = Filter.substr(0, DotPosition);
		const std::string RightFilter = Filter.substr(DotPosition + 1);
		if (LeftFilter.empty() || RightFilter.empty())
		{
			return;
		}
		std::vector<std::filesystem::directory_entry>::iterator FileIt = Files.begin();

		for (; FileIt != Files.end();)
		{
			std::string Name = (*FileIt).path().filename().string();
			const size_t DotPosition = Name.find_last_of('.');
			if (DotPosition == std::string::npos)
			{
				++FileIt;
				continue;
			}

			bool bRemove = false;
			const std::string Filename = Name.substr(0, DotPosition);
			const std::string FileExt = Name.substr(DotPosition + 1);			
			if (!Filename.empty() && LeftFilter.compare("*") != 0)
			{

			}
			else if (!FileExt.empty() && RightFilter.compare("*") != 0)
			{
				if (FileExt.find(RightFilter) == std::string::npos)
				{
					bRemove = true;
				}
			}

			if (bRemove)
			{
				FileIt = Files.erase(FileIt);
			}
			else
			{
				++FileIt;
			}
		}
	}
}
