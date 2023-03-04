#pragma once

#define INDEX_NONE (-1)

#define WIN32_LEAN_AND_MEAN
#define IMGUI_DEFINE_MATH_OPERATORS
#pragma warning(disable : 4996)				//_CRT_SECURE_NO_WARNINGS

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
#define IM_VEC2_CLASS_EXTRA
inline bool operator==(const ImVec2& lhs, const ImVec2& rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; } \
inline bool operator!=(const ImVec2& lhs, const ImVec2& rhs) { return lhs.x != rhs.x || lhs.y != rhs.y; }
#include "imgui_internal.h"

#include "Log.h"
#include "Char.h"
#include "Math_.h"
#include "Window.h"
#include "Delegates.h"
#include "Transform.h"

struct FImage;

struct FFrameworkFlags
{
	bool bLog = false;
};

enum class EDialogMode;

#include "..\ZX-Convert\Resource.h"
