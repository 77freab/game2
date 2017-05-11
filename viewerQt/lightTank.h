#pragma once

#include "vehicle.h"

class LightTank : public Vehicle
{
public:
  LightTank(int x, int z, int playerNum, int controlDevice,
    std::vector<osg::ref_ptr<Vehicle>>& vehicles,
    std::vector<std::vector<osg::ref_ptr<Tile>>>& tileMap,
    std::list<osg::Node*>& toDelete,
    ViewerWidget& ViewerWindow);

  void Shoot() override;
};

