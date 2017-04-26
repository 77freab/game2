#pragma once

#include <string>
#include <osg/MatrixTransform>
#include <osg/Texture2D>
#include <osg/ref_ptr>
#include <osg/Geode>
#include <QXmlStreamReader>

enum class blockType
{
  BORDER = 0,
  BRICK = 1,
  ARMOR = 2,
  WATER = 3,
  BUSHES = 4,
  ICE = 5,
  PRJ_UP = 6,
  PRJ_DOWN = 7,
  PRJ_LEFT = 8,
  PRJ_RIGHT = 9
};

class tile
{
public:
  tile();
  osg::ref_ptr<osg::MatrixTransform> getTile(int x, int y, int z, blockType bt, bool pr = false);
  osg::Vec2i createMap(osg::ref_ptr<osg::Group> scene, std::map<osg::Vec2i, blockType>& typeMap,
    std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>>& tileMap, QString fileName);
private:
  void skipUnknownElement(QXmlStreamReader& reader);
  osg::ref_ptr<osg::Geode> makeNewTile(blockType bt, bool pr);
  std::map<blockType, osg::ref_ptr<osg::Geode>> _tiles;
  std::map<blockType, std::string> blockTex;
};