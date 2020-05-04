#include "GeoMeshManager.h"
#include "WebTileManager.h"
#include "GeoMesh.h"


GeoMeshManager::GeoMeshManager()
{

}


GeoMeshManager::~GeoMeshManager()
{

}


void GeoMeshManager::initEngine(WebTileManager* webTileManager)
{
  m_webTileManager = webTileManager;
}


void GeoMeshManager::refreshData()
{
  // How many terrain tiles (at closest level)
  int x0, y0, w, h;
  m_webTileManager->getTileSpan(x0, y0, w, h);

  for (int i = x0; i < x0 + w; ++i)
  {
    for (int j = y0; j < y0 + h; ++j)
    {
      // 5 from dz
      float* elevation = new float[5 * 5];
      GeoMesh* mesh = new GeoMesh();
      //mesh->texture =
      //    m_webTileManager->getTexture(i, j);

      // calculate sub-square of elevation tile
      int z1 = 7;
      int z2 = 13;// 15 ?
      int dz = z2 - z1;
      // 2^6 = 64
      // 256 / 64 = 4
      // Each image tile covers 4 elevation spots

      
      int x1 = i / pow(2, dz);
      int y1 = j / pow(2, dz);
      
      int x2_0 = x1 * pow(2, dz);
      int y2_0 = y1 * pow(2, dz);
      int dx = i - x2_0;
      int dy = j - x2_0;
      int range_x = 4 * dx; // 4 from size 64 into 256
      int range_y = 4 * dy;

      // tile x2, y2 is a sub tile of x1, y1 with pixel offset off_x, off_y
      // Assume at distance 5
      unsigned int* data = new unsigned int[5 * 5];
      //for i0, j0 in (range_x, range_y, 5, 5);
      for (int i0 = 0; i0 < 5; ++i0)
      {
        for (int j0 = 0; j0 < 5; ++j0)
        {
          int H = m_webTileManager->getElevationData(x1, y1, range_x + i0, range_y + j0);
          float h = 0.1e-6 * H;
          int id = 0;
          elevation[id] = h;
        }
      }

      mesh->createGeometry(5, 5, elevation);
      delete[] elevation;
    }
  }
}
