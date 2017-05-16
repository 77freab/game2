#pragma once

#include <osg/MatrixTransform>

class Tile : public osg::MatrixTransform
{
public:
  enum class TileStyle;
  enum class TileType;
  Tile(TileType bt)
    : _type(bt)
  {}
  inline TileType GetType()
  {
    return _type;
  }

  Tile(const Tile&) = delete;
  Tile& operator=(const Tile&) = delete;

protected:
  virtual ~Tile() {}

private:
  TileType _type;
};

enum class Tile::TileStyle
{
  FLAT = 0,
  VOLUMETRIC = 1
};

enum class Tile::TileType
{
  BORDER = 0,
  BRICK = 1,
  ARMOR = 2,
  WATER = 3,
  BUSHES = 4,
  ICE = 5
};