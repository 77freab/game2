#pragma once

#include "tile.h"
#include <QDeadlineTimer>
#include <QObject>

class projectile;

enum class direction
{
  UP = 0,
  LEFT = 1,
  DOWN = 2,
  RIGHT = 3
}; 

class tankCallback : public osg::NodeCallback
{
public:
  void operator()(osg::Node*, osg::NodeVisitor*);
private:
  bool delay = false;
};

class tank : public QObject, public osg::MatrixTransform
{
  Q_OBJECT
public:
  enum class type
  {
    LIGHT = 0,
    HEAVY = 1
  };

  tank(int x, int z, std::string texNum, int controlDevice,
    std::vector<osg::ref_ptr<tank>>* tank,
    std::map<osg::Vec2i, blockType>* typeMap,
    std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>>* tileMap,
    std::list<osg::Node*>* toDelete, tileMaker* prjMaker);
  void Move();
  void Shoot();
  void Enable();
  void Disable();
  void ChangeType();
  void SetMovingDirection(direction dir);
  void Stop();
  const bool Enabled() const;
  const direction CurDir() const;
  const type GetType() const;
  inline const bool NeedToGo() const;
  const int GetControlDevice() const;
  void SetControlDevice(int cd);
  const bool NeedTypeChange() const;
  void SetNeedTypeChange();
  void SetXCoord(int x);
  void SetZCoord(int z);
  const int GetXCoord() const;
  const int GetZCoord() const;
  const int AddKill();
signals:
  void smbdyKilled(int killCount);
  void iNeedRespawn(osg::ref_ptr<tank> myself);
private:
  bool _needTypeChange = false;
  type _type;
  bool _go = false;
  osg::ref_ptr<projectile> _projectile;
  int _x;
  int _z;
  QDeadlineTimer* _timer;
  int _controlDevice;
  int _killCount = 0;
  bool _enabled = false;
  osg::ref_ptr<tankCallback> _clb;
  std::map<osg::Vec2i, blockType>* _typeMap;
  std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>>* _tileMap;
  std::list<osg::Node*>* _toDelete;
  std::string _texNum;
  tileMaker* _prjMaker;
  osg::Vec2i _collisionPt1;
  osg::Vec2i _collisionPt2;
  osg::ref_ptr<osg::MatrixTransform> _rMt;
  std::vector<osg::ref_ptr<tank>>* _tank;
  direction _goDir = direction::UP;
  direction _curDir = direction::UP;
  std::string _texDir = "UP";
};