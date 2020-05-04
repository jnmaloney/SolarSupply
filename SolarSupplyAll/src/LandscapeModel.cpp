#include "LandscapeModel.h"
#include "WebTileManager.h"
#include "GeoMeshManager.h"
#include "RenderSystem.h"


LandscapeModel::LandscapeModel()
{

}


LandscapeModel::~LandscapeModel()
{

}


void LandscapeModel::init()
{
  m_geoMeshManager = new GeoMeshManager();
  m_webTileManager = new WebTileManager();

  m_geoMeshManager->initEngine(m_webTileManager);
  m_webTileManager->initEngine(m_geoMeshManager);

  // Default OSM Tiles
  m_webTileManager->initImageLayer("https://a.tile.openstreetmap.org/{z}/{x}/{y}.png");

  // Mapbox Terrain Imagery (Elevation in rgb channels)
  m_webTileManager->initElevationLayer("https://api.mapbox.com/v4/mapbox.terrain-rgb/{z}/{x}/{y}.pngraw?access_token=pk.eyJ1Ijoiam5tYWxvbmV5IiwiYSI6ImNrMXJqM3BjdDAxeHMzaG1naHo1bGRreWUifQ.tEcnjy-7zLUQP-EuncDW0Q");
  
}


void LandscapeModel::setLocation(double a, double b, double c, double d)
{
  m_webTileManager->prepareLocation(a, b, c, d);
}


void LandscapeModel::draw(RenderSystem* rs)
{
  //if (m_webTileManager->readyForUpdate())
  //  m_geoMeshManager->refreshData();TileTextureTileTexture

  m_geoMeshManager->draw();


  // Loop through all meshes
  // for (auto& i : m_index)
  // {
  //   if (i->load < 3) continue;

  //   glm::mat4 mvp(1.0);
  //   mvp = glm::scale(mvp, glm::vec3(0.1, 0.1, 0.01));
  //   mvp = glm::translate(mvp, glm::vec3(-256.0*i->index.x, 256.0*i->index.y, -1.0));
  //   rs->setModelLocal(mvp);

  //   i->texture->bind();
  //   rs->bindMesh(i->mesh);
  //   rs->bindMeshElement(i->mesh, 0);
  //   rs->drawMesh();
  // }
}