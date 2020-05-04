#include <vector>


void deg2num(double lat_deg, double lon_deg, int zoom, int& out_x, int& out_y);
void deg2numf(double lat_deg, double lon_deg, int zoom, float& out_x, float& out_y);

void num2deg(int x, int y, int zoom, double& out_lat_deg, double& out_lon_deg);

void HeightmapLoadPngRaw_fromMemory(const char* data, uint64_t numBytes, std::vector<int>& a_raster);
