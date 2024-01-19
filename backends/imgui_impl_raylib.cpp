// dear imgui: raylib backend

#include "imgui_impl_raylib.h"
#include <raylib.h>
#include <rlgl.h>
#include <memory>
#include <iostream>

static double g_Time = 0.0;
static bool g_UnloadAtlas = false;
static int g_AtlasTexID = 0;


// load the default font atlas
static void ImGui_ImplRaylib_CreateFontsTexture()
{
    if (!g_UnloadAtlas) {
        ImGuiIO& io = ImGui::GetIO();
        unsigned char* pixels = NULL;
        int width, height, bpp;
        Image image;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bpp);
        std::cout << "width: " << width << ", height: " << height << ", bpp: " << bpp<<std::endl;
        unsigned int size = GetPixelDataSize(width, height, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
        image.data = malloc(size);
        memcpy(image.data, pixels, size);
        image.width = width;
        image.height = height;
        image.mipmaps = 1;
        image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
        Texture2D tex = LoadTextureFromImage(image);
		g_AtlasTexID = tex.id;
		io.Fonts->TexID = (void*)&g_AtlasTexID;
        free(pixels);
        free(image.data);
        g_UnloadAtlas = true;
    }
};

// Updates mouse position and button state
static void ImGui_ImplRaylib_UpdateMouseState()
{
  ImGuiIO& io = ImGui::GetIO();

  if (io.WantSetMousePos)
    SetMousePosition(io.MousePos.x, io.MousePos.y);
  else
    io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);

  io.MouseDown[0] = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
  io.MouseDown[1] = IsMouseButtonDown(MOUSE_RIGHT_BUTTON);
  io.MouseDown[2] = IsMouseButtonDown(MOUSE_MIDDLE_BUTTON);

  if (!IsWindowMinimized())
    io.MousePos = ImVec2(GetMouseX(), GetMouseY());
}

static void ImGui_ImplRaylib_UpdateMouseCursor()
{
  ImGuiIO& io = ImGui::GetIO();
  if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
    return;

  ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
  if (io.MouseDrawCursor || imgui_cursor == ImGuiMouseCursor_None)
    HideCursor(); // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
  else
    ShowCursor();
}


IMGUI_IMPL_API bool ImGui_ImplRaylib_Init()
{
  ImGuiIO& io = ImGui::GetIO();
  io.BackendPlatformName = "imgui_impl_raylib";

  io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
  io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
  
  ImGui_ImplRaylib_CreateFontsTexture();

  io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
  return true;
}

IMGUI_IMPL_API void ImGui_ImplRaylib_Shutdown()
{
  if (g_UnloadAtlas) 
  {
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->ClearTexData();
  }
  g_Time = 0.0;
}

IMGUI_IMPL_API void ImGui_ImplRaylib_NewFrame()
{
  ImGuiIO& io = ImGui::GetIO();

  io.DisplaySize = ImVec2((float)GetScreenWidth(), (float)GetScreenHeight());

  double current_time = GetTime();
  io.DeltaTime = g_Time > 0.0 ? (float)(current_time - g_Time) : (float)(1.0f / 60.0f);
  g_Time = current_time;

  io.KeyCtrl = IsKeyDown(KEY_RIGHT_CONTROL) || IsKeyDown(KEY_LEFT_CONTROL);
  io.KeyShift = IsKeyDown(KEY_RIGHT_SHIFT) || IsKeyDown(KEY_LEFT_SHIFT);
  io.KeyAlt = IsKeyDown(KEY_RIGHT_ALT) || IsKeyDown(KEY_LEFT_ALT);
  io.KeySuper = IsKeyDown(KEY_RIGHT_SUPER) || IsKeyDown(KEY_LEFT_SUPER);


  ImGui_ImplRaylib_UpdateMouseState();
  ImGui_ImplRaylib_UpdateMouseCursor();
  if (GetMouseWheelMove() > 0)
    io.MouseWheel += 1;
  else if (GetMouseWheelMove() < 0)
    io.MouseWheel -= 1;
}


IMGUI_IMPL_API void ImGui_ImplRaylib_RenderDrawData(ImDrawData* draw_data)
{
  auto DrawTriangleVertex = [](ImDrawVert vertex){
    Color* c = (Color*)&vertex.col;
    rlColor4ub(c->r, c->g, c->b, c->a);
    rlTexCoord2f(vertex.uv.x, vertex.uv.y);
    rlVertex2f(vertex.pos.x, vertex.pos.y);
  };

  rlDisableBackfaceCulling();
  for (int n = 0; n < draw_data->CmdListsCount; n++)
  {
    const ImDrawList* cmd_list = draw_data->CmdLists[n];
    const ImDrawVert* vtx_buffer = cmd_list->VtxBuffer.Data; // vertex buffer generated by Dear ImGui
    const ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;  // index buffer generated by Dear ImGui
    for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
    {
      const ImDrawCmd* pcmd = &(cmd_list->CmdBuffer.Data)[cmd_i]; // cmd_list->CmdBuffer->data[cmd_i];
      if (pcmd->UserCallback)
      {
        pcmd->UserCallback(cmd_list, pcmd);
      }
      else
      {
        ImVec2 pos = draw_data->DisplayPos;
        int rectX = (int)(pcmd->ClipRect.x - pos.x);
        int rectY = (int)(pcmd->ClipRect.y - pos.y);
        int rectW = (int)(pcmd->ClipRect.z - rectX);
        int rectH = (int)(pcmd->ClipRect.w - rectY);

        BeginScissorMode(rectX, rectY, rectW, rectH);
        unsigned int* ti = (unsigned int*)pcmd->TextureId;
        for (unsigned int i = 0; i <= (pcmd->ElemCount - 3); i += 3)
        {
            rlPushMatrix();
            rlBegin(RL_TRIANGLES);
            rlEnableTexture(*ti);

            ImDrawIdx index;
            ImDrawVert vertex;

            index = idx_buffer[i];
            vertex = vtx_buffer[index];
            DrawTriangleVertex(vertex);

            index = idx_buffer[i + 2];
            vertex = vtx_buffer[index];
            DrawTriangleVertex(vertex);

            index = idx_buffer[i + 1];
            vertex = vtx_buffer[index];
            DrawTriangleVertex(vertex);
            rlDisableTexture();
            rlEnd();
            rlPopMatrix();
        }
      }
      idx_buffer += pcmd->ElemCount;
    }
  }
  EndScissorMode();
  rlEnableBackfaceCulling();
}


IMGUI_IMPL_API bool ImGui_ImplRaylib_ProcessEvent()
{
    return true;
}
