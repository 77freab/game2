#include "tank.h"
#include "createMap.h"
#include <osg/Array>
#include <osg/Switch>
#include <cmath>

extern std::map<osg::Vec2i, blockType> map;
extern std::map<osg::Vec2i, tile*> tileMap;
extern std::list<osg::Node*> toDelete;

tank::tank(int x, int z, std::string texTankType)
  : _dl(new tile(x, 0, z, texTankType + "UP_C1/dl.png", true)),
  _dr(new tile(x + 8, 0, z, texTankType + "UP_C1/dr.png", true)),
  _ul(new tile(x, 0, z + 8, texTankType + "UP_C1/ul.png", true)),
  _ur(new tile(x + 8, 0, z + 8, texTankType + "UP_C1/ur.png", true)),
  _texTankType(texTankType), _clb(new tankCallback),
  _x0(x), _z0(z), _x(x), _z(z)
{
  this->setDataVariance(osg::Object::DYNAMIC);
  this->setUpdateCallback(_clb);
  this->addChild(_dl);
  this->addChild(_dr);
  this->addChild(_ul);
  this->addChild(_ur);
}

void tank::setEnemy(tank* enemy)
{
  _enemyTank = enemy;
}

void tank::moveTo(direction dir)
{
  _go = true;
  _goDir = dir;
}

void tank::stop()
{
  _go = false;
}

void tank::move()
{
  osg::Vec2i cpt1, cpt2;
  switch (_goDir)
  {
    case(direction::UP) :
    {
      cpt1 = { _x + 1, _z + 16 };
      cpt2 = { _x + 15, _z + 16 };
      _collisionPt1 = { (_x) / 8, (_z + 16) / 8 };
      _collisionPt2 = { static_cast<int>(ceil((_x + 7) / 8.)), (_z + 16) / 8 };
      break;
    }
    case(direction::DOWN) :
    {
      cpt1 = { _x + 1, _z };
      cpt2 = { _x + 15, _z };
      _collisionPt1 = { (_x) / 8, (_z) / 8 };
      _collisionPt2 = { static_cast<int>(ceil((_x + 7) / 8.)), (_z) / 8 };
      break;
    }
    case(direction::LEFT) :
    {
      cpt1 = { _x, _z + 1 };
      cpt2 = { _x, _z + 15 };
      _collisionPt1 = { (_x - 1) / 8, (_z + 1) / 8 };
      _collisionPt2 = { (_x - 1) / 8, static_cast<int>(ceil((_z + 8) / 8.)) };
      break;
    }
    case(direction::RIGHT) :
    {
      cpt1 = { _x + 16, _z + 1 };
      cpt2 = { _x + 16, _z + 15 };
      _collisionPt1 = { (_x + 15) / 8, (_z + 1) / 8 };
      _collisionPt2 = { (_x + 15) / 8, static_cast<int>(ceil((_z + 8) / 8.)) };
      break;
    }
  }

  std::map<osg::Vec2i, blockType>::const_iterator a, b;

  if (((a = map.find(_collisionPt1)) == map.end()) && ((b = map.find(_collisionPt2)) == map.end()))
    if (cpt1[0] <= _enemyTank->_x || cpt1[0] >= _enemyTank->_x + 16 ||
        cpt1[1] <= _enemyTank->_z || cpt1[1] >= _enemyTank->_z + 16)
      if (cpt2[0] <= _enemyTank->_x || cpt2[0] >= _enemyTank->_x + 16 ||
          cpt2[1] <= _enemyTank->_z || cpt2[1] >= _enemyTank->_z + 16)
  {
    if (_goDir == direction::UP)
      _z++;
    if (_goDir == direction::DOWN)
      _z--;
    if (_goDir == direction::LEFT)
      _x--;
    if (_goDir == direction::RIGHT)
      _x++;
  }
  

  //if ((((*a).second == blockType::ICE) || ((*a).second == blockType::BUSHES)) && (((*b).second == blockType::ICE) || ((*b).second == blockType::BUSHES)))
  //  _z++;


  if (_curDir != _goDir)
  {
    switch (_goDir)
    {
      case(direction::UP) :
      {
        if (_x % 8 >= 4)
          _x = (_x / 8) * 8 + 8;
        else
          _x = (_x / 8) * 8;
        _texDir = "UP";
        break;
      }
      case(direction::DOWN) :
      {
        if (_x % 8 >= 4)
          _x = (_x / 8) * 8 + 8;
        else
          _x = (_x / 8) * 8;
        _texDir = "DOWN";
        break;
      }
      case(direction::LEFT) :
      {
        if (_z % 8 >= 4)
          _z = (_z / 8) * 8 + 8;
        else
          _z = (_z / 8) * 8;
        _texDir = "LEFT";
        break;
      }
      case(direction::RIGHT) :
      {
        if (_z % 8 >= 4)
          _z = (_z / 8) * 8 + 8;
        else
          _z = (_z / 8) * 8;
        _texDir = "RIGHT";
        break;
      }
    }
    _curDir = _goDir; // новое текущее направление
  }
  dynamic_cast<tile*>(this->getChild(0))->setTexture(_texTankType + _texDir + _texChassis + "dl.png");
  dynamic_cast<tile*>(this->getChild(1))->setTexture(_texTankType + _texDir + _texChassis + "dr.png");
  dynamic_cast<tile*>(this->getChild(2))->setTexture(_texTankType + _texDir + _texChassis + "ul.png");
  dynamic_cast<tile*>(this->getChild(3))->setTexture(_texTankType + _texDir + _texChassis + "ur.png");

  _texChassis == "_C1/" ? _texChassis = "_C2/" : _texChassis = "_C1/"; // меняем тип шасси

  osg::Matrix mT;
  mT.makeTranslate(_x - _x0, 0, _z - _z0);
  this->setMatrix(mT);
}

void tank::shoot()
{
  if (_projectile == nullptr)
  {
    _projectile = new projectile(_x + 4, 0, _z + 4, _curDir, 
      "projectile/" + _texDir + ".png", this, _enemyTank);
    this->getParent(0)->addChild(_projectile);
    _projectile->setName(this->getName() + " - projectile");
  }
}

projectile::projectile(int x, int y, int z, direction dir, 
  std::string texPath, tank* parentTank, tank* enemyTank)
  : tile(x, y, z, texPath, true), _dir(dir), _x(x), _z(z), _clb(new projectileCallback),
  _parentTank(parentTank), _enemyTank(enemyTank)
{
  this->setDataVariance(osg::Object::DYNAMIC);
  this->setUpdateCallback(_clb);
  switch (_dir)
  {
    case(direction::UP) :
    {
      _collisionPt = { (_x + 4) / 8, (_z + 8) / 8 };
      moving = [this]
      {
        _z += 4;
        _collisionPt[1] = (_z + 8) / 8;
        mT.makeTranslate(_x, 0, _z);
        this->setMatrix(mT);
      };
      break;
    }
    case(direction::DOWN) :
    {
      _collisionPt = { (_x + 4) / 8, (_z) / 8 };
      moving = [this]
      {
        _z -= 4;
        _collisionPt[1] = (_z) / 8;
        mT.makeTranslate(_x, 0, _z);
        this->setMatrix(mT);
      };
      break;
    }
    case(direction::LEFT) :
    {
      _collisionPt = { (_x) / 8, (_z + 4) / 8 };
      moving = [this]
      {
        _x -= 4;
        _collisionPt[0] = (_x) / 8;
        mT.makeTranslate(_x, 0, _z);
        this->setMatrix(mT);
      };
      break;
    }
    case(direction::RIGHT) :
    {
      _collisionPt = { (_x + 8) / 8, (_z + 4) / 8 };
      moving = [this]
      {
        _x += 4;
        _collisionPt[0] = (_x + 8) / 8;
        mT.makeTranslate(_x, 0, _z);
        this->setMatrix(mT);
      };
      break;
    }
  }
}

void projectile::move()
{
  std::map<osg::Vec2i, blockType>::const_iterator a;

  if ((a = map.find(_collisionPt)) == map.end()) // если на пути ничего нет
    moving();
  else if (((*a).second == blockType::WATER) || // если на пути вода, кусты или лед
    ((*a).second == blockType::BUSHES) || ((*a).second == blockType::ICE))
    moving();
  else if ((*a).second == blockType::BRICK) // если на пути кирпич (разрушаемое)
  {
    // уничтожаем блок
    toDelete.push_back(tileMap[_collisionPt]);
    map.erase(a);
    // уничтожаем снаряд
    toDelete.push_back(this);
    this->removeUpdateCallback(_clb);
    _parentTank->_projectile = nullptr;
  }
  else // если на пути граница уровня или броня (неразрушаемое)
  {
    // уничтожаем снаряд
    toDelete.push_back(this);
    this->removeUpdateCallback(_clb);
    _parentTank->_projectile = nullptr;
  }

  if (_z+4 >= _enemyTank->_z && _z+4 <= _enemyTank->_z + 16)
    if (_x+4 >= _enemyTank->_x && _x+4 <= _enemyTank->_x + 16) // есть попадание
    {
      // уничтожаем снаряд
      //toDelete.push_back(this);
      this->removeUpdateCallback(_clb);
      //_parentTank->_projectile = nullptr;
      // уничтожаем танк
      //toDelete.push_back(_enemyTank);
      _enemyTank->removeUpdateCallback(_enemyTank->_clb);
      // останавливаем игру
      _parentTank->removeUpdateCallback(_parentTank->_clb);
      if (_enemyTank->_projectile != nullptr)
        _enemyTank->_projectile->removeUpdateCallback(_enemyTank->_projectile->_clb);
    }
}

void projectileCallback::operator()(osg::Node* nd, osg::NodeVisitor* ndv)
{
  projectile* prj = dynamic_cast<projectile*>(nd);
  prj->move();
  traverse(nd, ndv);
}

void tankCallback::operator()(osg::Node* nd, osg::NodeVisitor* ndv)
{
  tank* tnk = dynamic_cast<tank*>(nd);
  if (tnk->_go)
    tnk->move();
  traverse(nd, ndv);
}