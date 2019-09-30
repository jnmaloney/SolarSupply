#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/fetch.h>
#endif
#include "graphics.h"


#include "RenderSystem.h"
#include "RenderQueue.h"
#include "WindowManager.h"
#include "MenuManager.h"
#include "imgui.h"
#include "OrbitCamera.h"
#include "Texture.h"

#include "MenuMap.h"
#include "MenuInfo.h"

#include "json.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


using nlohmann::json;


RenderSystem* g_rs = 0;
WindowManager g_windowManager;
MenuManager g_menuManager;
OrbitCamera cameraControl;
bool m_mouseDrag = false;
double g_xpos = 0;
double g_ypos = 0;
int g_actionButton = GLFW_MOUSE_BUTTON_LEFT;
int g_cameraButton = GLFW_MOUSE_BUTTON_RIGHT;
int g_cancelButton = GLFW_MOUSE_BUTTON_RIGHT;

Texture* g_texArray[10][10];
MenuMap g_menuMap;
MenuInfo g_menuInfo;

//std::string g_location = "Brisbane";

void resetLocation();
void resetLocation_tiles();

//
// Fetch
//
void downloadSucceeded(emscripten_fetch_t *fetch)
{
  //printf("Finished downloading %llu bytes from URL %s.\n", fetch->numBytes, fetch->url);
  // The data is now available at fetch->data[0] through fetch->data[fetch->numBytes-1];

  Texture* t = (Texture*)fetch->userData;
  t->loadPng_fromMemory(fetch->data, fetch->numBytes);

  emscripten_fetch_close(fetch); // Free data associated with the fetch.
}


void downloadSucceeded_openweather(emscripten_fetch_t *fetch)
{
  // slow parse,,,
  std::string x;
  for (int i = 0; i < fetch->numBytes; ++i)
  {
    x += fetch->data[i];
  }
  printf("%s\n", x.c_str());
  json response_data = json::parse(x);

  g_menuInfo.m_lon = response_data["coord"]["lon"];
  g_menuInfo.m_lat = response_data["coord"]["lat"];
  g_menuInfo.m_temp = response_data["main"]["temp"];
  g_menuInfo.m_clouds = response_data["clouds"]["all"];
  g_menuInfo.m_visibility = response_data["visibility"];
  g_menuInfo.m_sunrise = response_data["sys"]["sunrise"];
  g_menuInfo.m_sunset = response_data["sys"]["sunset"];

  emscripten_fetch_close(fetch); // Free data associated with the fetch.

  resetLocation_tiles();
}


void downloadFailed(emscripten_fetch_t *fetch)
{
  printf("Downloading %s failed, HTTP failure status code: %d.\n", fetch->url, fetch->status);
  emscripten_fetch_close(fetch); // Also free data on failure.
}

//
// Drawing
//
void draw()
{
  // Set camera view
  cameraControl.update(1.0);
  g_rs->setCameraPos(cameraControl.getPos(), cameraControl.pivot);
  g_rs->start();
  g_rs->end();

  // ImGui
  g_menuManager.predraw();
  g_menuMap.draw(g_texArray);
  g_menuInfo.draw();
  g_menuManager.postdraw(g_windowManager);

  if (g_menuInfo.resetLocation)
  {
    g_menuInfo.resetLocation = 0;
    resetLocation();
  }
}


//
// Input
//
void input()
{
}


void _window_size_callback(GLFWwindow* window, int width, int height)
{
  g_windowManager.width = width;
  g_windowManager.height = height;

  //g_rs->setWindow(width, height);
}


void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
  if (ImGui::GetIO().WantCaptureMouse) return;

  g_xpos = xpos; g_ypos = ypos;

  if (cameraControl.move)
  {
    //cameraControl.moveIt(xpos - g_xpos, ypos - g_ypos);

    // HACK THE CAMERA RAY
    glm::vec3 pivot_reset = cameraControl.pivot;
    cameraControl.pivot = cameraControl.worldMoveBeginPivot;
    g_rs->setCameraPos(cameraControl.getPos(), cameraControl.pivot); // Stting extra ?

    glm::vec3 origin = cameraControl.getPos();
    glm::vec3 ray1 = g_rs->createRay(xpos, ypos);
    float d = -origin.z / ray1.z;
    glm::vec3 pos1 = origin + d * ray1;

    glm::vec3 moveVector = pos1 - cameraControl.worldMoveBeginClick;
    cameraControl.pivot = cameraControl.worldMoveBeginPivot - moveVector;
  }

  //g_actionManager.cursorPos((int)(0.5f + g_rs->m_cursorX), (int)(0.5f + g_rs->m_cursorY));
}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
  if (ImGui::GetIO().WantCaptureMouse) return;

  //g_is->mouse_button_callback(button, action, mods);

  if (button == g_actionButton && action == GLFW_PRESS)
  {
  }

  if (button == g_cameraButton && action == GLFW_PRESS)
  {
    // Trigger camera movement
    cameraControl.move = true;//(action == GLFW_PRESS);
    // cameraControl.mouseBeginMoveX = g_xpos;
    // cameraControl.mouseBeginMoveY = g_ypos;
    cameraControl.worldMoveBeginPivot = cameraControl.pivot;

    // Calculate world begin click ray (camera move)
    glm::vec3 origin = cameraControl.getPos();
    glm::vec3 ray0 = g_rs->createRay(g_xpos, g_ypos);
    float d = -origin.z / ray0.z;
    cameraControl.worldMoveBeginClick = origin + d * ray0;
  }

  if (button == g_actionButton && action == GLFW_RELEASE)
  {
  }

  if (button == g_cameraButton && action == GLFW_RELEASE)
  {
    cameraControl.move = false;
  }

  if (button == g_cancelButton && action == GLFW_PRESS)
  {
  }
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  if (yoffset > 0.0) cameraControl.zoom( 1);
  if (yoffset < 0.0) cameraControl.zoom(-1);
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
}


void loop()
{
  // TODO / Resize...
  static int s_width = g_windowManager.width;
  static int s_height = g_windowManager.height;
  if (g_windowManager.width != s_width || g_windowManager.height != s_height)
  {
    s_width = g_windowManager.width;
    s_height = g_windowManager.height;
    g_rs->setProjectionPerspective(30.f); // ?
  }

  //
  // Normal operation:
  //
  input();
  draw();
}


int init()
{
  g_windowManager.width = 1024;
  g_windowManager.height = 768;
  g_windowManager.init("Project");
  g_menuManager.fontNameTTF =  "data/font/xkcd-script.ttf";
  g_menuManager.init(g_windowManager);

  g_rs = new RenderSystem();
  g_rs->init();

  cameraControl.pivot.x = 50 * 2.0;
  cameraControl.pivot.y = 50 * 2.0;

  // Cursor callbacks
  glfwSetCursorPosCallback(g_windowManager.g_window, cursor_pos_callback);
  glfwSetMouseButtonCallback(g_windowManager.g_window, mouse_button_callback);
  glfwSetScrollCallback(g_windowManager.g_window, scroll_callback);

  // Key callbacks
  glfwSetKeyCallback(g_windowManager.g_window, ImGui_ImplGlfw_KeyCallback);
  glfwSetCharCallback(g_windowManager.g_window, ImGui_ImplGlfw_CharCallback);

  // Set Camera Projection
  g_rs->setProjectionPerspective(30.f);

  //
  // Weather API load for a location
  //
  // {
  //   std::string request = "https://api.openweathermap.org/data/2.5/weather?q=Brisbane&APPID=f78f50c399b86a78970da6eab769c6de";
  //
  //   emscripten_fetch_attr_t attr;
  //   emscripten_fetch_attr_init(&attr);
  //   strcpy(attr.requestMethod, "POST");
  //   attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
  //   attr.onsuccess = downloadSucceeded_openweather;
  //   attr.onerror = downloadFailed;
  //   //attr.userData = 0;
  //   emscripten_fetch(&attr, request.c_str());
  // }

  //
  // WMS Texture load for a location
  //
  g_texArray[0][0] = new Texture();
  g_texArray[0][1] = new Texture();
  g_texArray[1][0] = new Texture();
  g_texArray[1][1] = new Texture();
  g_texArray[2][0] = new Texture();
  g_texArray[2][1] = new Texture();

  // for (int i = 0; i < 4; ++i)
  // {
  //   for (int j = 0; j < 3; ++j)
  //   {
  //     g_texArray[i][j] = new Texture();
  //
  //     emscripten_fetch_attr_t attr;
  //     emscripten_fetch_attr_init(&attr);
  //     strcpy(attr.requestMethod, "GET");
  //     attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
  //     attr.onsuccess = downloadSucceeded;
  //     attr.onerror = downloadFailed;
  //     attr.userData = g_texArray[i][j];
  //
  //     int x = 115 + i;
  //     int y = 72 + j;
  //     int z = 7;
  //
  //     std::string url = std::string("https://maps.omniscale.net/v2/local-c988aaf7/style.outdoor/{z}/{x}/{y}.png");
  //     url = url.replace(url.find("{x}"), 3, std::to_string(x));
  //     url = url.replace(url.find("{y}"), 3, std::to_string(y));
  //     url = url.replace(url.find("{z}"), 3, std::to_string(z));
  //     printf("%s\n", url.c_str());
  //
  //     emscripten_fetch(&attr, url.c_str());
  //
  //   }
  // }

  //
  // ImGui Style
  //
  ImGuiIO& io = ImGui::GetIO();

  io.Fonts->Clear();
  io.Fonts->AddFontFromFileTTF("/data/font/OpenSans-Light.ttf", 16);
  io.Fonts->AddFontFromFileTTF("/data/font/OpenSans-Regular.ttf", 16);
  io.Fonts->AddFontFromFileTTF("/data/font/OpenSans-Light.ttf", 32);
  io.Fonts->AddFontFromFileTTF("/data/font/OpenSans-Regular.ttf", 11);
  io.Fonts->AddFontFromFileTTF("/data/font/OpenSans-Bold.ttf", 11);
  io.Fonts->Build();

  ImGuiStyle * style = &ImGui::GetStyle();

  style->WindowPadding            = ImVec2(15, 15);
  style->WindowRounding           = 5.0f;
  style->FramePadding             = ImVec2(5, 5);
  style->FrameRounding            = 4.0f;
  style->ItemSpacing              = ImVec2(12, 8);
  style->ItemInnerSpacing         = ImVec2(8, 6);
  style->IndentSpacing            = 25.0f;
  style->ScrollbarSize            = 15.0f;
  style->ScrollbarRounding        = 9.0f;
  style->GrabMinSize              = 5.0f;
  style->GrabRounding             = 3.0f;

  style->Colors[ImGuiCol_Text]                  = ImVec4(0.40f, 0.39f, 0.38f, 1.00f);
  style->Colors[ImGuiCol_TextDisabled]          = ImVec4(0.40f, 0.39f, 0.38f, 0.77f);
  style->Colors[ImGuiCol_WindowBg]              = ImVec4(0.92f, 0.91f, 0.88f, 0.70f);
  style->Colors[ImGuiCol_ChildWindowBg]         = ImVec4(1.00f, 0.98f, 0.95f, 0.58f);
  style->Colors[ImGuiCol_PopupBg]               = ImVec4(0.92f, 0.91f, 0.88f, 0.92f);
  style->Colors[ImGuiCol_Border]                = ImVec4(0.84f, 0.83f, 0.80f, 0.65f);
  style->Colors[ImGuiCol_BorderShadow]          = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
  style->Colors[ImGuiCol_FrameBg]               = ImVec4(1.00f, 0.98f, 0.95f, 1.00f);
  style->Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.99f, 1.00f, 0.40f, 0.78f);
  style->Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.26f, 1.00f, 0.00f, 1.00f);
  style->Colors[ImGuiCol_TitleBg]               = ImVec4(1.00f, 0.98f, 0.95f, 1.00f);
  style->Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
  style->Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
  style->Colors[ImGuiCol_MenuBarBg]             = ImVec4(1.00f, 0.98f, 0.95f, 0.47f);
  style->Colors[ImGuiCol_ScrollbarBg]           = ImVec4(1.00f, 0.98f, 0.95f, 1.00f);
  style->Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.00f, 0.00f, 0.00f, 0.21f);
  style->Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.90f, 0.91f, 0.00f, 0.78f);
  style->Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
  //style->Colors[ImGuiCol_ComboBg]               = ImVec4(1.00f, 0.98f, 0.95f, 1.00f);
  style->Colors[ImGuiCol_CheckMark]             = ImVec4(0.25f, 1.00f, 0.00f, 0.80f);
  style->Colors[ImGuiCol_SliderGrab]            = ImVec4(0.00f, 0.00f, 0.00f, 0.14f);
  style->Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
  style->Colors[ImGuiCol_Button]                = ImVec4(0.00f, 0.00f, 0.00f, 0.14f);
  style->Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.99f, 1.00f, 0.22f, 0.86f);
  style->Colors[ImGuiCol_ButtonActive]          = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
  style->Colors[ImGuiCol_Header]                = ImVec4(0.25f, 1.00f, 0.00f, 0.76f);
  style->Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.25f, 1.00f, 0.00f, 0.86f);
  style->Colors[ImGuiCol_HeaderActive]          = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
  style->Colors[ImGuiCol_Column]                = ImVec4(0.00f, 0.00f, 0.00f, 0.32f);
  style->Colors[ImGuiCol_ColumnHovered]         = ImVec4(0.25f, 1.00f, 0.00f, 0.78f);
  style->Colors[ImGuiCol_ColumnActive]          = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
  style->Colors[ImGuiCol_ResizeGrip]            = ImVec4(0.00f, 0.00f, 0.00f, 0.04f);
  style->Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.25f, 1.00f, 0.00f, 0.78f);
  style->Colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
  // style->Colors[ImGuiCol_CloseButton]           = ImVec4(0.40f, 0.39f, 0.38f, 0.16f);
  // style->Colors[ImGuiCol_CloseButtonHovered]    = ImVec4(0.40f, 0.39f, 0.38f, 0.39f);
  // style->Colors[ImGuiCol_CloseButtonActive]     = ImVec4(0.40f, 0.39f, 0.38f, 1.00f);
  style->Colors[ImGuiCol_PlotLines]             = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
  style->Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
  style->Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
  style->Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
  style->Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
  style->Colors[ImGuiCol_ModalWindowDarkening]  = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);

  return 0;
}


void quit()
{
  glfwTerminate();
}


extern "C" int main(int argc, char** argv)
{
  if (init() != 0) return 1;

  #ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(loop, 0, 1);
  #endif

  quit();

  return 0;
}


void deg2num(double lat_deg, double lon_deg, int zoom, int& out_x, int& out_y)
{
  double lat_rad = 0.01745329 * (lat_deg);
  double n = pow(2.0, zoom);
  int xtile = (lon_deg + 180.0) / 360.0 * n;
  int ytile = (1.0 - asinh(tan(lat_rad)) / M_PI) / 2.0 * n;
  out_x = xtile;
  out_y = ytile;
}


void resetLocation()
{
  {
    //std::string request = "https://api.openweathermap.org/data/2.5/weather?q=Brisbane&APPID=f78f50c399b86a78970da6eab769c6de";
    std::string request = "https://api.openweathermap.org/data/2.5/weather?q={0}&APPID=f78f50c399b86a78970da6eab769c6de";
    request = request.replace(request.find("{0}"), 3, g_menuInfo.m_location);

    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "POST");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess = downloadSucceeded_openweather;
    attr.onerror = downloadFailed;
    //attr.userData = 0;
    emscripten_fetch(&attr, request.c_str());
  }

}


void resetLocation_tiles()
{
  int o_x, o_y;
  int z = 7;
  deg2num(g_menuInfo.m_lat, g_menuInfo.m_lon, 7, o_x, o_y);

  for (int i = 0; i < 4; ++i)
  {
    for (int j = 0; j < 3; ++j)
    {
      g_texArray[i][j] = new Texture();

      emscripten_fetch_attr_t attr;
      emscripten_fetch_attr_init(&attr);
      strcpy(attr.requestMethod, "GET");
      attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
      attr.onsuccess = downloadSucceeded;
      attr.onerror = downloadFailed;
      attr.userData = g_texArray[i][j];

      int x = o_x + i - 2;//3;
      int y = o_y + j - 1;

      // key =  solar-870093db 
      std::string url = std::string("https://maps.omniscale.net/v2/solar-870093db/style.outdoor/{z}/{x}/{y}.png");
      url = url.replace(url.find("{x}"), 3, std::to_string(x));
      url = url.replace(url.find("{y}"), 3, std::to_string(y));
      url = url.replace(url.find("{z}"), 3, std::to_string(z));
      printf("%s\n", url.c_str());

      emscripten_fetch(&attr, url.c_str());

    }
  }
}
