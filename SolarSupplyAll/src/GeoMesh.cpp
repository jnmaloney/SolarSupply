#include "GeoMesh.h"


GeoMesh::GeoMesh()
{

}


GeoMesh::~GeoMesh()
{

}


void GeoMesh::createGeometry(int a_w, int a_h, float* data)
{
  // Fill height map into data
  m_vertP.resize(a_w * a_h);
  m_vertT.resize(a_w * a_h);
  m_vertN.resize(a_w * a_h);
  for (int j = 0; j < a_h; ++j)
  {
    for (int i = 0; i < a_w; ++i)
    {
      //double H2 = data[(a_i+i) + (a_j+j)*256]; // <-- special data number 256
      double elevation = data[i + j*a_w];
      glm::vec4 p;
      p.x = i;
      p.y = j;
      p.z = elevation;
      p.w = 1.0;
      m_vertP[i + j*a_w] = p;

      glm::vec2 t;
      t.x = (float)i / (float)(a_w - 1);
      t.y = (float)j / (float)(a_h - 1);
      m_vertT[i + j*a_w] = t;

      glm::vec3 n;
      n.x = 0;
      n.y = 0;
      n.z = 1;
      m_vertN[i + j*a_w] = n;
    }
  }

  std::vector<GLushort>  list1;
  for (int j = 0; j < a_h - 1; ++j)
  {
    for (int i = 0; i < a_w - 1; ++i)
    {
      int t0 = i + (j+0) * a_w;
      int t1 = i + (j+1) * a_w;

      list1.push_back(t0);
      list1.push_back(t1);
      list1.push_back(t0+1);

      list1.push_back(t0+1);
      list1.push_back(t1);
      list1.push_back(t1+1);
    }
  }
  m_matElements["list1"] = list1;

  // Create the mesh from data
  upload();
}
