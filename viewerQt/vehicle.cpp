#include "vehicle.h"

void vehicleCallback::operator()(osg::Node* nd, osg::NodeVisitor* ndv)
{
  vehicle* veh = dynamic_cast<vehicle*>(nd);
  if (!delay)
    if (veh->NeedToGo())
      veh->Move();
  delay = !delay;
  traverse(nd, ndv);
}

vehicle::vehicle(int x, int z, int speed, type startType, int playerNum, int controlDevice,
  std::vector<osg::ref_ptr<vehicle>>* vehicles, 
  std::map<osg::Vec2i, blockType>* typeMap, 
  std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>>* tileMap, 
  std::list<osg::Node*>* toDelete, 
  ViewerWidget* ViewerWindow)
  : _shotDelayTimer(new QDeadlineTimer(-1)), _rMt(new MatrixTransform), _speed(speed),
  _typeMap(typeMap), _tileMap(tileMap), _toDelete(toDelete), _vehicles(vehicles),
  _controlDevice(controlDevice), _player(playerNum), _x(x), _z(z), _ViewerWindow(ViewerWindow),
  _currentType(startType)
{
  // additional MatrixTransform for rotating a vehicle
  addChild(_rMt.get());
}

// calculating collisions and moving
void vehicle::Move()
{
  // calculating collision points depending on current vehicle direction
  osg::Vec2i vehicleCollizionPt1, vehicleCollizionPt2;
  osg::Vec2i tileCollizionPt1, tileCollizionPt2;
  switch (_goDir)
  {
  case(direction::UP) :
  {
    vehicleCollizionPt1 = { _x - 7, _z + 7 };
    vehicleCollizionPt2 = { _x + 7, _z + 7 };
    tileCollizionPt1 = { (_x - 8) / 8, (_z + 8) / 8 };
    tileCollizionPt2 = { static_cast<int>(ceil((_x - 1) / 8.)), (_z + 8) / 8 };
    break;
  }
  case(direction::DOWN) :
  {
    vehicleCollizionPt1 = { _x - 7, _z - 7 };
    vehicleCollizionPt2 = { _x + 7, _z - 7 };
    tileCollizionPt1 = { (_x - 8) / 8, (_z - 8) / 8 };
    tileCollizionPt2 = { static_cast<int>(ceil((_x - 1) / 8.)), (_z - 8) / 8 };
    break;
  }
  case(direction::LEFT) :
  {
    vehicleCollizionPt1 = { _x - 7, _z + 7 };
    vehicleCollizionPt2 = { _x - 7, _z - 7 };
    tileCollizionPt1 = { (_x - 8) / 8, (_z - 8) / 8 };
    tileCollizionPt2 = { (_x - 8) / 8, static_cast<int>(ceil((_z) / 8.)) };
    break;
  }
  case(direction::RIGHT) :
  {
    vehicleCollizionPt1 = { _x + 7, _z + 7 };
    vehicleCollizionPt2 = { _x + 7, _z - 7 };
    tileCollizionPt1 = { (_x + 8) / 8, (_z - 8) / 8 };
    tileCollizionPt2 = { (_x + 8) / 8, static_cast<int>(ceil((_z) / 8.)) };
    break;
  }
  }

  std::map<osg::Vec2i, blockType>::const_iterator a, b;
  bool aGo = false, bGo = false, vStop = false;

  // checking collisions with tiles
  if ((a = _typeMap->find(tileCollizionPt1)) != _typeMap->end())
  {
    if ((*a).second == blockType::ICE)
    {
      aGo = true;
      // ICE !!!!!!!!!!!!!
    }
    if ((*a).second == blockType::BUSHES)
      aGo = true;
  }
  else
    aGo = true;
  if ((b = _typeMap->find(tileCollizionPt2)) != _typeMap->end())
  {
    if ((*b).second == blockType::ICE)
    {
      bGo = true;
      // ICE !!!!!!!!!!!!!
    }
    if ((*b).second == blockType::BUSHES)
      bGo = true;
  }
  else
    bGo = true;

  // if there is no any collisions with tiles checking collisions with other vehicles
  if (aGo && bGo)
  {
    // cycle on all vehicles
    for (auto it = _vehicles->cbegin(); it != _vehicles->end(); ++it)
    {
      if ((*it).get() != this && (*it)->_enabled)
      {
        if (vehicleCollizionPt1[0] > (*it)->_x - 8 && vehicleCollizionPt1[0] < (*it)->_x + 8 &&
          vehicleCollizionPt1[1] > (*it)->_z - 8 && vehicleCollizionPt1[1] < (*it)->_z + 8)
          vStop = true; // vehicle ahead
        if (vehicleCollizionPt2[0] > (*it)->_x - 8 && vehicleCollizionPt2[0] < (*it)->_x + 8 &&
          vehicleCollizionPt2[1] > (*it)->_z - 8 && vehicleCollizionPt2[1] < (*it)->_z + 8)
          vStop = true;
      }
    }
    // nothing ahead, moving
    if (!vStop)
    {
      if (_goDir == direction::UP)
        _z += _speed;
      if (_goDir == direction::DOWN)
        _z -= _speed;
      if (_goDir == direction::LEFT)
        _x -= _speed;
      if (_goDir == direction::RIGHT)
        _x += _speed;
    }
  }

  osg::Matrix mR;

  if (_curDir != _goDir) // if vehicle is turning
  {
    switch (_goDir)
    {
    case(direction::UP) :
    {
      // slightly correcting position of vehicle when turnung to simplify driving near tiles
      if (_x % 8 >= 4)
        _x = (_x / 8) * 8 + 8;
      else
        _x = (_x / 8) * 8;
      // making rotation
      mR.makeRotate(0, osg::Vec3(0, -1, 0));
      _rMt->setMatrix(mR);
      break;
    }
    case(direction::DOWN) :
    {
      if (_x % 8 >= 4)
        _x = (_x / 8) * 8 + 8;
      else
        _x = (_x / 8) * 8;
      mR.makeRotate(osg::PI, osg::Vec3(0, -1, 0));
      _rMt->setMatrix(mR);
      break;
    }
    case(direction::LEFT) :
    {
      if (_z % 8 >= 4)
        _z = (_z / 8) * 8 + 8;
      else
        _z = (_z / 8) * 8;
      mR.makeRotate(osg::PI_2, osg::Vec3(0, -1, 0));
      _rMt->setMatrix(mR);
      break;
    }
    case(direction::RIGHT) :
    {
      if (_z % 8 >= 4)
        _z = (_z / 8) * 8 + 8;
      else
        _z = (_z / 8) * 8;
      mR.makeRotate(-osg::PI_2, osg::Vec3(0, -1, 0));
      _rMt->setMatrix(mR);
      break;
    }
    }
    _curDir = _goDir; // new current direction
  }

  // moving vehicle
  osg::Matrix mT;
  mT.makeTranslate(_x, 0, _z);
  setMatrix(mT);
}

void vehicle::Enable()
{
  _clb = new vehicleCallback;
  setUpdateCallback(_clb);
  _shotDelayTimer->setRemainingTime(SHOT_TIMEOUT);
  _enabled = true;
}

void vehicle::Disable()
{
  removeUpdateCallback(_clb); // vehicle can not move
  _clb = nullptr; // deleting callback
  _shotDelayTimer->setRemainingTime(-1); // can not fire
  _enabled = false;
}