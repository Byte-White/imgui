#pragma once
#include "imgui.h"

#ifndef IMGUI_DISABLE

IMGUI_IMPL_API bool     ImGui_ImplRaylib_Init();
IMGUI_IMPL_API void     ImGui_ImplRaylib_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplRaylib_NewFrame();
IMGUI_IMPL_API void     ImGui_ImplRaylib_RenderDrawData(ImDrawData* draw_data);
IMGUI_IMPL_API bool     ImGui_ImplRaylib_ProcessEvent();

#endif // #ifndef IMGUI_DISABLE
