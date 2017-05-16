#pragma once

#include <string>
#include <QXmlStreamReader>
#include <osg/MatrixTransform>
#include "Tile.h"

class MapBuilder
{
public:
  MapBuilder();
  int CreateMap(osg::ref_ptr<osg::Group> scene, 
                std::vector<std::vector<osg::ref_ptr<Tile>>>& tileMap,
                QString fileName, 
                osg::Vec2i& mapSize);
private:
  osg::ref_ptr<Tile> getTile(int x, int y, int z, Tile::TileType bt);
  void skipUnknownElement(QXmlStreamReader& reader);
  osg::ref_ptr<osg::Geode> makeNewTile(Tile::TileType bt, Tile::TileStyle ts);

private:
  //! hold already made tiles of each type
  std::vector<osg::ref_ptr<osg::Geode>> _tiles;
  //! hold paths to textures of tiles
  std::vector<std::string> _blockTex;
};