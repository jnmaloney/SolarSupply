#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/fetch.h>
#endif
#include "graphics.h"
//#include "OdinEngine.h"


#include "RenderSystem.h"
#include "RenderQueue.h"
#include "WindowManager.h"
#include "MenuManager.h"
#include "imgui.h"
#include "OrbitCamera.h"
#include "Texture.h"
#include "HeightMap_Data.h"
#include "HeightMap_MeshTile.h"

#include "MenuMap.h"
#include "MenuInfo.h"

#include "json.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "MeshBank.h"
#include "SceneBatch.h"
#include "geoutil.h"


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
HeightMap_MeshTile* g_hmmArray[10][10];
HeightMap_Data* g_hmmLevel;
bool g_hmmLevelLoaded = false;
MenuMap g_menuMap;
MenuInfo g_menuInfo;

SceneBatch g_batch;
MeshBank g_meshBank;


//std::string g_location = "Brisbane";

void resetLocation();
void resetLocation_tiles();


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

    // //     test
    // int z = 7; // HeightMap
    // //int z = 13;
    // int tile_x_min, tile_y_min, tile_x_max, tile_y_max;
    // deg2num(pe->lat_min, pe->lon_min, z, tile_x_min, tile_y_min);
    // deg2num(pe->lat_max, pe->lon_max, z, tile_x_max, tile_y_max);
    // if (tile_x_min != tile_x_max || tile_y_min != tile_y_max)
    // {
    //   if ((tile_x_max - tile_x_min + 1) * (-tile_y_max + tile_y_min + 1) > 10)
    //   std::cout << pe->pc << " " << (tile_x_max - tile_x_min + 1) * (-tile_y_max + tile_y_min + 1) << " " << tile_x_min << " " << tile_x_max << " " << tile_y_min << " " << tile_y_max << " " << pe->name << std::endl;
    // }
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
// Fetch
//
void downloadSucceeded_webtile(emscripten_fetch_t *fetch)
{
  //printf("Finished downloading %llu bytes from URL %s.\n", fetch->numBytes, fetch->url);
  // The data is now available at fetch->data[0] through fetch->data[fetch->numBytes-1];

  Texture* t = (Texture*)fetch->userData;
  t->loadPng_fromMemory(fetch->data, fetch->numBytes);

  emscripten_fetch_close(fetch); // Free data associated with the fetch.
}


void downloadSucceeded_heightmap(emscripten_fetch_t *fetch)
{
  printf("HEIGHTMAP: Finished downloading %llu bytes from URL %s.\n", fetch->numBytes, fetch->url);
  // The data is now available at fetch->data[0] through fetch->data[fetch->numBytes-1];

  //HeightMap* h = (HeightMap*)fetch->userData;
  //h->HloadPngRaw_fromMemory(fetch->data, fetch->numBytes);

  g_hmmLevel->HloadPngRaw_fromMemory(fetch->data, fetch->numBytes);
  //printf("HEIGHTMAP: did loaded    ");

  // Tiles from data
  g_hmmArray[0][0] = g_hmmLevel->createTile(128 + 0 * 8, 128 + 0 * 8, 8 + 1, 8 + 1);
  g_hmmArray[0][1] = g_hmmLevel->createTile(128 + 0 * 8, 128 + 1 * 8, 8 + 1, 8 + 1);
  g_hmmArray[1][0] = g_hmmLevel->createTile(128 + 1 * 8, 128 + 0 * 8, 8 + 1, 8 + 1);
  g_hmmArray[1][1] = g_hmmLevel->createTile(128 + 1 * 8, 128 + 1 * 8, 8 + 1, 8 + 1);
  g_hmmArray[2][0] = g_hmmLevel->createTile(128 + 2 * 8, 128 + 0 * 8, 8 + 1, 8 + 1);
  g_hmmArray[2][1] = g_hmmLevel->createTile(128 + 2 * 8, 128 + 1 * 8, 8 + 1, 8 + 1);

  g_hmmLevelLoaded = true;
  emscripten_fetch_close(fetch); // Free data associated with the fetch.
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
  //g_rs->setCameraPos(cameraControl.getPos(), cameraControl.pivot);
  g_rs->setCameraPos(
    glm::vec3(0, 0, -200),
    glm::vec3(0, 0, 10),
    glm::vec3(1, 0, 0));
  g_rs->start();

  // if (g_hmmLevelLoaded)
  // {
  //   cameraControl.pivot.z = g_hmmLevel->mMaxH;
  //
  //   for (int i = 0; i < 3; ++i)
  //   {
  //     for (int j = 0; j < 2; ++j)
  //     {
  //       g_rs->bindMesh(g_hmmArray[i][j]);
  //       g_rs->bindMeshElement(g_hmmArray[i][j], 0);
  //       g_texArray[i][j]->bind();
  //       glm::mat4 xform(1.0);
  //       if (g_rs->testModelLocal(xform))
  //       {
  //         g_rs->drawMesh();
  //       }
  //     }
  //   }
  // }


  g_batch.draw(g_rs, &g_meshBank);

  g_rs->end();

  // ImGui
  g_menuManager.predraw();
  //g_menuMap.draw(g_texArray);
  bool set = g_menuInfo.draw();
  if (set)
  {
    PostcodeExtents* e = g_name_to_extents[g_menuInfo.m_location];

    double deg_to_pix_x = 16.977777777777778;
    double deg_to_pix_y = 19.082602954611577;

    double x0_pixel = -deg_to_pix_x * 146.25 + 737;
    double y0_pixel =  deg_to_pix_y * -21.943045533438177 + 266;

    g_menuInfo.x_label = x0_pixel + deg_to_pix_x * e->lon;
    g_menuInfo.y_label = y0_pixel - deg_to_pix_y * e->lat;

    PostcodeInstalls* i = g_pc_to_installs[e->pc];
    g_menuInfo.installs = i->installs;
    g_menuInfo.dwellings = i->dwellings;
    g_menuInfo.cap_tot = i->cap_tot;
    g_menuInfo.cap_under10 = i->cap_under10;
    g_menuInfo.cap_10_100 = i->cap_10_100;
    g_menuInfo.cap_over100 = i->cap_over100;

  }
  g_menuManager.postdraw();
}


//
// Input
//
void input()
{
  if (g_menuInfo.resetLocation)
  {
    g_menuInfo.resetLocation = 0;
    resetLocation();
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
  g_windowManager.width = 1024;
  g_windowManager.height = 768;
  g_windowManager.init("Project");
  g_menuManager.fontNameTTF =  "data/font/OpenSans-Regular.ttf";
  g_menuManager.init(g_windowManager);

  g_rs = new RenderSystem();
  g_rs->init();

  cameraControl.pivot.x = 128.0;
  cameraControl.pivot.y = 128.0;
  cameraControl.zoom(10);

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
  // WMS Texture load for a location
  //
  g_texArray[0][0] = new Texture();
  g_texArray[0][1] = new Texture();
  g_texArray[1][0] = new Texture();
  g_texArray[1][1] = new Texture();
  g_texArray[2][0] = new Texture();
  g_texArray[2][1] = new Texture();

  g_hmmLevel = new HeightMap_Data();

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
  g_meshBank.add("aus_all", "data/australia_states.obj", "data/bg1.png");
  g_meshBank.load();
  glm::mat4 mvp(1.0);
  mvp = glm::scale(mvp, glm::vec3(1280.0));
  g_batch.addElement("aus_all", mvp);

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


// Place has been changed...
void resetLocation()
{
}


void resetLocation_tiles()
{
  int o_x, o_y;
  int z = 7; // 15
  deg2num(g_menuInfo.m_lat, g_menuInfo.m_lon, z, o_x, o_y);

  // HeightMap tile

  {
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess = downloadSucceeded_heightmap;
    attr.onerror = downloadFailed;
    attr.userData = 0;

    int x = o_x;
    int y = o_y;

    std::string url = std::string("https://api.mapbox.com/v4/mapbox.terrain-rgb/{z}/{x}/{y}.pngraw?access_token=pk.eyJ1Ijoiam5tYWxvbmV5IiwiYSI6ImNrMXJqM3BjdDAxeHMzaG1naHo1bGRreWUifQ.tEcnjy-7zLUQP-EuncDW0Q");
    url = url.replace(url.find("{x}"), 3, std::to_string(x));
    url = url.replace(url.find("{y}"), 3, std::to_string(y));
    url = url.replace(url.find("{z}"), 3, std::to_string(z));
    emscripten_fetch(&attr, url.c_str());
  }

  // Web Tiles

  z = 13;
  deg2num(g_menuInfo.m_lat, g_menuInfo.m_lon, z, o_x, o_y);

  for (int i = 0; i < 4; ++i)
  {
    for (int j = 0; j < 3; ++j)
    {
      g_texArray[i][j] = new Texture();

      emscripten_fetch_attr_t attr;
      emscripten_fetch_attr_init(&attr);
      strcpy(attr.requestMethod, "GET");
      attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
      attr.onsuccess = downloadSucceeded_webtile;
      attr.onerror = downloadFailed;
      attr.userData = g_texArray[i][j];

      int x = o_x + i - 2;//3;
      int y = o_y + j - 1;

      // key =  solar-870093db
      //std::string url = std::string("https://maps.omniscale.net/v2/solar-870093db/style.outdoor/{z}/{x}/{y}.png");
      std::string url = std::string("https://maps.omniscale.net/v2/opensolar-public-aa5ae3b0/style.outdoor/{z}/{x}/{y}.png");
      url = url.replace(url.find("{x}"), 3, std::to_string(x));
      url = url.replace(url.find("{y}"), 3, std::to_string(y));
      url = url.replace(url.find("{z}"), 3, std::to_string(z));
      //printf("%s\n", url.c_str());

      emscripten_fetch(&attr, url.c_str());

    }
  }
}
