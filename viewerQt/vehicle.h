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
  vehicle();
  vehicle(int x, int z, int speed, type startType, int playerNum, int controlDevice,
    std::vector<osg::ref_ptr<vehicle>>& vehicles,
    std::map<osg::Vec2i, blockType>& typeMap,
    std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>>& tileMap,
    std::list<osg::Node*>& toDelete,
    ViewerWidget& ViewerWindow);

  inline osg::MatrixTransform* getRotateMT() const;
  inline QDeadlineTimer* getShotDelayTimer() const;

  std::map<osg::Vec2i, blockType>* _typeMap;
  std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>>* _tileMap;
  std::list<osg::Node*>* _toDelete;
  std::vector<osg::ref_ptr<vehicle>>* _vehicles;
  ViewerWidget* _ViewerWindow;

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

inline void vehicle::SetMovingDirection(direction dir)
{
  _go = true;
  _goDir = dir;
}

inline void vehicle::Stop()
{
  _go = false;
}

inline const bool vehicle::IsEnabled() const
{
  return _enabled;
}

inline const direction vehicle::GetCurDir() const
{
  return _curDir;
}

inline const bool vehicle::NeedToGo() const
{
  return _go;
}

inline const int vehicle::GetControlDevice() const
{
  return _controlDevice;
}

inline void vehicle::SetControlDevice(int cd)
{
  _controlDevice = cd;
}

inline void vehicle::SetXCoord(int x)
{
  _x = x;
}

inline void vehicle::SetZCoord(int z)
{
  _z = z;
}

inline const int vehicle::GetXCoord() const
{
  return _x;
}

inline const int vehicle::GetZCoord() const
{
  return _z;
}

inline const int vehicle::AddKill()
{
  return ++_killCount;
}

inline const int vehicle::GetPlayerNum() const
{
  return _player;
}

inline const vehicle::type vehicle::GetType() const
{
  return _currentType;
}

inline osg::MatrixTransform* vehicle::getRotateMT() const
{
  return _rMt.get();
}
inline QDeadlineTimer* vehicle::getShotDelayTimer() const
{
  return _shotDelayTimer;
}