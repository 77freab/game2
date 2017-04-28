#include "tank.h"

const int SHOOT_TIMEOUT = 300; // задержка между выстрелами в мс

tank::tank(int x, int z, std::string texNum, int joyNum,
  std::vector<osg::ref_ptr<tank>>* tank,
  std::map<osg::Vec2i, blockType>* typeMap,
  std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>>* tileMap,
  std::list<osg::Node*>* toDelete, tile* prjMaker)
  : _timer(new QDeadlineTimer(SHOOT_TIMEOUT)), _rMt(new MatrixTransform),
  _typeMap(typeMap), _tileMap(tileMap), _toDelete(toDelete), _texNum(texNum),
  _tank(tank), _joyNum(joyNum), _x(x), _z(z), _prjMaker(prjMaker), _type(type::LIGHT)
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

void tank::changeType()
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

void tank::enable()
{
  _clb = new tankCallback;
  this->setUpdateCallback(_clb);
  _timer->setRemainingTime(SHOOT_TIMEOUT);
  _enabled = true;
}

void tank::disable()
{
  this->removeUpdateCallback(_clb); // чтоб не мог двигаться
  _clb = nullptr;
  _timer->setRemainingTime(-1); // чтоб не мог стрелять
  _enabled = false; // чтоб коллизии для него не расчитывались
}

direction tank::curDir()
{
  return _curDir;
}

void tank::shoot()
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

projectile::projectile(int x, int y, int z, direction dir,
  blockType prjDir, tank* parentTank,
  std::vector<osg::ref_ptr<tank>>* tank,
  std::map<osg::Vec2i, blockType>* typeMap,
  std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>>* tileMap,
  std::list<osg::Node*>* toDelete, tile* prjMaker)
  : MatrixTransform(*prjMaker->getTile(x, y, z, prjDir, true)), _dir(dir), _x(x), _y(y), _z(z), _clb(new projectileCallback),
  _parentTank(parentTank), _tank(tank), _typeMap(typeMap), _tileMap(tileMap), _toDelete(toDelete)
{
  this->setDataVariance(osg::Object::DYNAMIC);
  this->setUpdateCallback(_clb);
  int prjSpeed;
  if (parentTank->_type == tank::type::LIGHT)
    prjSpeed = 4;
  else if(parentTank->_type == tank::type::HEAVY)
    prjSpeed = 2;
  switch (_dir)
  {
    case(direction::UP) :
    {
      _collisionPt1 = { (_x + 2) / 8, (_z + 8) / 8 };
      _collisionPt2 = { (_x + 6) / 8, (_z + 8) / 8 };
      moving = [this, prjSpeed]
      {
        _z += prjSpeed;
        _collisionPt1[1] = (_z + 8) / 8;
        _collisionPt2[1] = (_z + 8) / 8;
        mT.makeTranslate(_x, _y, _z);
        this->setMatrix(mT);
      };
      break;
    }
    case(direction::DOWN) :
    {
      _collisionPt1 = { (_x + 2) / 8, (_z) / 8 };
      _collisionPt2 = { (_x + 6) / 8, (_z) / 8 };
      moving = [this, prjSpeed]
      {
        _z -= prjSpeed;
        _collisionPt1[1] = (_z) / 8;
        _collisionPt2[1] = (_z) / 8;
        mT.makeTranslate(_x, _y, _z);
        this->setMatrix(mT);
      };
      break;
    }
    case(direction::LEFT) :
    {
      _collisionPt1 = { (_x) / 8, (_z + 2) / 8 };
      _collisionPt2 = { (_x) / 8, (_z + 6) / 8 };
      moving = [this, prjSpeed]
      {
        _x -= prjSpeed;
        _collisionPt1[0] = (_x) / 8;
        _collisionPt2[0] = (_x) / 8;
        mT.makeTranslate(_x, _y, _z);
        this->setMatrix(mT);
      };
      break;
    }
    case(direction::RIGHT) :
    {
      _collisionPt1 = { (_x + 8) / 8, (_z + 2) / 8 };
      _collisionPt2 = { (_x + 8) / 8, (_z + 6) / 8 };
      moving = [this, prjSpeed]
      {
        _x += prjSpeed;
        _collisionPt1[0] = (_x + 8) / 8;
        _collisionPt2[0] = (_x + 8) / 8;
        mT.makeTranslate(_x, _y, _z);
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

  if (!projDel)
    for (auto it = _tank->cbegin(); it != _tank->end(); ++it)
    {
      if ((*it).get() != _parentTank && (*it)->_enabled)
        if (_z + 6 >= (*it)->_z - 8 && _z + 2 <= (*it)->_z + 8)
          if (_x + 6 >= (*it)->_x - 8 && _x + 2 <= (*it)->_x + 8) // есть попадание
          {
            // уничтожить снаряд
            projDel = true;
          
            // если попали в лоб тяжелому танку - нет пробития
            osg::ref_ptr<tank> attackedEnemy = *it;
            if (!(attackedEnemy->_type == tank::type::HEAVY &&
              abs(static_cast<int>(attackedEnemy->curDir()) - static_cast<int>(_dir)) == 2))
            {
              // создаем взрыв
              bang* bng = new bang((*it)->_x, 4, (*it)->_z, _toDelete);
              _parentTank->getParent(0)->addChild(bng);

              // уничтожаем танк
              attackedEnemy->disable(); // отключаем его
              _toDelete->push_back(attackedEnemy); // ставим в очередь на удаление со сцены

              // увеличиваем число убийств
              emit _parentTank->smbdyKilled(++_parentTank->_killCount);
            
              // через какое-то время противнек зареспавнится
              QTimer::singleShot(3000, attackedEnemy, [attackedEnemy] { emit attackedEnemy->iNeedRespawn(attackedEnemy.get()); });
            }
          }
    }

  if (projDel)
  {
    // уничтожаем снаряд
    _toDelete->push_back(this);
    this->removeUpdateCallback(_clb);
  }

  if (aGo && bGo && !projDel)
    moving();
}

bang::bang(int x, int y, int z, std::list<osg::Node*>* toDelete)
  : _geode(new osg::Geode), _normals(new osg::Vec3Array), _geom(new osg::Geometry),
  _vertices(new osg::Vec3Array), _color(new osg::Vec4Array), _texCoord(new osg::Vec2Array), 
  _texture(new osg::Texture2D), _image(osgDB::readImageFile("./Resources/bang.png")),
  _toDelete(toDelete), _clb(new bangCallback)
{
  this->setDataVariance(osg::Object::DYNAMIC);
  this->setUpdateCallback(_clb);

  osg::Matrix m;
  m.makeTranslate(x, y, z);
  this->setMatrix(m);

  _geom->setColorBinding(osg::Geometry::BIND_OVERALL);
  _color->push_back(osg::Vec4(1.f, 1.f, 1.f, 1.f));
  _geom->setColorArray(_color.get());

  _vertices->push_back(osg::Vec3(-8, 0, -8)); // 1
  _vertices->push_back(osg::Vec3( 8, 0, -8)); // 2
  _vertices->push_back(osg::Vec3( 8, 0,  8)); // 3
  _vertices->push_back(osg::Vec3(-8, 0,  8)); // 4

  _geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));

  _geom->setVertexArray(_vertices.get());

  _normals->setBinding(osg::Array::BIND_OVERALL);
  _normals->push_back(osg::Vec3(0, -1, 0));

  _geom->setNormalArray(_normals.get());

  _texCoord->push_back(osg::Vec2(2./3, 0));
  _texCoord->push_back(osg::Vec2(   1, 0));
  _texCoord->push_back(osg::Vec2(   1, 1));
  _texCoord->push_back(osg::Vec2(2./3, 1));

  _geom->setTexCoordArray(0, _texCoord.get(), osg::Array::Binding::BIND_PER_VERTEX);

  _texture->setImage(_image);
  _texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE);
  _texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
  osg::StateSet* state = _geom->getOrCreateStateSet();
  state->setTextureAttributeAndModes(0, _texture);
  state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

  _geode->addDrawable(_geom.get());
  this->addChild(_geode.get());
}

void bang::makeBang()
{
  int temp = 10;
  _roughTimer++;
  if (_roughTimer == temp)
  {
    _texCoord = new osg::Vec2Array;

    _texCoord->push_back(osg::Vec2(1./3, 0));
    _texCoord->push_back(osg::Vec2(2./3, 0));
    _texCoord->push_back(osg::Vec2(2./3, 1));
    _texCoord->push_back(osg::Vec2(1./3, 1));

    _geom->setTexCoordArray(0, _texCoord.get(), osg::Array::Binding::BIND_PER_VERTEX);
  }
  if (_roughTimer == temp*2)
  {
    _texCoord = new osg::Vec2Array;

    _texCoord->push_back(osg::Vec2(   0, 0));
    _texCoord->push_back(osg::Vec2(1./3, 0));
    _texCoord->push_back(osg::Vec2(1./3, 1));
    _texCoord->push_back(osg::Vec2(   0, 1));

    _geom->setTexCoordArray(0, _texCoord.get(), osg::Array::Binding::BIND_PER_VERTEX);
  }
  if (_roughTimer == temp*3)
  {
    _toDelete->push_back(this);
    this->removeUpdateCallback(_clb);
  }
}

void projectileCallback::operator()(osg::Node* nd, osg::NodeVisitor* ndv)
{
  projectile* prj = dynamic_cast<projectile*>(nd);
  if (!delay)
    prj->move();
  delay = !delay;
  traverse(nd, ndv);
}

void tankCallback::operator()(osg::Node* nd, osg::NodeVisitor* ndv)
{
  tank* tnk = dynamic_cast<tank*>(nd);
  if (!delay)
    if (tnk->_go)
      tnk->move();
  delay = !delay;
  traverse(nd, ndv);
}

void bangCallback::operator()(osg::Node* nd, osg::NodeVisitor* ndv)
{
  bang* bng = dynamic_cast<bang*>(nd);
  bng->makeBang();
  traverse(nd, ndv);
}