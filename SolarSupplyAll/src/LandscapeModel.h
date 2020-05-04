#pragma once


class WebTileManager;
class GeoMeshManager;
class RenderSystem;


class LandscapeModel
{
public:
  LandscapeModel();
  ~LandscapeModel();

  void init();

  void setLocation(double lat, double lon, double c, double d);

  void draw(RenderSystem* rs);

protected:

  GeoMeshManager* m_geoMeshManager;
  WebTileManager* m_webTileManager;

};