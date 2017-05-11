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
    QString fileName, osg::Vec2i& mapSize);
private:
  osg::ref_ptr<Tile> getTile(int x, int y, int z, tileType bt);
  void skipUnknownElement(QXmlStreamReader& reader);
  osg::ref_ptr<osg::Geode> makeNewTile(tileType bt, tileStyle ts);

  std::vector<osg::ref_ptr<osg::Geode>> _tiles;
  std::vector<std::string> _blockTex;
};