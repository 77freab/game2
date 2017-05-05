#pragma once

#include <osg/MatrixTransform>

#include "mapBuilder.h"
#include "vehicle.h"

class bombCallback;
class ViewerWidget;

class bomb : public osg::MatrixTransform
{
public:
  bomb(int x, int y, int z, vehicle* parentVehicle, std::vector<osg::ref_ptr<vehicle>>* vehicles,
    std::map<osg::Vec2i, blockType>* typeMap,
    std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>>* tileMap,
    std::list<osg::Node*>* toDelete, ViewerWidget* ViewerWindow);
  void Explode();
private:
  bool destroyTilesAt(int x, int z);
  void destroyVehiclesAt(int fromX, int toX, int fromZ, int toZ);

  std::map<osg::Vec2i, blockType>* _typeMap;
  std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>>* _tileMap;
  std::list<osg::Node*>* _toDelete;
  std::vector<osg::ref_ptr<vehicle>>* _vehicles;
  int _x;
  int _y;
  int _z;
  osg::ref_ptr<bombCallback> _clb;
  ViewerWidget* _ViewerWindow;
  vehicle* _parentVehicle;
};