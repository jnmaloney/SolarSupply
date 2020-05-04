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


  void draw();

  double m_lon = 0;
  double m_lat = 0;
  double m_temp = 0;
  double m_clouds = 0;
  double m_visibility = 0;
  int m_sunrise = 0;
  int m_sunset = 0;

  std::string m_location;

  int resetLocation = 0;

  std::list<std::string>* m_names = 0;

protected:

  void setSelection(std::string& name);

  void drawLocation();

  char InputBuf[256] = "Search...";

  bool m_isSet = false;
};
