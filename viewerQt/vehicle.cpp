#include "vehicle.h"

class VehicleCallback : public osg::NodeCallback
{
public:
  void operator()(osg::Node* nd, osg::NodeVisitor* ndv) override
  {
    Vehicle* veh = static_cast<Vehicle*>(nd);
    if (!delay && veh->NeedToGo()) veh->Move();
    delay = !delay;
    traverse(nd, ndv);
  }
private:
  bool delay = false;
};

Vehicle::Vehicle( int x, 
                  int z, 
                  int speed, 
                  type startType, 
                  int playerNum, 
                  int controlDevice, 
                  int killCount,
                  std::vector<osg::ref_ptr<Vehicle>>& vehicles, 
                  std::vector<std::vector<osg::ref_ptr<Tile>>>& tileMap,
                  std::list<osg::Node*>& toDelete, 
                  ViewerWidget& ViewerWindow) : 
  _shotDelayTimer(new QDeadlineTimer(-1)), 
  _rMt(new MatrixTransform), 
  _speed(speed), 
  _tileMap(tileMap), 
  _toDelete(toDelete), 
  _killCount(killCount),
  _vehicles(vehicles), 
  _controlDevice(controlDevice), 
  _player(playerNum), 
  _x(x), 
  _z(z), 
  _ViewerWindow(ViewerWindow), 
  _currentType(startType)
{
  // additional MatrixTransform for rotating a vehicle
  addChild(_rMt.get());
}

// calculating collisions and moving
void Vehicle::Move()
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

  bool aGo = false, bGo = false, vStop = false;
  const osg::ref_ptr<Tile>& curTile1 = _tileMap[tileCollizionPt1[0]][tileCollizionPt1[1]]; // current tile

  // checking collisions with tiles
  if (curTile1 != nullptr)
  {
    Tile::tileType curTileType = curTile1->GetType();
    if (curTileType == Tile::tileType::ICE)
    {
      aGo = true;
      // ICE !!!!!!!!!!!!!
    }
    if (curTileType == Tile::tileType::BUSHES) aGo = true;
  }
  else
    aGo = true;

  const osg::ref_ptr<Tile>& curTile2 = _tileMap[tileCollizionPt2[0]][tileCollizionPt2[1]];
  if (curTile2 != nullptr)
  {
    Tile::tileType curTileType = curTile2->GetType();
    if (curTileType == Tile::tileType::ICE)
    {
      bGo = true;
      // ICE !!!!!!!!!!!!!
    }
    if (curTileType == Tile::tileType::BUSHES) bGo = true;
  }
  else
    bGo = true;

  // if there is no any collisions with tiles checking collisions with other vehicles
  if (aGo && bGo)
  {
    // cycle on all vehicles
    for (Vehicle* curVehicle : _vehicles)
    {
      if (curVehicle != this && curVehicle->_enabled)
      {
        if (vehicleCollizionPt1[0] > curVehicle->_x - 8 && vehicleCollizionPt1[0] < curVehicle->_x + 8 &&
          vehicleCollizionPt1[1] > curVehicle->_z - 8 && vehicleCollizionPt1[1] < curVehicle->_z + 8)
        {
          vStop = true; // vehicle ahead
        }
        if (vehicleCollizionPt2[0] > curVehicle->_x - 8 && vehicleCollizionPt2[0] < curVehicle->_x + 8 &&
          vehicleCollizionPt2[1] > curVehicle->_z - 8 && vehicleCollizionPt2[1] < curVehicle->_z + 8)
        {
          vStop = true;
        }
      }
    }
    // nothing ahead, moving
    if (!vStop)
    {
      if (_goDir == direction::UP) _z += _speed;
      if (_goDir == direction::DOWN) _z -= _speed;
      if (_goDir == direction::LEFT) _x -= _speed;
      if (_goDir == direction::RIGHT) _x += _speed;
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
      if (_x % 8 >= 4) _x = (_x / 8) * 8 + 8;
      else _x = (_x / 8) * 8;
      // making rotation
      mR.makeRotate(0, osg::Vec3(0, -1, 0));
      _rMt->setMatrix(mR);
      break;
    }
    case(direction::DOWN) :
    {
      if (_x % 8 >= 4) _x = (_x / 8) * 8 + 8;
      else _x = (_x / 8) * 8;
      mR.makeRotate(osg::PI, osg::Vec3(0, -1, 0));
      _rMt->setMatrix(mR);
      break;
    }
    case(direction::LEFT) :
    {
      if (_z % 8 >= 4) _z = (_z / 8) * 8 + 8;
      else _z = (_z / 8) * 8;
      mR.makeRotate(osg::PI_2, osg::Vec3(0, -1, 0));
      _rMt->setMatrix(mR);
      break;
    }
    case(direction::RIGHT) :
    {
      if (_z % 8 >= 4) _z = (_z / 8) * 8 + 8;
      else _z = (_z / 8) * 8;
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

void Vehicle::Enable()
{
  setUpdateCallback(new VehicleCallback);
  _shotDelayTimer->setRemainingTime(SHOT_TIMEOUT);
  _enabled = true;
}

void Vehicle::Disable()
{
  removeUpdateCallback(getUpdateCallback()); // vehicle can not move
  _shotDelayTimer->setRemainingTime(-1); // can not fire
  _enabled = false;
}