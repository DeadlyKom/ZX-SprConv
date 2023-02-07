#pragma once

#ifndef IMGUI_DISABLE_LOG
#define LOG(...)						{ ImGui::LogText("Log: "); ImGui::LogText(__VA_ARGS__); ImGui::LogText(IM_NEWLINE); }
#define LOG_DISPLAY(...)				{ ImGui::LogText("Display: "); ImGui::LogText(__VA_ARGS__); ImGui::LogText(IM_NEWLINE); }
#define LOG_WARNING(...)				{ ImGui::LogText("Warning: "); ImGui::LogText(__VA_ARGS__); ImGui::LogText(IM_NEWLINE); }
#define LOG_ERROR(...)					{ ImGui::LogText("Error: "); ImGui::LogText(__VA_ARGS__); ImGui::LogText(IM_NEWLINE); }
#else
#define LOG(...)
#define LOG_DISPLAY(...)
#define LOG_WARNING(...)
#define LOG_ERROR(...)	
#endif
