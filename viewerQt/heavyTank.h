#pragma once

#include "vehicle.h"

class HeavyTank : public Vehicle
{
public:
  HeavyTank(int x, 
            int z, 
            int playerNum, 
            int controlDevice,
            std::vector<osg::ref_ptr<Vehicle>>& vehicles,
            std::vector<std::vector<osg::ref_ptr<Tile>>>& tileMap,
            std::list<osg::Node*>& toDelete,
            ViewerWidget& ViewerWindow, 
            int killCount);

  HeavyTank(const HeavyTank&) = delete;
  HeavyTank& operator=(const HeavyTank&) = delete;

protected:
  virtual ~HeavyTank() {}

private:
  virtual void Shoot() override;
};

