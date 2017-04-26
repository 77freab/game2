#pragma once

#include "tile.h"
#include <functional>
#include <osg/Geometry>
#include <osg/Geode>
#include <QDeadlineTimer> 
#include <QObject> 
#include <osgDB/ReadFile>
#include <QTimer>

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

class bangCallback : public osg::NodeCallback
{
public:
  void operator()(osg::Node*, osg::NodeVisitor*);
};

class bang : public osg::MatrixTransform
{
public:
  bang(int x, int y, int z, std::list<osg::Node*>* toDelete);
  void makeBang();
private:
  osg::ref_ptr<osg::Geode> _geode;
  osg::ref_ptr<osg::Geometry> _geom;
  osg::ref_ptr<osg::Vec4Array> _color;
  osg::ref_ptr<osg::Vec3Array> _vertices;
  osg::ref_ptr<osg::Vec3Array> _normals;
  osg::ref_ptr<osg::Vec2Array> _texCoord;
  osg::ref_ptr<osg::Image> _image;
  osg::ref_ptr<osg::Texture2D> _texture;
  int _roughTimer = 0;
  std::list<osg::Node*>* _toDelete;
  osg::ref_ptr<bangCallback> _clb;
};

class projectile;

class tank : public QObject, public osg::MatrixTransform
{
  Q_OBJECT
public:
  tank(int x, int z, std::string texNum, int joyNum,
    std::vector<osg::ref_ptr<tank>>* tank,
    std::map<osg::Vec2i, blockType>* typeMap,
    std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>>* tileMap,
    std::list<osg::Node*>* toDelete, tile* prjMaker);
  void moveTo(direction dir);
  void move();
  void stop();
  void shoot();
  bool _go = false;
  void enable();
  void disable();
  osg::ref_ptr<projectile> _projectile;
  int _x;
  int _z;
  QDeadlineTimer* _timer;
  int _joyNum;
  int _killCount = 0;
  bool _enabled = false;
signals:
  void smbdyKilled(int killCount);
  void iNeedRespawn(osg::ref_ptr<tank> myself);
private:
  osg::ref_ptr<tankCallback> _clb;
  std::map<osg::Vec2i, blockType>* _typeMap;
  std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>>* _tileMap;
  std::list<osg::Node*>* _toDelete;
  tile* _prjMaker;
  osg::Vec2i _collisionPt1;
  osg::Vec2i _collisionPt2;
  osg::ref_ptr<osg::MatrixTransform> _rMt;
  std::vector<osg::ref_ptr<tank>>* _tank;
  const int _x0;
  const int _z0;
  direction _goDir = direction::UP;
  direction _curDir = direction::UP;
  std::string _texDir = "UP";
};

class projectile : public osg::MatrixTransform
{
public:
  projectile(int x, int y, int z, direction,
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
  int _z;
  osg::ref_ptr<projectileCallback> _clb;
};