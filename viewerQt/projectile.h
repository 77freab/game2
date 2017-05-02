#pragma once

#include <map>
#include "tank.h"
#include <vector>
#include <osg/ref_ptr>
#include <functional>

class projectileCallback : public osg::NodeCallback
{
public:
  void operator()(osg::Node*, osg::NodeVisitor*);
private:
  bool delay = false;
};

class projectile : public osg::MatrixTransform
{
public:
  projectile(int x, int y, int z, direction dir,
    blockType prjDir, tank* parentTank,
    std::vector<osg::ref_ptr<tank>>* tank,
    std::map<osg::Vec2i, blockType>* typeMap,
    std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>>* tileMap,
    std::list<osg::Node*>* toDelete, tile* prjMaker);
  std::function<void()> moving;
  void move();
private:
  std::map<osg::Vec2i, blockType>* _typeMap;
  std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>>* _tileMap;
  std::list<osg::Node*>* _toDelete;
  direction _dir;
  osg::Matrix mT;
  osg::Vec2i _collisionPt1;
  osg::Vec2i _collisionPt2;
  tank* _parentTank;
  std::vector<osg::ref_ptr<tank>>* _tank;
  int _x;
  int _y;
  int _z;
  osg::ref_ptr<projectileCallback> _clb;
};