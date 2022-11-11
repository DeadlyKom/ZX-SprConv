#pragma once

#define WIN32_LEAN_AND_MEAN

#include <map>
#include <string>
#include <stdint.h>
#include <vector>
#include <windows.h>
#include <memory>

#include "imgui.h"
#include "Core\Window.h"

enum class EWindowsType
{
	ImageList = 0,
	Sprite,
	Palette,
};

class SViewer : public SWindow
{
public:
	SViewer();

	std::shared_ptr<SWindow> GetWindow(EWindowsType Type);

	virtual void Initialize();
	virtual void Shutdown();
	virtual void Render();

private:
	std::map<EWindowsType, std::shared_ptr<SWindow>> Windows;
};
