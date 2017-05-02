#include "tank.h"
#include "projectile.h"
#include <osgDB/ReadFile>
#include <osg/Texture2D>

const int SHOOT_TIMEOUT = 300; // задержка между выстрелами в мс

tank::tank(int x, int z, std::string texNum, int controlDevice,
  std::vector<osg::ref_ptr<tank>>* tank,
  std::map<osg::Vec2i, blockType>* typeMap,
  std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>>* tileMap,
  std::list<osg::Node*>* toDelete, tile* prjMaker)
  : _timer(new QDeadlineTimer(SHOOT_TIMEOUT)), _rMt(new MatrixTransform),
  _typeMap(typeMap), _tileMap(tileMap), _toDelete(toDelete), _texNum(texNum),
  _tank(tank), _controlDevice(controlDevice), _x(x), _z(z), _prjMaker(prjMaker), _type(type::LIGHT)
{
  this->setDataVariance(osg::Object::DYNAMIC);

  osg::Matrix m; // перемещаем в точку спавна
  m.makeTranslate(_x, 0, _z);
  this->setMatrix(m); // наследуется от MatrixTransform для перемещения

  osg::ref_ptr<osg::Node> node = // читаем модельку
    osgDB::readNodeFile("./Resources/lightTank/bradle.3ds.15.scale.90,90,0.rot");
  osg::ref_ptr<osg::Image> image = // читаем текстуру
    osgDB::readImageFile("./Resources/lightTank/" + texNum + ".png");

  // устанавливаем текстуру
  osg::StateSet* state = node->getOrCreateStateSet();
  osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
  texture->setImage(image.get());
  state->setTextureAttributeAndModes(0, texture.get());
  
  _rMt->addChild(node.get()); // дополнительный MatrixTransform для поворота
  this->addChild(_rMt.get());
}

void tank::ChangeType()
{
  _rMt->removeChildren(0, _rMt->getNumChildren());

  osg::ref_ptr<osg::Node> node;
  osg::ref_ptr<osg::Image> image;
  
  if (_type == type::HEAVY)
  {
    node = // читаем модельку
      osgDB::readNodeFile("./Resources/lightTank/bradle.3ds.15.scale.90,90,0.rot");
    image = // читаем текстуру
      osgDB::readImageFile("./Resources/lightTank/" + _texNum + ".png");
    _type = type::LIGHT;
  }
  else if (_type == type::LIGHT)
  {
    node = // читаем модельку
      osgDB::readNodeFile("./Resources/heavyTank/Leopard 2A4.3ds.5.scale.90,180,0.rot");
    image = // читаем текстуру
      osgDB::readImageFile("./Resources/heavyTank/" + _texNum + ".bmp");
    _type = type::HEAVY;
  }

  // устанавливаем текстуру
  osg::StateSet* state = node->getOrCreateStateSet();
  osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
  texture->setImage(image.get());
  state->setTextureAttributeAndModes(0, texture.get());

  _rMt->addChild(node.get());

  _needTypeChange = false;
}

const int tank::AddKill()
{
  return ++_killCount;
}

const int tank::GetControlDevice() const
{
  return _controlDevice;
}

void tank::SetControlDevice(int cd)
{
  _controlDevice = cd;
}

const bool tank::NeedTypeChange() const
{
  return _needTypeChange;
}

void tank::SetNeedTypeChange()
{
  _needTypeChange = true;
}

const tank::type tank::GetType() const
{
  return _type;
}

void tank::SetXCoord(int x)
{
  _x = x;
}

void tank::SetZCoord(int z)
{
  _z = z;
}

const int tank::GetXCoord() const
{
  return _x;
}

const int tank::GetZCoord() const
{
  return _z;
}

void tank::SetMovingDirection(direction dir)
{
  _go = true;
  _goDir = dir;
}

void tank::Stop()
{
  _go = false;
}

const bool tank::Enabled() const
{
  return _enabled;
}

void tank::Move()
{
  // вычисление точек коллизии в зависимости от направления танка
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

  // определение коллизий с блоками
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

  // если не было коллизий с блоками определяет коллизии с другими танками
  if (aGo && bGo)
  {
    // цикл по всем танкам
    for (auto it = _tank->cbegin(); it != _tank->end(); ++it)
    {
      if ((*it).get() != this && (*it)->_enabled)
      {
        if (cpt1[0] > (*it)->_x - 8 && cpt1[0] < (*it)->_x + 8 &&
          cpt1[1] > (*it)->_z - 8 && cpt1[1] < (*it)->_z + 8)
          tStop = true; // впереди танк
        if (cpt2[0] > (*it)->_x - 8 && cpt2[0] < (*it)->_x + 8 &&
          cpt2[1] > (*it)->_z - 8 && cpt2[1] < (*it)->_z + 8)
          tStop = true;
      }
    }
    // впереди чисто, двигаемся
    if (!tStop)
    {
      if (_type == type::HEAVY)
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
      else
      {
        if (_goDir == direction::UP)
          _z+=2;
        if (_goDir == direction::DOWN)
          _z-=2;
        if (_goDir == direction::LEFT)
          _x-=2;
        if (_goDir == direction::RIGHT)
          _x+=2;
      }
    }
  }

  osg::Matrix mR;

  if (_curDir != _goDir) // если танк поворачивает
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

void tank::Enable()
{
  _clb = new tankCallback;
  this->setUpdateCallback(_clb);
  _timer->setRemainingTime(SHOOT_TIMEOUT);
  _enabled = true;
}

void tank::Disable()
{
  this->removeUpdateCallback(_clb); // чтоб не мог двигаться
  _clb = nullptr;
  _timer->setRemainingTime(-1); // чтоб не мог стрелять
  _enabled = false; // чтоб коллизии для него не расчитывались
}

const direction tank::CurDir() const
{
  return _curDir;
}

const bool tank::NeedToGo() const
{
  return _go;
}

void tank::Shoot()
{
  // обеспечиваем задержку при стрельбе
  if (_timer->hasExpired())
  {
    blockType prjDir;
    if (_curDir == direction::UP)
      prjDir = blockType::PRJ_UP;
    else if (_curDir == direction::DOWN)
      prjDir = blockType::PRJ_DOWN;
    else if (_curDir == direction::LEFT)
      prjDir = blockType::PRJ_LEFT;
    else if (_curDir == direction::RIGHT)
      prjDir = blockType::PRJ_RIGHT;

    _projectile = new projectile(_x - 4, -4, _z - 4, _curDir, prjDir, 
      this, _tank, _typeMap, _tileMap, _toDelete, _prjMaker);
    this->getParent(0)->addChild(_projectile.get());
    _projectile->setName(this->getName() + " - projectile");
    // обновляем таймер
    _timer->setRemainingTime(SHOOT_TIMEOUT);
  }
}

void tankCallback::operator()(osg::Node* nd, osg::NodeVisitor* ndv)
{
  tank* tnk = dynamic_cast<tank*>(nd);
  if (!delay)
    if (tnk->NeedToGo())
      tnk->Move();
  delay = !delay;
  traverse(nd, ndv);
}