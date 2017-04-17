#pragma once

#include "tile.h"
#include <functional>
#include <osg/Geometry>
#include <osg/Geode>

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

class tank;

class projectile : public tile
{
public:
  projectile(int x, int y, int z, direction, 
    std::string texPath, tank* parentTank, tank* enemyTank);
  std::function<void()> moving;
  void move();
private:
  direction _dir;
  osg::Matrix mT;
  osg::Vec2i _collisionPt;
  tank* _parentTank;
  tank* _enemyTank;
  int _x;
  int _z;
  osg::ref_ptr<projectileCallback> _clb;
};

class tank : public osg::MatrixTransform
{
public:
  tank(int x, int z, std::string texPath);
  void moveTo(direction dir);
  void move();
  void stop();
  void shoot();
  bool _go = false;
  osg::ref_ptr<projectile> _projectile = nullptr;
  int _x;
  int _z;
  void setEnemy(tank* enemy);
  tank* _enemyTank;
  osg::ref_ptr<tankCallback> _clb;
private:
  osg::ref_ptr<tile> _dl;
  osg::ref_ptr<tile> _dr;
  osg::ref_ptr<tile> _ul;
  osg::ref_ptr<tile> _ur;
  osg::Vec2i _collisionPt1;
  osg::Vec2i _collisionPt2;
  const int _x0;
  const int _z0;
  direction _goDir = direction::UP;
  direction _curDir = direction::UP;
  std::string _texDir = "UP";
  std::string _texTankType;
  std::string _texChassis = "_C1/";
};