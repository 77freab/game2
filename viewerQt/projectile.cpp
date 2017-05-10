#include <QApplication>
#include <QTimer>

#include <osgDB/ReadFile>

#include "bang.h"
#include "main.h"
#include "projectile.h"

class projectileCallback : public osg::NodeCallback
{
public:
  void operator()(osg::Node* nd, osg::NodeVisitor* ndv) override;
private:
  bool delay = false;
};

void projectileCallback::operator()(osg::Node* nd, osg::NodeVisitor* ndv)
{
  projectile* prj = dynamic_cast<projectile*>(nd);
  if (!delay)
    prj->TryToMove();
  delay = !delay;
  traverse(nd, ndv);
}

// construcor
projectile::projectile(int x, int y, int z, int speed, direction dir, vehicle& parentVehicle,
  std::vector<osg::ref_ptr<vehicle>>& vehicles, std::map<osg::Vec2i, blockType>& typeMap,
  std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>>& tileMap,
  std::list<osg::Node*>& toDelete, ViewerWidget& ViewerWindow)
  : _dir(dir), _x(x), _y(y), _z(z), _speed(speed), _clb(new projectileCallback),
  _parentVehicle(&parentVehicle), _vehicles(&vehicles), _typeMap(&typeMap), _tileMap(&tileMap),
  _toDelete(&toDelete), _ViewerWindow(&ViewerWindow)
{
  setDataVariance(osg::Object::DYNAMIC);
  setUpdateCallback(_clb);

  // move projectile to spawn place
  osg::Matrix m;
  m.makeTranslate(_x, _y, _z);
  setMatrix(m); // inherited from MatrixTransform for transition
  
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
        _mT.makeTranslate(_x, _y, _z);
        setMatrix(_mT);
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
        _mT.makeTranslate(_x, _y, _z);
        setMatrix(_mT);
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
        _mT.makeTranslate(_x, _y, _z);
        setMatrix(_mT);
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
        _mT.makeTranslate(_x, _y, _z);
        setMatrix(_mT);
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
void projectile::TryToMove()
{
  std::map<osg::Vec2i, blockType>::const_iterator a, b;
  bool aGo = false, bGo = false, projDel = false;

  if ((a = _typeMap->find(_tileCollizionPt1)) != _typeMap->end())
    if ((*a).second == blockType::BRICK)
    {
      // destroying tile
      _toDelete->push_back((*_tileMap)[_tileCollizionPt1]);
      _typeMap->erase(a);
      // need to destroy this projectile
      projDel = true;
    }
    else if (((*a).second == blockType::ARMOR) || ((*a).second == blockType::BORDER))
    {
      // need to destroy this projectile
      projDel = true;
    }
    else
      aGo = true;
  else
    aGo = true;

  if ((b = _typeMap->find(_tileCollizionPt2)) != _typeMap->end())
    if ((*b).second == blockType::BRICK)
    {
      // destroying tile
      _toDelete->push_back((*_tileMap)[_tileCollizionPt2]);
      _typeMap->erase(b);
      // need to destroy this projectile
      projDel = true;
    }
    else if (((*b).second == blockType::ARMOR) || ((*b).second == blockType::BORDER))
    {
      // need to destroy this projectile
      projDel = true;
    }
    else
      bGo = true;
  else
    bGo = true;

  if (!projDel)
    for (auto it = _vehicles->cbegin(); it != _vehicles->end(); ++it)
    {
      osg::ref_ptr<vehicle> currentVehicle = *it;
      if (currentVehicle.get() != _parentVehicle && currentVehicle->IsEnabled())
        if (_z + 6 >= currentVehicle->GetZCoord() - 8 && _z + 2 <= currentVehicle->GetZCoord() + 8)
          if (_x + 6 >= currentVehicle->GetXCoord() - 8 && _x + 2 <= currentVehicle->GetXCoord() + 8) // есть попадание
          {
            if (currentVehicle->IsEnabled())
            {
              // need to destroy this projectile
              projDel = true;
              // if we hit front armor of heavy tank projectile can't pierce it
              if (!(currentVehicle->GetType() == vehicle::type::HEAVY &&
                abs(static_cast<int>(currentVehicle->GetCurDir()) - static_cast<int>(_dir)) == 2))
              {
                // creating explosion
                bang* bng = new bang(currentVehicle->GetXCoord(), 4, currentVehicle->GetZCoord(), *_toDelete);
                _parentVehicle->getParent(0)->addChild(bng);

                // destroying vehicle
                currentVehicle->Disable(); // disabe it
                _toDelete->push_back(currentVehicle); // puting it to the queue for deleting from scene

                // increase number of kills
                QApplication::postEvent(_ViewerWindow, new vehicleKilledSomebody
                  (_parentVehicle->GetPlayerNum(), _parentVehicle->AddKill()));

                // after pause enemy will respawn
                ViewerWidget* vw = _ViewerWindow;
                QTimer::singleShot(3000, vw, [currentVehicle, vw]
                {
                  QApplication::postEvent(vw, new vehicleNeedRespawn(currentVehicle.get()));
                });
              }
            }
          }
    }

  if (projDel)
  {
    // destroying this projectile
    removeUpdateCallback(_clb);
    _toDelete->push_back(this);
  }

  if (aGo && bGo && !projDel)
    moving();
}