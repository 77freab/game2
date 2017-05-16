#pragma once

#include "vehicle.h"

class Motorcycle : public Vehicle
{
public:
  Motorcycle( int x, 
              int z, 
              int playerNum, 
              int controlDevice,
              std::vector<osg::ref_ptr<Vehicle>>& vehicles,
              std::vector<std::vector<osg::ref_ptr<Tile>>>& tileMap,
              std::list<osg::Node*>& toDelete,
              ViewerWidget& ViewerWindow, 
              int killCount);

  Motorcycle(const Motorcycle&) = delete;
  Motorcycle& operator=(const Motorcycle&) = delete;

  inline void BombExploded()
  { 
    _numBombs--; 
  }

protected:
  virtual ~Motorcycle() {}

private:
  static const int MAX_NUM_BOMBS = 5;
  int _numBombs = 0;

  virtual void Shoot() override;
};