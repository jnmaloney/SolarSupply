#include "WebTileManager.h"
#include "GeoMeshManager.h"
#include "downloadutil.h"
#include "geoutil.h"
#include "graphics.h"
#include "WebTile.h"
#include "PngLoader.h"


int WebTileManager::s_pending = 0;


WebTileManager::WebTileManager()
{

}


WebTileManager::~WebTileManager()
{

}


void WebTileManager::initEngine(GeoMeshManager* geoMeshManager)
{
    m_meshManager = geoMeshManager;
}


void WebTileManager::initImageLayer(std::string url)
{
    m_urlImage = url;
}


void WebTileManager::initElevationLayer(std::string url)
{
    m_urlElevation = url;
}


void WebTileManager::prepareLocation(double a, double b, double c, double d)
{

  int z;
  int tile_x_min, tile_y_min, tile_x_max, tile_y_max;

  // Image data
  z = 13;
  deg2num(c, a, z, tile_x_min, tile_y_min);
  deg2num(d, b, z, tile_x_max, tile_y_max);
  m_span_x = tile_x_max - tile_x_min;
  m_span_y = tile_y_min - tile_y_max;
  m_tile_x0 = tile_x_min;
  m_tile_y0 = tile_y_max;
  for (int i = tile_x_min; i <= tile_x_max; ++i)
  {
    for (int j = tile_y_max; j <= tile_y_min; ++j)
    {
      // download tile i,j
      download(
        m_urlImage, z, i, j,
        [] (emscripten_fetch_t *fetch) {
          // Store texture data
          int id = (int)fetch->userData;
          WebTile* targetTile = new WebTile();
          //m_activeImageTiles[id] = targetTile;
          // Generate png data
          PngLoader loader;
          loader.load_mem(fetch->data, fetch->numBytes);
          // Generate texture
          GLuint texture = 0;
          //glActiveTexture(GL_TEXTURE1);
          glGenTextures(1, &texture);
          glBindTexture(GL_TEXTURE_2D, texture);

          // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
          // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
          // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
          // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

          glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            loader.x,
            loader.y,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            loader.buffer);
          glGenerateMipmap(GL_TEXTURE_2D);
          glBindTexture(GL_TEXTURE_2D, texture);
          
          // Set texture in webtile
          targetTile->setTexture(texture);

          // Finish callback
          emscripten_fetch_close(fetch);
          --s_pending;
        }
      );
    }
  }

  // Elevation data
  z = 7;  // (variable?)
  deg2num(c, a, z, tile_x_min, tile_y_min);
  deg2num(d, b, z, tile_x_max, tile_y_max);
  
  for (int i = tile_x_min; i <= tile_x_max; ++i)
  {
    for (int j = tile_y_max; j <= tile_y_min; ++j)
    {
      // download tile i,j
      download(
        m_urlElevation, z, i, j,
        [] (emscripten_fetch_t *fetch) {
          // Find place to store it
          int id = (int)fetch->userData;
          WebTile* targetTile = new WebTile();
          //m_activeElevTiles[id] = targetTile;
          // Store raster data
          HeightmapLoadPngRaw_fromMemory(fetch->data, fetch->numBytes, targetTile->getData());
          // Finish callback
          emscripten_fetch_close(fetch);
          --s_pending;
        }
      );
    }
  }
}


void WebTileManager::download(
  std::string url, int z, int x, int y, 
  void (*onsuccess)(struct emscripten_fetch_t *fetch)
  )
{
  int id = y * pow(2, z) + x;
  //m_activeTiles[id] = new WebTile();

  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
  attr.onsuccess = onsuccess;
  attr.onerror = downloadFailed;
  attr.userData = (void*)id;

  url = url.replace(url.find("{x}"), 3, std::to_string(x));
  url = url.replace(url.find("{y}"), 3, std::to_string(y));
  url = url.replace(url.find("{z}"), 3, std::to_string(z));

  emscripten_fetch(&attr, url.c_str());

  ++s_pending;
}


void WebTileManager::getTileSpan(int& x0, int& y0, int& w, int& h)
{
  x0 = m_tile_x0;
  y0 = m_tile_y0;
  w = m_span_x;
  h = m_span_y;
}
    
    
int WebTileManager::getElevationData(int i, int j, int off_x, int off_y)
{
  // find id of i, j
  int z = 7; // can search for highest z?
  int id = j * pow(2, z) + i;

  WebTile* tile = m_activeElevTiles[id];

  // return data point offset
  return tile->getData(off_x, off_y);
}
