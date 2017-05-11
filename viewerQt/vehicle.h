#pragma once

#include <QDeadlineTimer>

#include <osg/MatrixTransform>

#include "mapBuilder.h"

const int SHOT_TIMEOUT = 300; // delay between shots in ms
const int COLORED_TEXTURES_NUM = 11;

enum class direction
{
  UP = 0,
  LEFT = 1,
  DOWN = 2,
  RIGHT = 3
};

class ViewerWidget;

class VehicleCallback : public osg::NodeCallback
{
public:
  void operator()(osg::Node*, osg::NodeVisitor*) override;
private:
  bool delay = false;
};

class Vehicle : public osg::MatrixTransform
{
public:
  enum class type
  {
    LIGHT = 0,
    HEAVY = 1,
    MOTO = 2
  };

  void Move();
  virtual void Shoot() {}
  void Enable();
  void Disable();

  inline void SetMovingDirection(direction dir);
  inline void Stop();
  inline const bool IsEnabled() const;
  inline const direction GetCurDir() const;
  inline const bool NeedToGo() const;
  inline const int GetControlDevice() const;
  inline void SetControlDevice(int cd);
  inline void SetXCoord(int x);
  inline void SetZCoord(int z);
  inline const int GetXCoord() const;
  inline const int GetZCoord() const;
  inline const int AddKill();
  inline const int GetPlayerNum() const;
  inline const type GetType() const;

protected:
  Vehicle();
  Vehicle(int x, int z, int speed, type startType, int playerNum, int controlDevice,
    std::vector<osg::ref_ptr<Vehicle>>& vehicles,
    std::vector<std::vector<osg::ref_ptr<Tile>>>& tileMap,
    std::list<osg::Node*>& toDelete,
    ViewerWidget& ViewerWindow);

  inline osg::MatrixTransform* getRotateMT() const;
  inline QDeadlineTimer* getShotDelayTimer() const;

  std::vector<std::vector<osg::ref_ptr<Tile>>>& _tileMap;
  std::list<osg::Node*>& _toDelete;
  std::vector<osg::ref_ptr<Vehicle>>& _vehicles;
  ViewerWidget& _ViewerWindow;

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
  osg::ref_ptr<VehicleCallback> _clb;
  osg::ref_ptr<osg::MatrixTransform> _rMt;
  direction _goDir = direction::UP;
  direction _curDir = direction::UP;
};

inline void Vehicle::SetMovingDirection(direction dir)
{
  _go = true;
  _goDir = dir;
}

inline void Vehicle::Stop()
{
  _go = false;
}

inline const bool Vehicle::IsEnabled() const
{
  return _enabled;
}

inline const direction Vehicle::GetCurDir() const
{
  return _curDir;
}

inline const bool Vehicle::NeedToGo() const
{
  return _go;
}

inline const int Vehicle::GetControlDevice() const
{
  return _controlDevice;
}

inline void Vehicle::SetControlDevice(int cd)
{
  _controlDevice = cd;
}

inline void Vehicle::SetXCoord(int x)
{
  _x = x;
}

inline void Vehicle::SetZCoord(int z)
{
  _z = z;
}

inline const int Vehicle::GetXCoord() const
{
  return _x;
}

inline const int Vehicle::GetZCoord() const
{
  return _z;
}

inline const int Vehicle::AddKill()
{
  return ++_killCount;
}

inline const int Vehicle::GetPlayerNum() const
{
  return _player;
}

inline const Vehicle::type Vehicle::GetType() const
{
  return _currentType;
}

inline osg::MatrixTransform* Vehicle::getRotateMT() const
{
  return _rMt.get();
}
inline QDeadlineTimer* Vehicle::getShotDelayTimer() const
{
  return _shotDelayTimer;
}