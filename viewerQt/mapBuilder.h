#pragma once

#include <string>

#include <QXmlStreamReader>

#include <osg/MatrixTransform>

enum class blockType
{
  BORDER = 0,
  BRICK = 1,
  ARMOR = 2,
  WATER = 3,
  BUSHES = 4,
  ICE = 5
};

class mapBuilder
{
public:
  mapBuilder();
  osg::ref_ptr<osg::MatrixTransform> GetTile(int x, int y, int z, blockType bt, bool pr = false);
  int CreateMap(osg::ref_ptr<osg::Group> scene, 
    std::map<osg::Vec2i, blockType>& typeMap,
    std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>>& tileMap, 
    QString fileName, osg::Vec2i& mapSize);
private:
  void skipUnknownElement(QXmlStreamReader& reader);
  osg::ref_ptr<osg::Geode> makeNewTile(blockType bt, bool pr);

  std::vector<osg::ref_ptr<osg::Geode>> _tiles;
  std::vector<std::string> _blockTex;
};