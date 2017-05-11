#pragma once

#include "vehicle.h"

const int MAX_NUM_BOMBS = 5;

class Motorcycle : public Vehicle
{
public:
  Motorcycle(int x, int z, int playerNum, int controlDevice,
    std::vector<osg::ref_ptr<Vehicle>>& vehicles,
    std::vector<std::vector<osg::ref_ptr<Tile>>>& tileMap,
    std::list<osg::Node*>& toDelete,
    ViewerWidget& ViewerWindow);

  void Shoot() override;
  inline void BombExploded()
  { 
    _numBombs--; 
  }

private:
  int _numBombs = 0;
};