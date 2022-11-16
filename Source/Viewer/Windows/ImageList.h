#pragma once

#include <CoreMinimal.h>

class SImageList : public SWindow
{
	DECLARE_MULTICAST_DELEGATE(FSelectedImageDelegate, const std::filesystem::directory_entry& /*Path*/);
public:
	virtual void Initialize() override;
	virtual void Render() override;

	FSelectedImageDelegate OnSelectedImage;

private:
	void ShowMenuFiles();

	int32_t FileSelectIndex;
	DelegateHandle FileDialogHandle;
	std::vector<std::filesystem::directory_entry> Files;
};
