#pragma once

#include <string>
#include <functional>
#include <filesystem>
#include "Core\Window.h"

class SFileDialog : public SWindow
{
public:
	static SFileDialog& Get();
	static uint32_t OpenWindow(std::function<void()>& OnCallback, const std::string& Path);
	static void CloseWindow(uint32_t Handle);

	virtual void Initialize() override;
	virtual void Render() override;
	virtual void Open() override;

private:
	void ReadDirectory(const std::string& Path);

	std::function<void()> CloseCallback;
	std::string CurrentPath;
	std::vector<std::filesystem::directory_entry> Files;
	std::vector<std::filesystem::directory_entry> Folders;
};
