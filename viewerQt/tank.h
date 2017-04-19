#pragma once

#include "tile.h"
#include <functional>
#include <osg/Geometry>
#include <osg/Geode>
#include <QDeadlineTimer> 

enum class direction
{
  UP = 0,
  LEFT = 1,
  DOWN = 2,
  RIGHT = 3
}; 

class projectileCallback : public osg::NodeCallback
{
public:
  void operator()(osg::Node*, osg::NodeVisitor*);
};

class tankCallback : public osg::NodeCallback
{
public:
  void operator()(osg::Node*, osg::NodeVisitor*);
};

class projectile;

class tank : public osg::MatrixTransform
{
public:
  tank(int x, int z, std::string texNum, int joyNum,
    std::list<osg::ref_ptr<tank>>* tank,
    std::map<osg::Vec2i, blockType>* typeMap,
    std::map<osg::Vec2i, tile*>* tileMap,
    std::list<osg::Node*>* toDelete);
  void moveTo(direction dir);
  void move();
  void stop();
  void shoot();
  bool _go = false;
  osg::ref_ptr<projectile> _projectile = nullptr;
  int _x;
  int _z;
  osg::ref_ptr<tankCallback> _clb;
  QDeadlineTimer* _timer;
  int _joyNum;
private:
  std::map<osg::Vec2i, blockType>* _typeMap;
  std::map<osg::Vec2i, tile*>* _tileMap;
  std::list<osg::Node*>* _toDelete;
  osg::Vec2i _collisionPt1;
  osg::Vec2i _collisionPt2;
  osg::ref_ptr<osg::MatrixTransform> _rMt;
  std::list<osg::ref_ptr<tank>>* _tank;
  const int _x0;
  const int _z0;
  direction _goDir = direction::UP;
  direction _curDir = direction::UP;
  std::string _texDir = "UP";
};

class projectile : public tile
{
public:
  projectile(int x, int y, int z, direction,
    std::string texPath, tank* parentTank,
    std::list<osg::ref_ptr<tank>>* tank,
    std::map<osg::Vec2i, blockType>* typeMap,
    std::map<osg::Vec2i, tile*>* tileMap,
    std::list<osg::Node*>* toDelete);
  std::function<void()> moving;
  void move();
private:
  std::map<osg::Vec2i, blockType>* _typeMap;
  std::map<osg::Vec2i, tile*>* _tileMap;
  std::list<osg::Node*>* _toDelete;
  direction _dir;
  osg::Matrix mT;
  osg::Vec2i _collisionPt1;
  osg::Vec2i _collisionPt2;
  tank* _parentTank;
  std::list<osg::ref_ptr<tank>>* _tank;
  int _x;
  int _z;
  osg::ref_ptr<projectileCallback> _clb;
};