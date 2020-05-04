#pragma once


#include "graphics.h"
#include "RenderSystem.h"
#include "Texture.h"
#include "HeightMap_Data.h"
#include "Mesh.h"


class GeoScene
{
public:

  GeoScene();
  ~GeoScene();

  void setLocation(double a, double b, double c, double d);

  void clearLocation();

  void draw(RenderSystem* rs);

protected:

  struct Index
  {
    Texture* texture;
    Mesh* mesh;
    glm::vec2 index;
    int load = 0;
  };
  std::vector<Index*> m_index;

  HeightMap_Data m_hmmLevel;

  int m_pendingLoads = 0;

  static GeoScene* Instance;  
};