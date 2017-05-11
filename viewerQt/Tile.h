#pragma once

#include <osg/MatrixTransform>

enum class tileStyle
{
  FLAT = 0,
  VOLUMETRIC = 1
};

enum class tileType
{
  BORDER = 0,
  BRICK = 1,
  ARMOR = 2,
  WATER = 3,
  BUSHES = 4,
  ICE = 5
};

class Tile : public osg::MatrixTransform
{
public:
  Tile(tileType bt)
    : _type(bt)
  {}
  inline tileType GetType()
  {
    return _type;
  }

private:
  tileType _type;
};