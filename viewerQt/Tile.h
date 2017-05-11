#pragma once

#include <osg/MatrixTransform>

class Tile : public osg::MatrixTransform
{
public:
  enum class tileStyle;
  enum class tileType;
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

enum class Tile::tileStyle
{
  FLAT = 0,
  VOLUMETRIC = 1
};

enum class Tile::tileType
{
  BORDER = 0,
  BRICK = 1,
  ARMOR = 2,
  WATER = 3,
  BUSHES = 4,
  ICE = 5
};