#include "GeoScene.h"
#include "graphics.h"
#include "downloadutil.h"
#include "HeightMap_MeshTile.h"
#include "geoutil.h"
#include "Texture.h"
#include <iostream>


GeoScene* GeoScene::Instance = 0;


GeoScene::GeoScene()
{
  if (Instance == 0) Instance = this;
}


GeoScene::~GeoScene()
{

}


void GeoScene::setLocation(double a, double b, double c, double d)
{
  clearLocation();

  int z = 7;
  int tile_x_min, tile_y_min, tile_x_max, tile_y_max;

  deg2num(c, a, z, tile_x_min, tile_y_min);
  deg2num(d, b, z, tile_x_max, tile_y_max);

  //std::cout << tile_x_min << tile_x_max << tile_y_max << tile_y_min << std::endl;

  // Load heightmaps
  for (int i = tile_x_min; i <= tile_x_max; ++i)
  {
    for (int j = tile_y_max; j <= tile_y_min; ++j)
    {
      // Create struct
      Index* inst = new Index;
      m_index.push_back(inst);
      inst->index.x = i - tile_x_min;
      inst->index.y = j - tile_y_max;

      {
        // download tile i,j
      
        emscripten_fetch_attr_t attr;
        emscripten_fetch_attr_init(&attr);
        strcpy(attr.requestMethod, "GET");
        attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
        attr.onsuccess = [] (emscripten_fetch_t *fetch) {
          Index* i = (Index*)fetch->userData;
          Instance->m_hmmLevel.HloadPngRaw_fromMemory(fetch->data, fetch->numBytes);
          emscripten_fetch_close(fetch);
          i->mesh = Instance->m_hmmLevel.createTile(0, 0, 256, 256); // Set pointer
            
          i->load += 1;
          //std::cout << "geomesh load" << std::endl;
        };
        attr.onerror = downloadFailed;
        attr.userData = (void*)inst;

        int x = i;
        int y = j;

        std::string url = std::string("https://api.mapbox.com/v4/mapbox.terrain-rgb/{z}/{x}/{y}.pngraw?access_token=pk.eyJ1Ijoiam5tYWxvbmV5IiwiYSI6ImNrMXJqM3BjdDAxeHMzaG1naHo1bGRreWUifQ.tEcnjy-7zLUQP-EuncDW0Q");
        url = url.replace(url.find("{x}"), 3, std::to_string(x));
        url = url.replace(url.find("{y}"), 3, std::to_string(y));
        url = url.replace(url.find("{z}"), 3, std::to_string(z));
        emscripten_fetch(&attr, url.c_str());
      }
        
  //   }
  // }

  // //z = 7; // 13

  // // Load image maps
  // for (int i = tile_x_min; i <= tile_x_max; ++i)
  // {
  //   for (int j = tile_y_max; j <= tile_y_min; ++j)
  //   {
      {
        inst->texture = new Texture;

        emscripten_fetch_attr_t attr;
        emscripten_fetch_attr_init(&attr);
        strcpy(attr.requestMethod, "GET");
        attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
        attr.onsuccess = [] (emscripten_fetch_t *fetch) 
          {
            Index*i = (Index*)fetch->userData;
            Texture* t = i->texture;
            t->loadPng_fromMemory(fetch->data, fetch->numBytes);
        
            emscripten_fetch_close(fetch); // Free data associated with the fetch.
            i->load += 2;
            //std::cout << "texture load" << std::endl;
          };
        attr.onerror = downloadFailed;
        attr.userData = (void*)inst;

        int x = i;// - 2;//3;
        int y = j;// - 1;

        // key =  solar-870093db
        //std::string url = std::string("https://maps.omniscale.net/v2/solar-870093db/style.outdoor/{z}/{x}/{y}.png");
        //std::string url = std::string("https://maps.omniscale.net/v2/opensolar-public-aa5ae3b0/style.outdoor/{z}/{x}/{y}.png");
        std::string url = std::string("https://a.tile.openstreetmap.org/{z}/{x}/{y}.png");
        url = url.replace(url.find("{x}"), 3, std::to_string(x));
        url = url.replace(url.find("{y}"), 3, std::to_string(y));
        url = url.replace(url.find("{z}"), 3, std::to_string(z));
        //printf("%s\n", url.c_str());

        emscripten_fetch(&attr, url.c_str());
      }
    }
  }
}


void GeoScene::clearLocation()
{
  for (auto& i : m_index)
  {
    delete i->mesh;
    delete i->texture;
    delete i;
  }
  m_index.clear();
}


void GeoScene::draw(RenderSystem* rs)
{
  // Loop through all meshes
  for (auto& i : m_index)
  {
    if (i->load < 3) continue;

    glm::mat4 mvp(1.0);
    mvp = glm::scale(mvp, glm::vec3(0.1, 0.1, 0.01));
    mvp = glm::translate(mvp, glm::vec3(-256.0*i->index.x, 256.0*i->index.y, -1.0));
    rs->setModelLocal(mvp);

    i->texture->bind();
    rs->bindMesh(i->mesh);
    rs->bindMeshElement(i->mesh, 0);
    rs->drawMesh();
  }
}