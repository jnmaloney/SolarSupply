#include "WebTile.h"


WebTile::WebTile()
{

}


WebTile::~WebTile()
{

}


std::vector<int>& WebTile::getData()
{
  m_data.clear();
  m_data.resize(256 * 256);
  return m_data;
}


int WebTile::getData(int i, int j)
{
  return m_data[j * 256 + i];
}