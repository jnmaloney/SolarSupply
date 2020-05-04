#pragma once


#include <map>
#include <string>


class WebTile;
class GeoMeshManager;


class WebTileManager
{
public:
    WebTileManager();
    ~WebTileManager();

    void prepareLocation(double lat, double lon, double c, double d);
  
    void onTileLoaded(int id);

    void initEngine(GeoMeshManager* meshManager);
    void initImageLayer(std::string);
    void initElevationLayer(std::string);

    void prepData(int i, int j);
    // void getImageData(int i, int j, void* data);
    // void getElevData(int i, int j, void* data);
    int getElevationData(int i, int j, int off_x, int off_y);

    void getTileSpan(int& x, int& y, int& h, int& w);

protected:

    void download(
        std::string url, int z, int x, int y,
        void (*onsuccess)(struct emscripten_fetch_t *fetch)
        );

    std::map<int, WebTile*> m_activeImageTiles;
    std::map<int, WebTile*> m_activeElevTiles;

    GeoMeshManager* m_meshManager;

    std::string m_urlImage;
    std::string m_urlElevation;

    static int s_pending;

    int m_span_x = 0;
    int m_span_y = 0;
    int m_tile_y0 = 0;
    int m_tile_x0 = 0;
};