#pragma once

#define INDEX_NONE (-1)

#define WIN32_LEAN_AND_MEAN
#define IMGUI_DEFINE_MATH_OPERATORS

#include <map>
#include <vector>
#include <memory>
#include <string>
#include <functional>
#include <filesystem>

#include <d3d11.h>
#include <d3dcompiler.h>

#include <stdint.h>
#include <windows.h>

#include "imgui.h"
#include "imgui_internal.h"

#include "Window.h"
#include "Delegates.h"
#include "Math_.h"
#include "Transform.h"

struct FImage;

enum class EDialogMode;
