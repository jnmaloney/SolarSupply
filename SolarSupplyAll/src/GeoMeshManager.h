#pragma once


#include <map>


class WebTileManager;
class GeoMesh;


class GeoMeshManager
{
public: 
    GeoMeshManager();
    ~GeoMeshManager();

    void initEngine(WebTileManager* webTileManager);

    void refreshData();
    void draw() {}


protected:

    std::map<int, GeoMesh*> m_meshes;

    WebTileManager* m_webTileManager;
};
