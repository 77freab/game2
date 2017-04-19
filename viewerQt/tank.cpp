#include "tank.h"
#include <osgDB/ReadFile>

const int SHOOT_TIMEOUT = 300; // задержка между выстрелами в мс

tank::tank(int x, int z, std::string texNum, int joyNum,
  std::list<osg::ref_ptr<tank>>* tank,
  std::map<osg::Vec2i, blockType>* typeMap,
  std::map<osg::Vec2i, tile*>* tileMap,
  std::list<osg::Node*>* toDelete)
  : _clb(new tankCallback), _timer(new QDeadlineTimer(SHOOT_TIMEOUT)),
  _rMt(new MatrixTransform), _typeMap(typeMap), _tileMap(tileMap), _toDelete(toDelete),
  _tank(tank), _joyNum(joyNum), _x0(x), _z0(z), _x(x), _z(z)
{
  this->setDataVariance(osg::Object::DYNAMIC);
  this->setUpdateCallback(_clb);

  osg::Matrix m; // перемещаем в точку спавна
  m.makeTranslate(_x, 0, _z);
  this->setMatrix(m); // наследуется от MatrixTransform для перемещения

  osg::ref_ptr<osg::Node> node = // читаем модельку
    osgDB::readNodeFile("./Resources/Leopard 2A4.3ds.5.scale.90,180,0.rot");
  osg::ref_ptr<osg::Image> image = // читаем текстуру
    osgDB::readImageFile("./Resources/tank/tank" + texNum + ".bmp");

  // устанавливаем текстуру
  osg::StateSet* state = node->getOrCreateStateSet();
  osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
  texture->setImage(image.get());
  state->setTextureAttributeAndModes(0, texture.get());
  
  _rMt->addChild(node.get()); // дополнительный MatrixTransform для поворота
  this->addChild(_rMt.get());
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
      cpt1 = { _x - 7, _z + 7 };
      cpt2 = { _x + 7, _z + 7 };
      _collisionPt1 = { (_x - 8) / 8, (_z + 8) / 8 };
      _collisionPt2 = { static_cast<int>(ceil((_x - 1) / 8.)), (_z + 8) / 8 };
      break;
    }
    case(direction::DOWN) :
    {
      cpt1 = { _x - 7, _z - 7 };
      cpt2 = { _x + 7, _z - 7 };
      _collisionPt1 = { (_x - 8) / 8, (_z - 8) / 8 };
      _collisionPt2 = { static_cast<int>(ceil((_x - 1) / 8.)), (_z - 8) / 8 };
      break;
    }
    case(direction::LEFT) :
    {
      cpt1 = { _x - 7, _z + 7 };
      cpt2 = { _x - 7, _z - 7 };
      _collisionPt1 = { (_x - 8) / 8, (_z - 8) / 8 };
      _collisionPt2 = { (_x - 8) / 8, static_cast<int>(ceil((_z) / 8.)) };
      break;
    }
    case(direction::RIGHT) :
    {
      cpt1 = { _x + 7, _z + 7 };
      cpt2 = { _x + 7, _z - 7 };
      _collisionPt1 = { (_x + 8) / 8, (_z - 8) / 8 };
      _collisionPt2 = { (_x + 8) / 8, static_cast<int>(ceil((_z) / 8.)) };
      break;
    }
  }

  std::map<osg::Vec2i, blockType>::const_iterator a, b;
  bool aGo = false, bGo = false, tStop = false;

  if ((a = _typeMap->find(_collisionPt1)) != _typeMap->end())
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
  if ((b = _typeMap->find(_collisionPt2)) != _typeMap->end())
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

  if (aGo && bGo)
  {
    for (auto it = _tank->cbegin(); it != _tank->end(); ++it)
    {
      if ((*it).get() != this)
        if (cpt1[0] > (*it)->_x - 8 && cpt1[0] < (*it)->_x + 8 &&
          cpt1[1] > (*it)->_z - 8 && cpt1[1] < (*it)->_z + 8)
          if (cpt2[0] > (*it)->_x - 8 && cpt2[0] < (*it)->_x + 8 &&
            cpt2[1] > (*it)->_z - 8 && cpt2[1] < (*it)->_z + 8)
            tStop = true;
    }
    if (!tStop)
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
  }

  //if (((a = _typeMap->find(_collisionPt1)) == _typeMap->end()) && ((b = _typeMap->find(_collisionPt2)) == _typeMap->end()))
  //  if (cpt1[0] <= _enemyTank->_x - 8 || cpt1[0] >= _enemyTank->_x + 8 ||
  //      cpt1[1] <= _enemyTank->_z - 8 || cpt1[1] >= _enemyTank->_z + 8)
  //    if (cpt2[0] <= _enemyTank->_x - 8 || cpt2[0] >= _enemyTank->_x + 8 ||
  //        cpt2[1] <= _enemyTank->_z - 8 || cpt2[1] >= _enemyTank->_z + 8)
  //{
  //  if (_goDir == direction::UP)
  //    _z++;
  //  if (_goDir == direction::DOWN)
  //    _z--;
  //  if (_goDir == direction::LEFT)
  //    _x--;
  //  if (_goDir == direction::RIGHT)
  //    _x++;
  //}
  

  //if ((((*a).second == blockType::ICE) || ((*a).second == blockType::BUSHES)) && (((*b).second == blockType::ICE) || ((*b).second == blockType::BUSHES)))
  //  _z++;


  osg::Matrix mR;

  if (_curDir != _goDir) // танк поворачивает
  {
    switch (_goDir)
    {
      case(direction::UP) :
      {
        // при повороте позиция танка слегка "исправляется" для легкого вхождения в повороты
        if (_x % 8 >= 4) 
          _x = (_x / 8) * 8 + 8;
        else
          _x = (_x / 8) * 8;
        // текстовое представление направления используется для присвоения правильной текстуры снаряда при выстреле
        _texDir = "UP";
        // собственно поворот
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
        _texDir = "DOWN";
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
        _texDir = "LEFT";
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
        _texDir = "RIGHT";
        mR.makeRotate(-osg::PI_2, osg::Vec3(0, -1, 0));
        _rMt->setMatrix(mR);
        break;
      }
    }
    _curDir = _goDir; // новое текущее направление
  }
  //dynamic_cast<tile*>(this->getChild(0))->setTexture(_texTankType + _texDir + _texChassis + "dl.png");
  //dynamic_cast<tile*>(this->getChild(1))->setTexture(_texTankType + _texDir + _texChassis + "dr.png");
  //dynamic_cast<tile*>(this->getChild(2))->setTexture(_texTankType + _texDir + _texChassis + "ul.png");
  //dynamic_cast<tile*>(this->getChild(3))->setTexture(_texTankType + _texDir + _texChassis + "ur.png");

  // перемещаем танк
  osg::Matrix mT;
  mT.makeTranslate(_x, 0, _z);
  this->setMatrix(mT);
}

void tank::shoot()
{
  // обеспечиваем задержку при стрельбе
  if (_timer->hasExpired())
  {
    _projectile = new projectile(_x - 4, 0, _z - 4, _curDir, 
      "projectile/" + _texDir + ".png", this, _tank,
      _typeMap, _tileMap, _toDelete);
    this->getParent(0)->addChild(_projectile.get());
    _projectile->setName(this->getName() + " - projectile");
    // обновляем таймер
    _timer->setRemainingTime(SHOOT_TIMEOUT);
  }
}

projectile::projectile(int x, int y, int z, direction dir, 
  std::string texPath, tank* parentTank, 
  std::list<osg::ref_ptr<tank>>* tank,
  std::map<osg::Vec2i, blockType>* typeMap,
  std::map<osg::Vec2i, tile*>* tileMap,
  std::list<osg::Node*>* toDelete)
  : tile(x, y, z, texPath, true), _dir(dir), _x(x), _z(z), _clb(new projectileCallback),
  _parentTank(parentTank), _tank(tank), _typeMap(typeMap), _tileMap(tileMap), _toDelete(toDelete)
{
  this->setDataVariance(osg::Object::DYNAMIC);
  this->setUpdateCallback(_clb);
  switch (_dir)
  {
    case(direction::UP) :
    {
      _collisionPt1 = { (_x + 2) / 8, (_z + 8) / 8 };
      _collisionPt2 = { (_x + 6) / 8, (_z + 8) / 8 };
      moving = [this]
      {
        _z += 4;
        _collisionPt1[1] = (_z + 8) / 8;
        _collisionPt2[1] = (_z + 8) / 8;
        mT.makeTranslate(_x, 0, _z);
        this->setMatrix(mT);
      };
      break;
    }
    case(direction::DOWN) :
    {
      _collisionPt1 = { (_x + 2) / 8, (_z) / 8 };
      _collisionPt2 = { (_x + 6) / 8, (_z) / 8 };
      moving = [this]
      {
        _z -= 4;
        _collisionPt1[1] = (_z) / 8;
        _collisionPt2[1] = (_z) / 8;
        mT.makeTranslate(_x, 0, _z);
        this->setMatrix(mT);
      };
      break;
    }
    case(direction::LEFT) :
    {
      _collisionPt1 = { (_x) / 8, (_z + 2) / 8 };
      _collisionPt2 = { (_x) / 8, (_z + 6) / 8 };
      moving = [this]
      {
        _x -= 4;
        _collisionPt1[0] = (_x) / 8;
        _collisionPt2[0] = (_x) / 8;
        mT.makeTranslate(_x, 0, _z);
        this->setMatrix(mT);
      };
      break;
    }
    case(direction::RIGHT) :
    {
      _collisionPt1 = { (_x + 8) / 8, (_z + 2) / 8 };
      _collisionPt2 = { (_x + 8) / 8, (_z + 6) / 8 };
      moving = [this]
      {
        _x += 4;
        _collisionPt1[0] = (_x + 8) / 8;
        _collisionPt2[0] = (_x + 8) / 8;
        mT.makeTranslate(_x, 0, _z);
        this->setMatrix(mT);
      };
      break;
    }
  }
}

void projectile::move()
{
  std::map<osg::Vec2i, blockType>::const_iterator a, b;
  bool aGo = false, bGo = false, projDel = false;

  if ((a = _typeMap->find(_collisionPt1)) != _typeMap->end())
    if ((*a).second == blockType::BRICK)
    {
      // уничтожаем блок
      _toDelete->push_back((*_tileMap)[_collisionPt1]);
      _typeMap->erase(a);
      projDel = true;
    }
    else if (((*a).second == blockType::ARMOR) || ((*a).second == blockType::BORDER))
    {
      projDel = true;
    }
    else
      aGo = true;
  else
    aGo = true;

  if ((b = _typeMap->find(_collisionPt2)) != _typeMap->end())
    if ((*b).second == blockType::BRICK)
    {
      // уничтожаем блок
      _toDelete->push_back((*_tileMap)[_collisionPt2]);
      _typeMap->erase(b);
      projDel = true;
    }
    else if (((*b).second == blockType::ARMOR) || ((*b).second == blockType::BORDER))
    {
      // уничтожить снаряд
      projDel = true;
    }
    else
      bGo = true;
  else
    bGo = true;

  for (auto it = _tank->cbegin(); it != _tank->end(); ++it)
  {
    if ((*it).get() != _parentTank)
      if (_z + 6 >= (*it)->_z - 8 && _z + 2 <= (*it)->_z + 8)
        if (_x + 6 >= (*it)->_x - 8 && _x + 2 <= (*it)->_x + 8) // есть попадание
        {
          // уничтожить снаряд
          projDel = true;
          // уничтожаем танк
          (*it)->_timer->setRemainingTime(-1); // чтоб не мог стрелять
          (*it)->removeUpdateCallback((*it)->_clb); // чтоб не мог ездить
          _toDelete->push_back((*it)); // ставим в очередь на удаление со сцены
          it = _tank->erase(it); // убираем из списка всех танков
          if (it == _tank->end())
            break;
          // останавливаем игру
          //_parentTank->_timer->setRemainingTime(-1);
          //_parentTank->removeUpdateCallback(_parentTank->_clb);
          //if (_enemyTank->_projectile != nullptr)
          //  _enemyTank->_projectile->removeUpdateCallback(_enemyTank->_projectile->_clb);
        }
  }

  bool dad = false;

  if (projDel)
  {
    // уничтожаем снаряд
    _toDelete->push_back(this);
    this->removeUpdateCallback(_clb);
  }

  if (aGo && bGo && !projDel)
    moving();
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