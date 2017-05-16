#pragma once

#include <functional>

#include <osg/MatrixTransform>

#include "mapBuilder.h"
#include "vehicle.h"

class Projectile : public osg::MatrixTransform
{
  using direction = Vehicle::direction;
public:
  Projectile( int x, 
              int y, 
              int z, 
              int speed, 
              direction dir, 
              Vehicle& parentVehicle,
              std::vector<osg::ref_ptr<Vehicle>>& vehicles,
              std::vector<std::vector<osg::ref_ptr<Tile>>>& tileMap,
              std::list<osg::Node*>& toDelete, 
              ViewerWidget& ViewerWindow);

  Projectile(const Projectile&) = delete;
  Projectile& operator=(const Projectile&) = delete;

protected:
  virtual ~Projectile() {}

private:
  std::function<void()> moving;
  std::vector<std::vector<osg::ref_ptr<Tile>>>& _tileMap;
  std::list<osg::Node*>& _toDelete;
  direction _dir;
  Vehicle& _parentVehicle;
  std::vector<osg::ref_ptr<Vehicle>>& _vehicles;
  osg::Vec2i _tileCollizionPt1;
  osg::Vec2i _tileCollizionPt2;
  int _x;
  int _y;
  int _z;
  int _speed; // light 4, heavy 2
  ViewerWidget& _ViewerWindow;

  void TryToMove();
  class ProjectileCallback;
};