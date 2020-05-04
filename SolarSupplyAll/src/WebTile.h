#pragma once


#include <vector>


// Image tiles are converted to texture 
// Can have raster data saved
class WebTile
{
public:
    WebTile();
    ~WebTile();

    // Access data array
    std::vector<int>& getData();

    // Get a sub-rectangle of the data 
    //void getData(int x, int y, int w, int h, void* data);
    int getData(int i, int j);

    // Get texture
    unsigned int getTexture() { return m_texture; }
    void setTexture(unsigned int id) { m_texture = id; }

protected:
    
    std::vector<int> m_data;
    unsigned int m_texture;
};