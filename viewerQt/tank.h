#pragma once

#include "mapBuilder.h"
#include "main.h"
#include <QDeadlineTimer>
#include <QTimer>
#include <QObject>
#include <osgDB/ReadFile>

class tankCallback;

enum class direction
{
  UP = 0,
  LEFT = 1,
  DOWN = 2,
  RIGHT = 3
}; 

class tank : public osg::MatrixTransform
{
public:
  enum class type
  {
    LIGHT = 0,
    HEAVY = 1,
    MOTO = 2
  };

  tank(int x, int z, int playerNum, int controlDevice, std::vector<osg::ref_ptr<tank>>* tank,
    std::map<osg::Vec2i, blockType>* typeMap, std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>>* tileMap, 
    std::list<osg::Node*>* toDelete, ViewerWidget* ViewerWindow);
  void Move();
  void Shoot();
  void Enable();
  void Disable();
  void ChangeType();
  void SetMovingDirection(direction dir);
  void Stop();
  const bool IsEnabled() const;
  const direction CurDir() const;
  const type GetType() const;
  inline const bool NeedToGo() const;
  const int GetControlDevice() const;
  void SetControlDevice(int cd);
  const bool NeedTypeChange() const;
  void SetNeedTypeChange(type newType);
  void SetXCoord(int x);
  void SetZCoord(int z);
  const int GetXCoord() const;
  const int GetZCoord() const;
  const int AddKill();
  inline const int GetPlayerNum() const;

private:
  ViewerWidget* _ViewerWindow;
  bool _needTypeChange = false;
  type _typeForChange;
  type _currentType;
  bool _go = false;
  int _x;
  int _z;
  QDeadlineTimer* _timer;
  int _controlDevice;
  int _killCount = 0;
  bool _enabled = false;
  int _player;
  osg::ref_ptr<tankCallback> _clb;
  std::map<osg::Vec2i, blockType>* _typeMap;
  std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>>* _tileMap;
  std::list<osg::Node*>* _toDelete;
  std::string _texNum;
  osg::ref_ptr<osg::MatrixTransform> _rMt;
  std::vector<osg::ref_ptr<tank>>* _tank;
  direction _goDir = direction::UP;
  direction _curDir = direction::UP;
  std::string _texDir = "UP";
};