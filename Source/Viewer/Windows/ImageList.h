#pragma once

#include <vector>
#include <filesystem>
#include "Core\Window.h"

class SImageList : public SWindow
{
public:
	virtual void Initialize() override;
	virtual void Render() override;

private:
	void ShowMenuFiles();

	uint32_t FileDialogHandle;
	std::vector<std::filesystem::directory_entry> Files;
};
