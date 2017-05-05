#pragma once

#include <QDeadlineTimer>

#include <osg/MatrixTransform>

#include "mapBuilder.h"

const int SHOT_TIMEOUT = 300; // задержка между выстрелами в мс

enum class direction
{
  UP = 0,
  LEFT = 1,
  DOWN = 2,
  RIGHT = 3
};

class ViewerWidget;

class vehicleCallback : public osg::NodeCallback
{
public:
  void operator()(osg::Node*, osg::NodeVisitor*) override;
private:
  bool delay = false;
};

class vehicle : public osg::MatrixTransform
{
public:
  enum class type
  {
    LIGHT = 0,
    HEAVY = 1,
    MOTO = 2
  };

protected:
  vehicle();
  vehicle(int x, int z, int speed, type startType, int playerNum, int controlDevice,
    std::vector<osg::ref_ptr<vehicle>>* vehicles,
    std::map<osg::Vec2i, blockType>* typeMap, 
    std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>>* tileMap,
    std::list<osg::Node*>* toDelete, 
    ViewerWidget* ViewerWindow);

  inline osg::MatrixTransform* getRotateMT() const
  { return _rMt.get(); }
  inline QDeadlineTimer* getShotDelayTimer() const
  { return _shotDelayTimer; }

  std::map<osg::Vec2i, blockType>* _typeMap;
  std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>>* _tileMap;
  std::list<osg::Node*>* _toDelete;
  std::vector<osg::ref_ptr<vehicle>>* _vehicles;
  ViewerWidget* _ViewerWindow;

public:
  void Move();
  virtual void Shoot() {}
  void Enable();
  void Disable();

  inline void SetMovingDirection(direction dir)
  {
    _go = true;
    _goDir = dir;
  }
  inline void Stop()
  { _go = false; }

  inline const bool IsEnabled() const
  { return _enabled; }

  inline const direction GetCurDir() const
  { return _curDir; }

  inline const bool NeedToGo() const
  { return _go; }

  inline const int GetControlDevice() const
  { return _controlDevice; }

  inline void SetControlDevice(int cd)
  { _controlDevice = cd; }

  inline void SetXCoord(int x)
  { _x = x; }

  inline void SetZCoord(int z)
  { _z = z; }

  inline const int GetXCoord() const
  { return _x; }

  inline const int GetZCoord() const
  { return _z; }

  inline const int AddKill()
  { return ++_killCount; }

  inline const int GetPlayerNum() const
  { return _player; }

  inline const type GetType() const
  { return _currentType; }

private:
  int _speed;
  type _currentType;
  bool _go = false;
  int _x;
  int _z;
  QDeadlineTimer* _shotDelayTimer;
  int _controlDevice;
  int _killCount = 0;
  bool _enabled = false;
  int _player;
  osg::ref_ptr<vehicleCallback> _clb;
  osg::ref_ptr<osg::MatrixTransform> _rMt;
  direction _goDir = direction::UP;
  direction _curDir = direction::UP;
};

