#pragma once

#include <vector>
#include <filesystem>
#include "Core\Window.h"
#include "core\Delegates.h"

class SImageList : public SWindow
{
public:
	virtual void Initialize() override;
	virtual void Render() override;

private:
	void ShowMenuFiles();

	DelegateHandle FileDialogHandle;
	std::vector<std::filesystem::directory_entry> Files;
};
