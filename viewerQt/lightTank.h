#pragma once

#include "vehicle.h"

class lightTank : public vehicle
{
public:
  lightTank(int x, int z, int playerNum, int controlDevice,
    std::vector<osg::ref_ptr<vehicle>>& vehicles,
    std::map<osg::Vec2i, blockType>& typeMap,
    std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>>& tileMap,
    std::list<osg::Node*>& toDelete,
    ViewerWidget& ViewerWindow);

  void Shoot() override;
};

