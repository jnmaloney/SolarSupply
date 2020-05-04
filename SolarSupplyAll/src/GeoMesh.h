
#pragma once


#include "graphics.h"
#include "Mesh.h"


class GeoMesh : public Mesh
{
public: 
    GeoMesh();
    ~GeoMesh();

    void createGeometry(int a_w, int a_h, float* data);

protected:

};


