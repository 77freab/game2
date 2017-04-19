#include "tile.h"

const int MAP_X_SIZE = 32;
const int MAP_Z_SIZE = 28;

void createMap(osg::ref_ptr<osg::Group> scene, 
  std::map<osg::Vec2i, blockType>& typeMap,
  std::map<osg::Vec2i, tile*>& tileMap)
{
  std::map<blockType, std::string> blockTex = {
    { blockType::BORDER, "blocks/BORDER.png" },
    { blockType::BRICK, "blocks/BRICK.png" },
    { blockType::ARMOR, "blocks/ARMOR.png" },
    { blockType::WATER, "blocks/WATER.png" },
    { blockType::BUSHES, "blocks/BUSHES.png" },
    { blockType::ICE, "blocks/ICE.png" } };

  for (int z = 3; z < 25; z++)
  {
    if (z == 10)
      z = 16;
    typeMap[{4, z}] = blockType::BRICK;
    typeMap[{5, z}] = blockType::BRICK;

    typeMap[{8, z}] = blockType::BRICK;
    typeMap[{9, z}] = blockType::BRICK;

    typeMap[{20, z}] = blockType::BRICK;
    typeMap[{21, z}] = blockType::BRICK;

    typeMap[{24, z}] = blockType::BRICK;
    typeMap[{25, z}] = blockType::BRICK;
  }
  for (int z = 6; z < 25; z++)
  {
    if (z == 12)
      z = 14;
    if (z == 16)
      z = 18;
    typeMap[{12, z}] = blockType::BRICK;
    typeMap[{13, z}] = blockType::BRICK;

    typeMap[{16, z}] = blockType::BRICK;
    typeMap[{17, z}] = blockType::BRICK;
  }
  for (int x = 6; x < 24; x++)
  {
    if (x == 10)
      x = 20;
    typeMap[{x, 12}] = blockType::BRICK;
    typeMap[{x, 13}] = blockType::BRICK;
  }
  typeMap[{2, 13}] = blockType::BRICK;
  typeMap[{3, 13}] = blockType::BRICK;
  typeMap[{2, 12}] = blockType::ARMOR;
  typeMap[{3, 12}] = blockType::ARMOR;

  typeMap[{26, 13}] = blockType::BRICK;
  typeMap[{27, 13}] = blockType::BRICK;
  typeMap[{26, 12}] = blockType::ARMOR;
  typeMap[{27, 12}] = blockType::ARMOR;

  typeMap[{14, 19}] = blockType::ARMOR;
  typeMap[{14, 20}] = blockType::ARMOR;
  typeMap[{15, 19}] = blockType::ARMOR;
  typeMap[{15, 20}] = blockType::ARMOR;

  typeMap[{14, 9}] = blockType::BRICK;
  typeMap[{14, 10}] = blockType::BRICK;
  typeMap[{15, 9}] = blockType::BRICK;
  typeMap[{15, 10}] = blockType::BRICK;

  for (int z = 1; z < 4; z++)
  {
    typeMap[{13, z}] = blockType::BRICK;
    typeMap[{16, z}] = blockType::BRICK;
  }
  typeMap[{14, 3}] = blockType::BRICK;
  typeMap[{15, 3}] = blockType::BRICK;
  
  // border
  for (int x = 2; x < 28; x++)
  {
    typeMap[{x, 0}] = blockType::BORDER; // низ
    typeMap[{x, 27}] = blockType::BORDER; // верх
  }
  for (int z = 0; z < 28; z++)
  {
    typeMap[{0, z}] = blockType::BORDER; // слева
    typeMap[{1, z}] = blockType::BORDER;

    for (int x = 28; x < 32; x++)
      typeMap[{x, z}] = blockType::BORDER; // справа
  }

  std::map<osg::Vec2i, blockType>::const_iterator a;
  for (int x = 0; x < MAP_X_SIZE; x++)
    for (int z = 0; z < MAP_Z_SIZE; z++)
      if ((a = typeMap.find({ x, z })) != typeMap.end())
      {
        tileMap[{ x, z }] = new tile(x * 8, 0, z * 8, blockTex[typeMap[{x, z}]]);
        scene->addChild(tileMap[{ x, z }]);
      }
}