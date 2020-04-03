#include "MenuInfo.h"
#include "imgui.h"
#include "WindowManager.h"
#include "Texture.h"
#include "time.h"
#include "Util.h"
#include <iostream>


MenuInfo::MenuInfo()
{}


MenuInfo::~MenuInfo()
{}


bool MenuInfo::draw()
{
  //ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1, 0.1, 0.1, 0.7));

  ImVec2 pos(42.f, 42.f);
  ImVec2 pivot(0, 0);
  ImGui::SetNextWindowPos(pos, 0, pivot);
  ImGui::Begin("Info", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

  bool set = drawLocation();
  resetLocation = 0;

  ImGui::End(); // Window

  if (x_label > 0)
  {
    ImGui::SetNextWindowPos(ImVec2(x_label, y_label), 0, ImVec2(0, 0));
    ImGui::Begin("LLabele", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::Text("%s", m_location.c_str());
    ImGui::End(); // Window


    ImGui::SetNextWindowPos(ImVec2(1024.0 - 199.0, 42.0), 0, ImVec2(0, 0));
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
    ImGui::Text("Daily Supply: ??? (MWh)");

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

  return resetLocation;
}


void MenuInfo::setSelection(std::string& name)
{
  strcpy(InputBuf, name.c_str());
  //strncpy(InputBuf, name.c_str(), name.size());
}
