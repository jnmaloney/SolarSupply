#pragma once


#include <list>
#include <vector>
#include <string>


class Texture;


class MenuInfo
{

public:

  MenuInfo();
  ~MenuInfo();


  bool draw();

  double m_lon = 0;
  double m_lat = 0;
  double m_temp = 0;
  double m_clouds = 0;
  double m_visibility = 0;
  int m_sunrise = 0;
  int m_sunset = 0;
  double m_x = -1;

  double m_s_x = 1.0;

  double m_a = -1.0;
  double m_b = -1.0;

  float m_daily_all = -1.f;
  std::vector<float> m_daily_state;


  std::string m_location;

  int resetLocation = 0;

  std::list<std::string>* m_names = 0;

  int x_label = 0;
  int y_label = 0;
  int x_patch_min = 0;
  int y_patch_min = 0;
  int x_patch_max = 0;
  int y_patch_max = 0;

  int installs;
  int dwellings;
  float cap_tot;
  float cap_under10;
  float cap_10_100;
  float cap_over100;

protected:

  void setSelection(std::string& name);

  bool drawLocation();

  char InputBuf[256] = "Search...";

  bool m_isSet = false;
};
