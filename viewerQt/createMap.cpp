#include "tile.h"
#include <createMap.h>

const int MAP_X_SIZE = 32;
const int MAP_Z_SIZE = 28;

std::map<osg::Vec2i, blockType> map;
std::map<osg::Vec2i, tile*> tileMap;

void createMap(osg::ref_ptr<osg::Group> scene)
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
    map[{4, z}] = blockType::BRICK;
    map[{5, z}] = blockType::BRICK;

    map[{8, z}] = blockType::BRICK;
    map[{9, z}] = blockType::BRICK;

    map[{20, z}] = blockType::BRICK;
    map[{21, z}] = blockType::BRICK;

    map[{24, z}] = blockType::BRICK;
    map[{25, z}] = blockType::BRICK;
  }
  for (int z = 6; z < 25; z++)
  {
    if (z == 12)
      z = 14;
    if (z == 16)
      z = 18;
    map[{12, z}] = blockType::BRICK;
    map[{13, z}] = blockType::BRICK;

    map[{16, z}] = blockType::BRICK;
    map[{17, z}] = blockType::BRICK;
  }
  for (int x = 6; x < 24; x++)
  {
    if (x == 10)
      x = 20;
    map[{x, 12}] = blockType::BRICK;
    map[{x, 13}] = blockType::BRICK;
  }
  map[{2, 13}] = blockType::BRICK;
  map[{3, 13}] = blockType::BRICK;
  map[{2, 12}] = blockType::ARMOR;
  map[{3, 12}] = blockType::ARMOR;

  map[{26, 13}] = blockType::BRICK;
  map[{27, 13}] = blockType::BRICK;
  map[{26, 12}] = blockType::ARMOR;
  map[{27, 12}] = blockType::ARMOR;

  map[{14, 19}] = blockType::ARMOR;
  map[{14, 20}] = blockType::ARMOR;
  map[{15, 19}] = blockType::ARMOR;
  map[{15, 20}] = blockType::ARMOR;

  map[{14, 9}] = blockType::BRICK;
  map[{14, 10}] = blockType::BRICK;
  map[{15, 9}] = blockType::BRICK;
  map[{15, 10}] = blockType::BRICK;

  for (int z = 1; z < 4; z++)
  {
    map[{13, z}] = blockType::BRICK;
    map[{16, z}] = blockType::BRICK;
  }
  map[{14, 3}] = blockType::BRICK;
  map[{15, 3}] = blockType::BRICK;
  
  // border
  for (int x = 2; x < 28; x++)
  {
    map[{x, 0}] = blockType::BORDER; // низ
    map[{x, 27}] = blockType::BORDER; // верх
  }
  for (int z = 0; z < 28; z++)
  {
    map[{0, z}] = blockType::BORDER; // слева
    map[{1, z}] = blockType::BORDER;

    for (int x = 28; x < 32; x++)
      map[{x, z}] = blockType::BORDER; // справа
  }

  std::map<osg::Vec2i, blockType>::const_iterator a;
  for (int x = 0; x < MAP_X_SIZE; x++)
    for (int z = 0; z < MAP_Z_SIZE; z++)
      if ((a = map.find({ x, z })) != map.end())
      {
        tileMap[{ x, z }] = new tile(x * 8, 0, z * 8, blockTex[map[{x, z}]]);
        //tileMap[{ x, z }]->setDataVariance(osg::Object::STATIC);
        //if (((*a).second == blockType::BORDER) || ((*a).second == blockType::ARMOR))
        //  tileMap[{ x, z }]->setCullingActive(false);
        scene->addChild(tileMap[{ x, z }]);
      }
}