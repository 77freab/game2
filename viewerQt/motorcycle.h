#pragma once

#include "vehicle.h"

const int MAX_NUM_BOMBS = 5;

class motorcycle : public vehicle
{
public:
  motorcycle(int x, int z, int playerNum, int controlDevice,
    std::vector<osg::ref_ptr<vehicle>>* vehicles,
    std::map<osg::Vec2i, blockType>* typeMap,
    std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>>* tileMap,
    std::list<osg::Node*>* toDelete,
    ViewerWidget* ViewerWindow);

  void Shoot() override;
  inline void BombExploded()
  { _numBombs--; }

private:
  int _numBombs;
};