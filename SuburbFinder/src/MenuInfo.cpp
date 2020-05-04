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


void MenuInfo::draw()
{
  //ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1, 0.1, 0.1, 0.7));

  ImVec2 pos(42.f, 42.f);
  ImVec2 pivot(0, 0);
  ImGui::SetNextWindowPos(pos, 0, pivot);
  ImGui::Begin("Info", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

  drawLocation();

  ImGui::End(); // Window

}


void MenuInfo::drawLocation()
{
  ImGui::Text("%s", "Location:");
  ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[2]);
  //ImGui::Text("%s", "Brisbane");

  static bool active = false;
  static bool select = false;

  bool typed = false;

  if (active)
  {
    // if (select)
    // {
    //   ImGui::SetKeyboardFocusHere();
    //   select = false;
    // }
    if (ImGui::InputText( "##inputname",
                          InputBuf,
                          IM_ARRAYSIZE(InputBuf),
                          /*ImGuiInputTextFlags_EnterReturnsTrue |*/ ImGuiInputTextFlags_AutoSelectAll,
                          NULL,
                          NULL))
    {
      // m_location = std::string(InputBuf);
      // resetLocation = 1;
      typed = true;
    }

    //active = ImGui::IsItemActive();
    if (ImGui::IsItemActive() && select)
    {
      select = false;
      ImGui::GetInputTextState("##inputname")->SelectAll();
    }
  }
  else
  {
    if (ImGui::Selectable(InputBuf))
    {
      active = true;
      select = true;
    }
  }

  static std::list<std::string> suggestions;
  if (typed)
  {
    suggestions.clear();

    // Combo list
    //if (ImGui::IsItemActive())
    {
      // if (!active)
      // {
      //   ImGui::SetKeyboardFocusHere();
      //   active = true;
      // }

      // ImGui::Selectable("Blah");
      // ImGui::Text("Ooohog");
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
      }
    }
  }

  //ImGui::SetKeyboardFocusHere(-1);
  ImGui::PopFont();
}


void MenuInfo::setSelection(std::string& name)
{
  strcpy(InputBuf, name.c_str());
  //strncpy(InputBuf, name.c_str(), name.size());
}
