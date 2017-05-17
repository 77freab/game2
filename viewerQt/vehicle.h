#pragma once

#include <QDeadlineTimer>

#include <osg/MatrixTransform>

#include "mapBuilder.h"

class ViewerWidget;

class Vehicle : public osg::MatrixTransform
{
public:
  static const int SHOT_TIMEOUT = 300; //!< delay between shots in ms
  static const int COLORED_TEXTURES_NUM = 11;
  enum class Type
  {
    LIGHT = 0,
    HEAVY = 1,
    MOTO = 2
  };
  enum class Direction
  {
    UP = 0,
    LEFT = 1,
    DOWN = 2,
    RIGHT = 3
  };

  void Move();
  virtual void Shoot() {}
  void Enable();
  void Disable();

  inline void SetMovingDirection(Direction dir);
  inline void Stop();
  inline const bool IsEnabled() const;
  inline const Direction GetCurDir() const;
  inline const bool NeedToGo() const;
  inline const int GetControlDevice() const;
  inline void SetControlDevice(int cd);
  inline void SetXCoord(int x);
  inline void SetZCoord(int z);
  inline const int GetXCoord() const;
  inline const int GetZCoord() const;
  inline const int AddKill();
  inline const int GetPlayerNum() const;
  inline const Type GetType() const;
  inline const int GetKillCount() const;
  inline std::vector<std::vector<osg::ref_ptr<Tile>>>& GetTileMap() const;
  inline std::list<osg::Node*>& GetToDeleteList() const;
  inline std::vector<osg::ref_ptr<Vehicle>>& GetVehicleList() const;
  inline ViewerWidget& GetMainWindow() const;

protected:
  Vehicle();
  Vehicle(int x, 
          int z, 
          int speed, 
          Type startType, 
          int playerNum, 
          int controlDevice, 
          int killCount,
          std::vector<osg::ref_ptr<Vehicle>>& vehicles,
          std::vector<std::vector<osg::ref_ptr<Tile>>>& tileMap,
          std::list<osg::Node*>& toDelete,
          ViewerWidget& ViewerWindow);

  Vehicle(const Vehicle&) = delete;
  Vehicle& operator=(const Vehicle&) = delete;

  virtual ~Vehicle();

  inline osg::MatrixTransform* getRotationMatrix() const;
  inline QDeadlineTimer* getShotDelayTimer() const;

private:
  std::vector<std::vector<osg::ref_ptr<Tile>>>& _tileMap;
  std::list<osg::Node*>& _toDelete;
  std::vector<osg::ref_ptr<Vehicle>>& _vehicles;
  ViewerWidget& _ViewerWindow;

  int _speed;
  Type _currentType;
  bool _go;
  int _x;
  int _z;
  QDeadlineTimer* _shotDelayTimer;
  int _controlDevice;
  int _killCount;
  bool _enabled = false;
  int _player;
  //! additional MatrixTransform for rotating a vehicle
  osg::ref_ptr<osg::MatrixTransform> _rotationMatrix;
  Direction _goDir = Direction::UP;
  Direction _curDir = Direction::UP;
};

inline void Vehicle::SetMovingDirection(Direction dir)
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

inline const Vehicle::Direction Vehicle::GetCurDir() const
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

inline const Vehicle::Type Vehicle::GetType() const
{
  return _currentType;
}

inline const int Vehicle::GetKillCount() const
{
  return _killCount;
}

inline std::vector<std::vector<osg::ref_ptr<Tile>>>& Vehicle::GetTileMap() const
{
  return _tileMap;
}

inline std::list<osg::Node*>& Vehicle::GetToDeleteList() const
{
  return _toDelete;
}

inline std::vector<osg::ref_ptr<Vehicle>>& Vehicle::GetVehicleList() const
{
  return _vehicles;
}

inline ViewerWidget& Vehicle::GetMainWindow() const
{
  return _ViewerWindow;
}

inline osg::MatrixTransform* Vehicle::getRotationMatrix() const
{
  return _rotationMatrix.get();
}

inline QDeadlineTimer* Vehicle::getShotDelayTimer() const
{
  return _shotDelayTimer;
}