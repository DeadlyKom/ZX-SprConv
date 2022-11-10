#pragma once

#define WIN32_LEAN_AND_MEAN

#include <string>
#include <stdint.h>
#include <vector>
#include <windows.h>
#include <memory>

#include "imgui.h"

using namespace std;

class SWindow;

class SViewer
{
public:
	SViewer();

	virtual void Initialize();
	virtual void Shutdown();
	virtual void Render();

private:
	vector<shared_ptr<SWindow>> Windows;
};
