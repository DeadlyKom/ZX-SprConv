#pragma once

#include <filesystem>
#include "Core\Window.h"

struct FImage;

class SSprite : public SWindow
{
public:
	virtual void Initialize() override;
	virtual void Render() override;

	void OnSelectedFileImage(const std::filesystem::directory_entry& Path);

private:
	std::shared_ptr<FImage> Image;
};
