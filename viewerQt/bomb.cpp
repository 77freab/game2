#include <QApplication>
#include <QTimer>
#include <QtMath>

#include <osgDB/ReadFile>

#include "bomb.h"
#include "bang.h"
#include "main.h"

const int BOMB_BANG_DELAY = 100;

class Bomb::BombCallback : public osg::NodeCallback
{
public:
  void operator()(osg::Node* nd, osg::NodeVisitor* ndv) override
  {
    if (_delay == BOMB_BANG_DELAY)
    {
      Bomb* bmb = static_cast<Bomb*>(nd);
      bmb->Explode();
    }
    _delay++;
    traverse(nd, ndv);
  }
private:
  int _delay = 0;
};

// constructor
Bomb::Bomb( int x, 
            int y, 
            int z, 
            Vehicle& parentVehicle,
            std::vector<osg::ref_ptr<Vehicle>>& vehicles,
            std::vector<std::vector<osg::ref_ptr<Tile>>>& tileMap,
            std::list<osg::Node*>& toDelete, 
            ViewerWidget& ViewerWindow) : 
  _x(qFloor(x / 8.) * 8), 
  _y(y), 
  _z(qFloor(z / 8.) * 8),
  _vehicles(vehicles), 
  _tileMap(tileMap),
  _toDelete(toDelete), 
  _ViewerWindow(ViewerWindow),
  _parentVehicle(parentVehicle)
{
  setUpdateCallback(new BombCallback);

  // move bomb to spawn place
  osg::Matrix m;
  m.makeTranslate(_x, _y, _z);
  setMatrix(m); // inherited from MatrixTransform for transition

  // reading model
  osg::ref_ptr<osg::Node> node = osgDB::readNodeFile
    ("./Resources/bomb/Bomb.3ds.90,0,0.rot.40.scale");
  // reading texture
  osg::ref_ptr<osg::Image> image = osgDB::readImageFile
    ("./Resources/bomb/Albedo.png");

  // setting the texture
  osg::StateSet* state = node->getOrCreateStateSet();
  osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
  texture->setImage(image.get());
  state->setTextureAttributeAndModes(0, texture.get());

  addChild(node.get());
}

bool Bomb::destroyTilesAt(int x, int z)
{
  Tile* curTile; // current tile
  bool stop = false;

  // if there is a tile on given coordinates
  if ((curTile = _tileMap[x][z]) != nullptr)
  {
    if (curTile->GetType() == Tile::tileType::BRICK)
    {
      // destroy this tile
      _toDelete.push_back(_tileMap[x][z]);
      _tileMap[x][z] = nullptr;
      stop = true;
    }
    else
      stop = true;
  }

  return stop;
}

void Bomb::destroyVehiclesAt(int fromX, int toX, int fromZ, int toZ)
{
  // cycle by all vehicles
  for (Vehicle* curVehicle : _vehicles)
  {
    int vehicleX = curVehicle->GetXCoord();
    int vehicleZ = curVehicle->GetZCoord();
    if (vehicleX > fromX && vehicleX < toX)
    {
      if (vehicleZ > fromZ && vehicleZ < toZ)
      {
        if (curVehicle->IsEnabled())
        {
          // creating explosion
          Bang* bng = new Bang(vehicleX, -4, vehicleZ, _toDelete);
          getParent(0)->addChild(bng);

          // destroing vehicle
          curVehicle->Disable(); // disabling it
          _toDelete.push_back(curVehicle); // puting it to the queue for deleting from scene

          // increase number of kills (suicide doesn't count)
          if (&_parentVehicle != curVehicle)
          {
            QApplication::postEvent(&_ViewerWindow, new VehicleKilledSomebody
              (_parentVehicle.GetPlayerNum(), _parentVehicle.AddKill()));
          }

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

void Bomb::Explode()
{
  int bombX = _x / 8;
  int bombZ = _z / 8;
  int fromX, toX, fromZ, toZ;
  bool pr1, pr2;

  // destroing first tiles from every direction
  // from above
  for (toZ = bombZ - 1; toZ < bombZ + 5; toZ++)
  {
    // creating explosion
    Bang* bng = new Bang(_x, -3, toZ * 8, _toDelete);
    getParent(0)->addChild(bng);
    // destroing tiles
    pr1 = destroyTilesAt(bombX - 1, toZ);
    pr2 = destroyTilesAt(bombX, toZ);
    if (pr1 || pr2) break;
  }
  // from below
  for (fromZ = bombZ - 2; fromZ > bombZ - 6; fromZ--)
  {
    // creating explosion
    Bang* bng = new Bang(_x, -3, fromZ * 8, _toDelete);
    getParent(0)->addChild(bng);
    // destroing tiles
    pr1 = destroyTilesAt(bombX - 1, fromZ);
    pr2 = destroyTilesAt(bombX, fromZ);
    if (pr1 || pr2) break;
  }
  // destroing vehicles vertically (above and below bomb)
  destroyVehiclesAt(_x - 8, _x + 8, fromZ * 8, toZ * 8);
  // from left
  for (fromX = bombX - 2; fromX > bombX - 6; fromX--)
  {
    // creating explosion
    Bang* bng = new Bang(fromX * 8, -3, _z, _toDelete);
    getParent(0)->addChild(bng);
    // destroing tiles
    pr1 = destroyTilesAt(fromX, bombZ - 1);
    pr2 = destroyTilesAt(fromX, bombZ);
    if (pr1 || pr2) break;
  }
  // from right
  for (toX = bombX + 1; toX < bombX + 5; toX++)
  {
    // creating explosion
    Bang* bng = new Bang(toX * 8, -3, _z, _toDelete);
    getParent(0)->addChild(bng);
    // destroing tiles
    pr1 = destroyTilesAt(toX, bombZ - 1);
    pr2 = destroyTilesAt(toX, bombZ);
    if (pr1 || pr2) break;
  }
  // destroing vehicles horizontally (right and left from bomb)
  destroyVehiclesAt(fromX * 8, toX * 8, _z - 8, _z + 8);

  // destroing bomb
  _toDelete.push_back(this);
  static_cast<Motorcycle*>(&_parentVehicle)->BombExploded();
}