#include <QApplication>
#include <QTimer>

#include <osgDB/ReadFile>

#include "bang.h"
#include "main.h"
#include "projectile.h"

class Projectile::ProjectileCallback : public osg::NodeCallback
{
public:
  void operator()(osg::Node* nd, osg::NodeVisitor* ndv) override
  {
    Projectile* prj = static_cast<Projectile*>(nd);
    if (!delay) prj->TryToMove();
    delay = !delay;
    traverse(nd, ndv);
  }
private:
  bool delay = false;
};

// construcor
Projectile::Projectile( int x, 
                        int y, 
                        int z, 
                        int speed, 
                        direction dir, 
                        Vehicle& parentVehicle,
                        std::vector<osg::ref_ptr<Vehicle>>& vehicles,
                        std::vector<std::vector<osg::ref_ptr<Tile>>>& tileMap,
                        std::list<osg::Node*>& toDelete, 
                        ViewerWidget& ViewerWindow) : 
  _dir(dir), 
  _x(x), 
  _y(y), 
  _z(z), 
  _speed(speed), 
  _parentVehicle(parentVehicle), 
  _vehicles(vehicles), 
  _tileMap(tileMap),
  _toDelete(toDelete), 
  _ViewerWindow(ViewerWindow)
{
  setDataVariance(osg::Object::DYNAMIC);
  setUpdateCallback(new ProjectileCallback);

  // move projectile to spawn place
  setMatrix(osg::Matrix::translate(_x, _y, _z));
  
  // reading texture
  osg::ref_ptr<osg::Image> textureImage = osgDB::readImageFile
    ("./Resources/projectile/Bullet_U.bmp");
  osg::ref_ptr<osg::Node> model;

  switch (_dir)
  {
    case(direction::UP) :
    {
      // rotating the model to appropriate direction
      model = osgDB::readNodeFile
        ("./Resources/projectile/Bullet.3ds.-90,0,0.rot.4,-4,4.trans");
      // calculating initial values for collision points
      _tileCollizionPt1 = { (_x + 2) / 8, (_z + 8) / 8 };
      _tileCollizionPt2 = { (_x + 6) / 8, (_z + 8) / 8 };
      // creating function that called in the callback to move the projectile
      moving = [this]
      {
        _z += _speed;
        _tileCollizionPt1[1] = (_z + 8) / 8;
        _tileCollizionPt2[1] = (_z + 8) / 8;
        setMatrix(osg::Matrix::translate(_x, _y, _z));
      };
      break;
    }
    // same for each direction
    case(direction::DOWN) :
    {
      model = osgDB::readNodeFile
        ("./Resources/projectile/Bullet.3ds.90,0,0.rot.4,4,4.trans");
      _tileCollizionPt1 = { (_x + 2) / 8, (_z) / 8 };
      _tileCollizionPt2 = { (_x + 6) / 8, (_z) / 8 };
      moving = [this]
      {
        _z -= _speed;
        _tileCollizionPt1[1] = (_z) / 8;
        _tileCollizionPt2[1] = (_z) / 8;
        setMatrix(osg::Matrix::translate(_x, _y, _z));
      };
      break;
    }
    case(direction::LEFT) :
    {
      model = osgDB::readNodeFile
        ("./Resources/projectile/Bullet.3ds.0,0,-90.rot.4,0,0.trans");
      _tileCollizionPt1 = { (_x) / 8, (_z + 2) / 8 };
      _tileCollizionPt2 = { (_x) / 8, (_z + 6) / 8 };
      moving = [this]
      {
        _x -= _speed;
        _tileCollizionPt1[0] = (_x) / 8;
        _tileCollizionPt2[0] = (_x) / 8;
        setMatrix(osg::Matrix::translate(_x, _y, _z));
      };
      break;
    }
    case(direction::RIGHT) :
    {
      model = osgDB::readNodeFile
        ("./Resources/projectile/Bullet.3ds.0,0,90.rot.4,0,0.trans");
      _tileCollizionPt1 = { (_x + 8) / 8, (_z + 2) / 8 };
      _tileCollizionPt2 = { (_x + 8) / 8, (_z + 6) / 8 };
      moving = [this]
      {
        _x += _speed;
        _tileCollizionPt1[0] = (_x + 8) / 8;
        _tileCollizionPt2[0] = (_x + 8) / 8;
        setMatrix(osg::Matrix::translate(_x, _y, _z));
      };
      break;
    }
  }

  // setting texture
  osg::StateSet* state = model->getOrCreateStateSet();
  osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
  texture->setImage(textureImage.get());
  state->setTextureAttributeAndModes(0, texture.get());

  addChild(model.get());
}

// calculating collisions
void Projectile::TryToMove()
{
  bool aGo = false, bGo = false, projDel = false;
  osg::ref_ptr<Tile>& curTile1 = _tileMap[_tileCollizionPt1[0]][_tileCollizionPt1[1]]; // current tile

  if (curTile1 != nullptr)
  {
    Tile::TileType curTileType = curTile1->GetType();
    if (curTileType == Tile::TileType::BRICK)
    {
      // destroying tile
      _toDelete.push_back(curTile1);
      curTile1 = nullptr;
      // need to destroy this projectile
      projDel = true;
    }
    else if ((curTileType == Tile::TileType::ARMOR) || (curTileType == Tile::TileType::BORDER))
    {
      // need to destroy this projectile
      projDel = true;
    }
    else
      aGo = true;
  }
  else
    aGo = true;

  osg::ref_ptr<Tile>& curTile2 = _tileMap[_tileCollizionPt2[0]][_tileCollizionPt2[1]];
  if (curTile2 != nullptr)
  {
    Tile::TileType curTileType = curTile2->GetType();
    if (curTileType == Tile::TileType::BRICK)
    {
      // destroying tile
      _toDelete.push_back(curTile2);
      curTile2 = nullptr;
      // need to destroy this projectile
      projDel = true;
    }
    else if ((curTileType == Tile::TileType::ARMOR) || (curTileType == Tile::TileType::BORDER))
    {
      // need to destroy this projectile
      projDel = true;
    }
    else
      bGo = true;
  }
  else
    bGo = true;

  if (!projDel)
  {
    for (Vehicle* curVehicle : _vehicles)
    {
      int vehicleX = curVehicle->GetXCoord();
      int vehicleZ = curVehicle->GetZCoord();

      if (curVehicle != &_parentVehicle && curVehicle->IsEnabled())
      {
        if (_z + 6 >= vehicleZ - 8 && _z + 2 <= vehicleZ + 8)
        {
          if (_x + 6 >= vehicleX - 8 && _x + 2 <= vehicleX + 8)
            // projectile hit the vehicle
          {
            // need to destroy this projectile
            projDel = true;
            // if we hit front armor of heavy tank projectile can't pierce it
            if (!(curVehicle->GetType() == Vehicle::Type::HEAVY &&
              abs(static_cast<int>(curVehicle->GetCurDir()) - static_cast<int>(_dir)) == 2))
            {
              // creating explosion
              Bang* bng = new Bang(vehicleX, 4, vehicleZ, _toDelete);

              _parentVehicle.getParent(0)->addChild(bng);

              // destroying vehicle
              curVehicle->Disable(); // disabe it
              _toDelete.push_back(curVehicle); // puting it to the queue for deleting from scene

              // increase number of kills
              QApplication::postEvent(&_ViewerWindow, new VehicleKilledSomebody
                (_parentVehicle.GetPlayerNum(), _parentVehicle.AddKill()));

              // after pause enemy will respawn
              ViewerWidget* vw = &_ViewerWindow;
              QTimer::singleShot(3000, vw, [curVehicle, vw]
              {
                QApplication::postEvent(vw, new VehicleNeedRespawn(curVehicle));
              });
            }
          }
        }
      }
    }
  }
  // destroying this projectile
  if (projDel) _toDelete.push_back(this);
  // if projectile still alive - moving
  if (aGo && bGo && !projDel) moving();
}