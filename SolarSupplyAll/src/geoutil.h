#include <math.h>


void deg2num(double lat_deg, double lon_deg, int zoom, int& out_x, int& out_y)
{
  double lat_rad = 0.01745329 * (lat_deg);
  double n = pow(2.0, zoom);
  int xtile = (lon_deg + 180.0) / 360.0 * n;
  int ytile = (1.0 - asinh(tan(lat_rad)) / M_PI) / 2.0 * n;
  out_x = xtile;
  out_y = ytile;
}


void num2deg(int x, int y, int zoom, double& out_lat_deg, double& out_lon_deg)
{
  double n = pow(2.0, zoom);
  out_lon_deg = x / n * 360.0 - 180.0;
  double lat_rad = atan(sinh(M_PI * (1 - 2 * y / n)));
  out_lat_deg = 180.0 * lat_rad / M_PI;
}
