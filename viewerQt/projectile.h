#pragma once

#include <functional>

#include <osg/MatrixTransform>

#include "mapBuilder.h"
#include "vehicle.h"

class projectileCallback;

class projectile : public osg::MatrixTransform
{
public:
  projectile(int x, int y, int z, int speed, direction dir, vehicle* parentVehicle,
    std::vector<osg::ref_ptr<vehicle>>* vehicles, std::map<osg::Vec2i, blockType>* typeMap,
    std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>>* tileMap,
    std::list<osg::Node*>* toDelete, ViewerWidget* ViewerWindow);
  void TryToMove();
private:
  std::function<void()> moving;
  std::map<osg::Vec2i, blockType>* _typeMap;
  std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>>* _tileMap;
  std::list<osg::Node*>* _toDelete;
  direction _dir;
  osg::Matrix _mT;
  vehicle* _parentVehicle;
  std::vector<osg::ref_ptr<vehicle>>* _vehicles;
  osg::Vec2i _tileCollizionPt1;
  osg::Vec2i _tileCollizionPt2;
  int _x;
  int _y;
  int _z;
  int _speed; // light 4, heavy 2
  osg::ref_ptr<projectileCallback> _clb;
  ViewerWidget* _ViewerWindow;
  osg::ref_ptr<osg::Node> _model;
  osg::ref_ptr<osg::Image> _textureImage;
};