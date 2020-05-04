#include "geoutil.h"
#include <math.h>
#include "PngLoader.h"


void deg2num(double lat_deg, double lon_deg, int zoom, int& out_x, int& out_y)
{
  double lat_rad = 0.01745329 * (lat_deg);
  double n = pow(2.0, zoom);
  int xtile = (lon_deg + 180.0) / 360.0 * n;
  int ytile = (1.0 - asinh(tan(lat_rad)) / M_PI) / 2.0 * n;
  out_x = xtile;
  out_y = ytile;
}


void deg2numf(double lat_deg, double lon_deg, int zoom, float& out_x, float& out_y)
{
  double lat_rad = 0.01745329 * (lat_deg);
  double n = pow(2.0, zoom);
  out_x = (lon_deg + 180.0) / 360.0 * n;
  out_y = (1.0 - asinh(tan(lat_rad)) / M_PI) / 2.0 * n;
}


void num2deg(int x, int y, int zoom, double& out_lat_deg, double& out_lon_deg)
{
  double n = pow(2.0, zoom);
  out_lon_deg = x / n * 360.0 - 180.0;
  double lat_rad = atan(sinh(M_PI * (1 - 2 * y / n)));
  out_lat_deg = 180.0 * lat_rad / M_PI;
}


void HeightmapLoadPngRaw_fromMemory(const char* data, uint64_t numBytes, std::vector<int>& a_raster)
{
  // Load Image
  PngLoader loader;
  loader.load_mem(data, numBytes);
  int width = loader.image.width;
  int height = loader.image.height;

  // Create Height Map
  unsigned short* buffer = (unsigned short*)loader.buffer;
  for (int j = 0; j < height; ++j)
  {
    for (int i = 0; i < width; ++i)
    {
      unsigned short R = buffer[i*4 + j * width * 4 + 0];
      unsigned short G = buffer[i*4 + j * width * 4 + 1];
      unsigned short B = buffer[i*4 + j * width * 4 + 2];

      double H = (R * 256. * 256. + G * 256. + B);
      a_raster[i + j*width] = H;
    }
  }
}
