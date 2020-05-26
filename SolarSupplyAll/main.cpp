#include <stdarg.h>
#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include "graphics.h"
//#include "OdinEngine.h"


#include "RenderSystem.h"
#include "RenderQueue.h"
#include "WindowManager.h"
#include "MenuManager.h"
#include "imgui.h"
#include "OrbitCamera.h"

#include "MenuMap.h"
#include "MenuInfo.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "LandscapeModel.h"
#include "geoutil.h"


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
LandscapeModel g_landscapeModel;

MenuMap g_menuMap;
MenuInfo g_menuInfo;


struct PostcodeExtents
{
  std::string name;
  std::string pc;
  double lon_min;
  double lon_max;
  double lon;
  double lat_min;
  double lat_max;
  double lat;
};
std::map<std::string, PostcodeExtents*> g_pc_to_extents;
std::map<std::string, PostcodeExtents*> g_name_to_extents;
std::list<std::string> g_names;


struct PostcodeInstalls
{
  char postcode[5];
  int dwellings = 0;
  int installs = 0;
  float cap_tot = 0;
  float cap_under10 = 0;
  float cap_10_100 = 0;
  float cap_over100 = 0;
};
std::map<std::string, PostcodeInstalls*> g_pc_to_installs;


//
// Load postcodes data file
//
void load_postcodes_data()
{
  //FILE* f = open("data/postcode_envelopes.txt");
  std::ifstream infile("data/postcode_envelopes.txt");
  char comma = ';';
  while (!infile.eof())
  {
    std::string line;
    std::getline(infile, line);

    std::istringstream ss1(line);
    std::string pc, name, ext, area;
    std::string lon_min, lon_max, lat_min, lat_max;
    //while (ss1)
    //{
      std::getline(ss1, pc, ';'); // pc
      std::getline(ss1, name, ';'); // name
      std::getline(ss1, ext, ';'); // extents
      std::getline(ss1, area); // area

      std::istringstream ss2(ext);
      std::getline(ss2, lon_min, ',');
      std::getline(ss2, lon_max, ',');
      std::getline(ss2, lat_min, ',');
      std::getline(ss2, lat_max);
    //}

    PostcodeExtents* pe = new PostcodeExtents;
    pe->name = name;
    pe->pc = pc;
    pe->lon_min = atof(lon_min.c_str());
    pe->lon_max = atof(lon_max.c_str());
    pe->lon = 0.5 * (pe->lon_max + pe->lon_min);
    pe->lat_min = atof(lat_min.c_str());
    pe->lat_max = atof(lat_max.c_str());
    pe->lat = 0.5 * (pe->lat_max + pe->lat_min);

    g_pc_to_extents[pc] = pe;
    g_name_to_extents[name] = pe;
    g_names.push_back(name);
  }
}


void load_postcodes_installs()
{
  std::ifstream infile("data/postcode_installs.bin", std::ios::in | std::ios::binary);
  while (!infile.eof())
  {
    PostcodeInstalls* i = new PostcodeInstalls;
    infile >> i->postcode[0];
    infile >> i->postcode[1];
    infile >> i->postcode[2];
    infile >> i->postcode[3];
    i->postcode[4] = 0;
    // infile >> i->dwellings;
    // infile >> i->installs;
    // infile >> i->installs;
    // infile >> i->installs;
    // infile >> i->cap_tot;
    // infile >> i->cap_under10;
    // infile >> i->cap_10_100;
    // infile >> i->cap_over100;
    // for (int i = 0; i < 5 * 4; ++i)
    // {
    //   char x;
    //   infile >> x;
    // }
    infile.read(reinterpret_cast<char*>(&i->installs), sizeof(int));
    infile.read(reinterpret_cast<char*>(&i->dwellings), sizeof(int));
    infile.read(reinterpret_cast<char*>(&i->cap_tot), sizeof(float));
    infile.read(reinterpret_cast<char*>(&i->cap_under10), sizeof(float));
    infile.read(reinterpret_cast<char*>(&i->cap_10_100), sizeof(float));
    infile.read(reinterpret_cast<char*>(&i->cap_over100), sizeof(float));

    g_pc_to_installs[std::string(i->postcode)] = i;

    // std::cout << "pc: " << i->postcode << std::endl;

    // std::cout << "dwellings  " << i->dwellings << std::endl;
    // std::cout << "installs  " << i->installs << std::endl;
    // std::cout << "cap_tot  " << i->cap_tot << std::endl;
    // std::cout << "cap_under10  " << i->cap_under10 << std::endl;
    // std::cout << "cap_10_100  " << i->cap_10_100 << std::endl;
    // std::cout << "cap_over100  " << i->cap_over100 << std::endl;
  }
}


// Find matches in g_name
void find_matches(std::string in, std::vector<std::string>& out)
{
  for (auto& i : g_names)
  {
    // match in vs i
    // append to out
  }
}


//
// Drawing
//
void draw()
{
  static bool g_viewMode = 0;

  if (g_viewMode == 0)
  {
    g_rs->start();
    g_rs->end();
  }
  if (g_viewMode == 1)
  {
    cameraControl.update(1.0);
    g_rs->setCameraPos(cameraControl.getPos(), cameraControl.pivot, glm::vec3(0, 0, 1));
    g_rs->start();
    g_landscapeModel.draw(g_rs);
    g_rs->end();
  }

  // ImGui
  g_menuManager.predraw();
  //ImGui::Checkbox("3D view", &g_viewMode);
  bool set = g_menuInfo.draw();
  static std::string s_setPostcode = "";
  if (set)
  {  
    PostcodeExtents* e = g_name_to_extents[g_menuInfo.m_location];
    s_setPostcode = e->pc;
 
    //double s = 2.68;
    double s = g_menuInfo.m_s_x;
    // double deg_to_pix_x = (s * 256.0 / 90.0);
    // double deg_to_pix_y = (s * 256.0 / -66.5);

    double x0_pixel = 242 - 64 * s + 16;
    double y0_pixel = 32 + 16;

    // // g_menuInfo.x_label = x0_pixel + deg_to_pix_x * (e->lon - 90.0);
    // // g_menuInfo.y_label = y0_pixel + deg_to_pix_y * e->lat;
    // g_menuInfo.x_label = x0_pixel + deg_to_pix_x * (0);
    // g_menuInfo.y_label = y0_pixel + deg_to_pix_y * 0;

    PostcodeInstalls* i = g_pc_to_installs[e->pc];
    g_menuInfo.installs = i->installs;
    g_menuInfo.dwellings = i->dwellings;
    g_menuInfo.cap_tot = i->cap_tot;
    g_menuInfo.cap_under10 = i->cap_under10;
    g_menuInfo.cap_10_100 = i->cap_10_100;
    g_menuInfo.cap_over100 = i->cap_over100;

    // okay den
    float xf, yf;
    deg2numf(e->lat, e->lon_min, 2, xf, yf);
    xf -= 3;
    yf -= 2;
    g_menuInfo.x_label = x0_pixel + xf * 256 * s;
    g_menuInfo.y_label = y0_pixel + yf * 256 * s;
    
    deg2numf(e->lat_min, e->lon_min, 2, xf, yf);
    xf -= 3;
    yf -= 2;   
    g_menuInfo.x_patch_min = x0_pixel + xf * 256 * s - 1;
    g_menuInfo.y_patch_min = y0_pixel + yf * 256 * s + 1;

    deg2numf(e->lat_max, e->lon_max, 2, xf, yf);
    xf -= 3;
    yf -= 2;  
    g_menuInfo.x_patch_max = x0_pixel + xf * 256 * s + 1;
    g_menuInfo.y_patch_max = y0_pixel + yf * 256 * s - 1;  

    // 3D
    //g_landscapeModel.setLocation(e->lon_min, e->lon_max, e->lat_min, e->lat_max);

    // Center
    // int tile_x_min, tile_y_min, tile_x_max, tile_y_max;
    // int z = 7;
    // deg2num(e->lat_min, e->lon_min, z, tile_x_min, tile_y_min);
    // deg2num(e->lat_max, e->lon_max, z, tile_x_max, tile_y_max);
    // float tile_x_center, tile_y_center;
    // deg2numf(e->lat, e->lon, z, tile_x_center, tile_y_center);

    // // float x = 0.5 * (1 + tile_x_max - tile_x_min);
    // // float y = 0.5 * (1 + tile_y_min - tile_y_max);
    // float x = 1.0 - (tile_x_center - tile_x_min);
    // float y = tile_y_center - tile_y_max;
    // std::cout << x << std::endl;
    // std::cout << tile_x_center << std::endl;
    // cameraControl.pivot.x = 0.1 * 256 * x;
    // cameraControl.pivot.y = 0.1 * 256 * y;

    // And then checking the postcode generated daily data
    // Once per set()
    EM_ASM({
      return postcode_x($0);
      }, atoi(e->pc.c_str()));
    
    // if (g_menuInfo.m_a < 0)
    // {
    //   double x = EM_ASM_DOUBLE({
    //       return daily_total;
    //       });
    //   g_menuInfo.m_a = x;
    // }    
    // if (g_menuInfo.m_b < 0)
    // {
    //   double x = EM_ASM_DOUBLE({
    //       return daily_state_total;
    //       });
    //   g_menuInfo.m_b = x;
    // }    
  }
  g_menuManager.postdraw();

  // Some additional settings:
  // Selected postcode
  if (g_menuInfo.m_x < 0 && s_setPostcode.size())
  {
    double x = EM_ASM_DOUBLE({
        return postcode_z($0);
        }, atoi(s_setPostcode.c_str()));
    g_menuInfo.m_x = x;
  }  

  // Daily Total
  if (g_menuInfo.m_daily_all < 0)
  {
    double x = EM_ASM_DOUBLE({
        return power_value;
        });
    g_menuInfo.m_daily_all = x;
  }  

  // Daily Total (State)
  if (g_menuInfo.m_daily_state.size() < 9)
  {
    // Init array
    g_menuInfo.m_daily_state.resize(9);
    for (int i = 0; i < 9; ++i)
    {
      g_menuInfo.m_daily_state[i] = -1;
    }
  }
  else
  {
    // Check array values with JS File Loader (async)
    for (int i = 0; i < 9; ++i)
    {
      if (g_menuInfo.m_daily_state[i] < 0)
      {
        g_menuInfo.m_daily_state[i] = EM_ASM_DOUBLE({ return getStateData($0); }, i);
      }
    }
  }
}


//
// Input
//
void input()
{
  if (g_menuInfo.resetLocation)
  {
    g_menuInfo.resetLocation = 0;
  }
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
    //g_rs->setWindow()
  }

  //
  // Normal operation:
  //
  input();
  draw();
}


int init()
{
  // g_windowManager.width = 1024;
  // g_windowManager.height = 768;
  // g_windowManager.width = 800;
  // g_windowManager.height = 600;
    g_windowManager.width = 640;
  g_windowManager.height = 480;
  //   g_windowManager.width = 480;
  // g_windowManager.height = 270;  
  g_windowManager.init("Project 2020");
  g_menuManager.fontNameTTF = "data/font/OpenSans-Regular.ttf";
  g_menuManager.init(g_windowManager);

  g_rs = new RenderSystem();
  g_rs->init();

  cameraControl.pivot.x = 128.0;
  cameraControl.pivot.y = 128.0;
  cameraControl.zoom(10);

   g_landscapeModel.init();

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
  //style->Colors[ImGuiCol_ChildWindowBg]         = ImVec4(1.00f, 0.98f, 0.95f, 0.58f);
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
  //style->Colors[ImGuiCol_Column]                = ImVec4(0.00f, 0.00f, 0.00f, 0.32f);
  //style->Colors[ImGuiCol_ColumnHovered]         = ImVec4(0.25f, 1.00f, 0.00f, 0.78f);
  //style->Colors[ImGuiCol_ColumnActive]          = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
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

  //
  // Create a Mesh
  //
  //g_meshBank.add("aus_all", "data/australia_states.obj", "data/bg1.png");
  //g_meshBank.load();
  //glm::mat4 mvp(1.0);
  //mvp = glm::scale(mvp, glm::vec3(1280.0));
  //g_batch.addElement("aus_all", mvp);

  return 0;
}


void quit()
{
  glfwTerminate();
}


extern "C" int main(int argc, char** argv)
{
  if (init() != 0) return 1;
  load_postcodes_data();
  load_postcodes_installs();
  g_menuInfo.m_names = &g_names;

  #ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(loop, 0, 1);
  #endif

  quit();

  return 0;
}

