#include "MenuInfo.h"
#include "imgui.h"
#include "WindowManager.h"
#include "Texture.h"
#include "time.h"
#include "Util.h"
#include <iostream>

#include <sstream>
template<class T>
std::string FormatWithCommas(T value)
{
    std::stringstream ss;
    ss.imbue(std::locale(""));
    ss << std::fixed << value;
    return ss.str();
}


MenuInfo::MenuInfo()
{}


MenuInfo::~MenuInfo()
{}


bool MenuInfo::draw()
{
  int screenwidth = ImGui::GetIO().DisplaySize.x;
  int screenheight = ImGui::GetIO().DisplaySize.y;
    int LABEL_FLAGS = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoInputs;

  // Au MAP
  {
    static Texture* texture = 0;
    if (texture == 0)
    {
      texture = new Texture;
      texture->loadPng("data/tile121.png");
    }
    ImTextureID id = (ImTextureID)texture->getID();

    int x = 242;
    int y = 32;
    int screen_wanted_x = screenwidth - 2 * x;
    int pixel_src_x = 118;
    double s_x = (double)screen_wanted_x / (double)pixel_src_x;
    int w = 256.0 * s_x;
    int h = w;
    x -= 64 * s_x;
    m_s_x = s_x;

    // std::cout << x << std::endl;
    // std::cout << y << std::endl;
    // std::cout << w << std::endl;
    // std::cout << h << std::endl;
    // std::cout << screenwidth << std::endl;
    // std::cout << screenheight << std::endl;

    //pixel_x = x;
    //pixel_width = w;
    //degree_width = c;
    //degree_start = c;

    // also lat-lon to pixel y-x

    ImGui::SetNextWindowPos(ImVec2(x, y), 0, ImVec2(0, 0));
    int FULL_SCREEN_FLAGS = ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs |
                                ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground;
    ImGui::Begin("Map", NULL, FULL_SCREEN_FLAGS);
    ImGui::Image(id, ImVec2(w, h));

    //ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(), ImVec2(100,110), ImGui::ColorConvertFloat4ToU32(ImVec4(1, .15, .15, 1)));
    ImGui::GetWindowDrawList()->AddRectFilled(
      ImVec2(x_patch_min, y_patch_min), 
      ImVec2(x_patch_max, y_patch_max),     
      ImGui::ColorConvertFloat4ToU32(ImVec4(.95, .95, .95, 0.5))
    );
    ImGui::GetWindowDrawList()->AddRect(
      ImVec2(x_patch_min-1, y_patch_min+1), 
      ImVec2(x_patch_max+1, y_patch_max-1),     
      ImGui::ColorConvertFloat4ToU32(ImVec4(.95, .95, .95, 1.0))
    );

    ImGui::End();
  }

  // "National Total" Text
  {
    int x = 0.5 * screenwidth;
    int y = 0.51 * screenheight;
    ImGui::SetNextWindowPos(ImVec2(x, y), 0, ImVec2(0.5, 0));
    ImGui::Begin("DailyAll", NULL, LABEL_FLAGS);
    ImGui::Text("Daily Solar Supply: %.1f MWh (previous 24 hours)", m_daily_all/1000.);
    ImGui::End();
  }

  // "States Total" Texts
  {    
    int x = 0.5 * screenwidth;
    int y = 0.68 * screenheight;
    ImGui::SetNextWindowPos(ImVec2(x, y), 0, ImVec2(0.5, 0));
    ImGui::Begin("DailyStates", NULL, LABEL_FLAGS);
    if (m_daily_state.size() == 9)
    {
      if (m_daily_state[0] > 0) ImGui::Text("New South Wales: %.1f",              m_daily_state[0]/1000.); ImGui::SameLine();
      if (m_daily_state[1] > 0) ImGui::Text("Victoria: %.1f",                     m_daily_state[1]/1000.); 
      if (m_daily_state[2] > 0) ImGui::Text("Queensland: %.1f",                   m_daily_state[2]/1000.); ImGui::SameLine();
      if (m_daily_state[3] > 0) ImGui::Text("South Australia: %.1f",              m_daily_state[3]/1000.);
      if (m_daily_state[4] > 0) ImGui::Text("Western Australia: %.1f",            m_daily_state[4]/1000.); ImGui::SameLine();
      if (m_daily_state[5] > 0) ImGui::Text("Tasmania: %.1f",                     m_daily_state[5]/1000.);
      if (m_daily_state[6] > 0) ImGui::Text("Northern Territory: %.1f",           m_daily_state[6]/1000.); ImGui::SameLine();
      if (m_daily_state[7] > 0) ImGui::Text("Australian Capital Territory: %.1f", m_daily_state[7]/1000.);
      if (m_daily_state[8] > 0) ImGui::Text("Other Territories: %.1f",            m_daily_state[8]/1000.); ImGui::SameLine();
    }
    ImGui::End();
  }

  ImVec2 pos(42.f, 42.f);
  ImVec2 pivot(0, 0);
  ImGui::SetNextWindowPos(pos, 0, pivot);
  ImGui::Begin("Info", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

  bool set = drawLocation();
  resetLocation = 0;

  ImGui::End(); // Window

  if (x_label > 0)
  {
    ImGui::SetNextWindowPos(ImVec2(x_label, y_label), 0, ImVec2(0.5, 1.2));
    ImGui::Begin("LLabele", NULL, LABEL_FLAGS);
    ImGui::Text("%s", m_location.c_str());
    ImGui::End(); // Window


    ImGui::SetNextWindowPos(ImVec2(screenwidth- 199.0, 42.0), 0, ImVec2(0, 0));
    ImGui::Begin("Stats", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::Text("Statistics for %s", m_location.c_str());
    if (dwellings == 0)
      ImGui::Text("Installs: %i / %i", installs, dwellings);
    else
      ImGui::Text("Installs: %i / %i (%i%%)", installs, dwellings, (int)(100.f * (float)installs / (float)dwellings));
    ImGui::Text("Capacity: %.1f (MW)", cap_tot);
    ImGui::Text("<10kW: %.1f", cap_under10);
    ImGui::Text("10-100kW: %.1f", cap_10_100);
    ImGui::Text(">100kW: %.1f", cap_over100);
    if (m_x < 0)
      ImGui::Text("Daily Supply: ??? (MWh)");
    else
      ImGui::Text("Daily Supply: %.1f (MWh)", m_x/1000.);

    ImGui::End(); // Window

  }

  return set;

}


bool MenuInfo::drawLocation()
{
  ImGui::Text("%s", "Location:");
  ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[2]);
  //ImGui::Text("%s", "Brisbane");

  static bool active = false;

  static bool typed = false;
  typed = false;

  //int callbackLambda(ImGui::ImGuiInputTextCallbackData* data)
  static auto callbackLambda = [] (ImGuiInputTextCallbackData* data) -> int
  {
    //std::cout << data->Buf << std::endl;

    // Tab Completion
    if (data->EventKey == ImGuiKey_Tab)
    {
      //active = false;
    }

    // Arrow Up/Down History
    else if (data->EventKey == ImGuiKey_UpArrow)
    {
      // ListSelect +
    }
    else if (data->EventKey == ImGuiKey_DownArrow)
    {
      // ListSelect -
    }

    // Char callback
    if (data->EventChar)
    {
      typed = true;
    }

    return 0;
  };

  if (active)
  {
    int callbackFlag = ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_CallbackCharFilter;
    if (ImGui::InputText( "##inputname",
                          InputBuf,
                          IM_ARRAYSIZE(InputBuf),
                          ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll | callbackFlag,
                          callbackLambda,
                          NULL))
    {
      m_location = std::string(InputBuf);
      resetLocation = 1;
      active = false;
    }
  }
  else
  {
    if (ImGui::Selectable(InputBuf))
    {
      active = true;
    }
  }

  static std::list<std::string> suggestions;
  if (typed)
  {
    suggestions.clear();

    // Combo list
    //if (ImGui::IsItemActive())
    {
      std::string typed(InputBuf);
      if (typed.size() > 0)
      {
        for (auto& i : *m_names)
        {
          if (ifind(i, typed))
          {
            suggestions.push_back(i);
            if (suggestions.size() >= 6) break;
          }
        }
      }
    }
  }

  if (active)
  {
    for (auto& i : suggestions)
    {
      if (ImGui::Selectable(i.c_str()))
      {
        ImGui::SetKeyboardFocusHere(); // Unselect text input
        setSelection(i);
        active = false;
        m_location = i;
        resetLocation = 1;
      }
    }
  }

  ImGui::PopFont();

  if (resetLocation)
  {
    m_x = -1;
  }

  return resetLocation;
}


void MenuInfo::setSelection(std::string& name)
{
  strcpy(InputBuf, name.c_str());
  //strncpy(InputBuf, name.c_str(), name.size());
}
