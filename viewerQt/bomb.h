#pragma once

#include <osg/MatrixTransform>

#include "mapBuilder.h"
#include "vehicle.h"

class ViewerWidget;

class Bomb : public osg::MatrixTransform
{
public:
  Bomb( int x, 
        int y, 
        int z, 
        Vehicle& parentVehicle, 
        std::vector<osg::ref_ptr<Vehicle>>& vehicles,
        std::vector<std::vector<osg::ref_ptr<Tile>>>& tileMap,
        std::list<osg::Node*>& toDelete, 
        ViewerWidget& ViewerWindow);

  Bomb(const Bomb&) = delete;
  Bomb& operator=(const Bomb&) = delete;

protected:
  virtual ~Bomb() {}

private:
  bool destroyTilesAt(int x, int z);
  void destroyVehiclesAt(int fromX, int toX, int fromZ, int toZ);
  void Explode();
  class BombCallback;
  
private:
  std::vector<std::vector<osg::ref_ptr<Tile>>>& _tileMap;
  std::list<osg::Node*>& _toDelete;
  std::vector<osg::ref_ptr<Vehicle>>& _vehicles;
  int _x;
  int _y;
  int _z;
  ViewerWidget& _ViewerWindow;
  Vehicle& _parentVehicle;
};