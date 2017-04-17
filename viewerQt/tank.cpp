#include "tank.h"
#include "createMap.h"
#include <osg/Array>
#include <osg/Switch>
#include <cmath>
#include <osgDB/ReadFile>

const int SHOOT_TIMEOUT = 300;

extern std::map<osg::Vec2i, blockType> map;
extern std::map<osg::Vec2i, tile*> tileMap;
extern std::list<osg::Node*> toDelete;

tank::tank(int x, int z, std::string texTankType)
  : //_dl(new tile(x, 0, z, texTankType + "UP_C1/dl.png", true)),
  //_dr(new tile(x + 8, 0, z, texTankType + "UP_C1/dr.png", true)),
  //_ul(new tile(x, 0, z + 8, texTankType + "UP_C1/ul.png", true)),
  //_ur(new tile(x + 8, 0, z + 8, texTankType + "UP_C1/ur.png", true)),
  _texTankType(texTankType), _clb(new tankCallback),
  _timer(new QDeadlineTimer(SHOOT_TIMEOUT)), _x0(x), _z0(z), _x(x), _z(z)
{
  this->setDataVariance(osg::Object::DYNAMIC);
  this->setUpdateCallback(_clb);
  //this->addChild(_dl);
  //this->addChild(_dr);
  //this->addChild(_ul);
  //this->addChild(_ur);

  tempMt = new MatrixTransform;

  osg::ref_ptr<osg::Geode> geode = new osg::Geode;
  osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
  osg::ref_ptr<osg::Vec4Array> color = new osg::Vec4Array;
  osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
  osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
  osg::ref_ptr<osg::Vec2Array> texCoord = new osg::Vec2Array;
  osg::ref_ptr<osg::Image> imageTop = new osg::Image;
  osg::ref_ptr<osg::Image> imageLeft = new osg::Image;
  osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;

  osg::Matrix m;
  m.makeTranslate(_x, 0, _z);
  this->setMatrix(m);

  geom->setColorArray(color);
  geom->setColorBinding(osg::Geometry::BIND_OVERALL);
  color->push_back(osg::Vec4(1.f, 1.f, 1.f, 1.f));

  geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,  0, 4));
  geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,  4, 4));
  geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,  8, 4));
  geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 12, 4));
  geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 16, 4));

  vertices->push_back(osg::Vec3(-8,  0, -8)); // 1 снизу
  vertices->push_back(osg::Vec3( 8,  0, -8)); // 2
  vertices->push_back(osg::Vec3( 8, -8, -8)); // 3
  vertices->push_back(osg::Vec3(-8, -8, -8)); // 4

  vertices->push_back(osg::Vec3( 8,  0, -8)); // 5 справа
  vertices->push_back(osg::Vec3( 8,  0,  8)); // 6
  vertices->push_back(osg::Vec3( 8, -8,  8)); // 7
  vertices->push_back(osg::Vec3( 8, -8, -8)); // 8

  vertices->push_back(osg::Vec3( 8,  0,  8)); // 9 сверху
  vertices->push_back(osg::Vec3(-8,  0,  8)); // 10
  vertices->push_back(osg::Vec3(-8, -8,  8)); // 11
  vertices->push_back(osg::Vec3( 8, -8,  8)); // 12

  vertices->push_back(osg::Vec3(-8,  0,  8)); // 13 слева
  vertices->push_back(osg::Vec3(-8,  0, -8)); // 14
  vertices->push_back(osg::Vec3(-8, -8, -8)); // 15
  vertices->push_back(osg::Vec3(-8, -8,  8)); // 16

  vertices->push_back(osg::Vec3(-8, -8, -8)); // 17 крышка
  vertices->push_back(osg::Vec3( 8, -8, -8)); // 18
  vertices->push_back(osg::Vec3( 8, -8,  8)); // 19
  vertices->push_back(osg::Vec3(-8, -8,  8)); // 20

  geom->setVertexArray(vertices);

  normals->setBinding(osg::Array::BIND_OVERALL);
  normals->push_back(osg::Vec3(0, -1, 0));

  geom->setNormalArray(normals);

  texCoord->push_back(osg::Vec2(  0, 0));
  texCoord->push_back(osg::Vec2(0.2, 0));
  texCoord->push_back(osg::Vec2(0.2, 1));
  texCoord->push_back(osg::Vec2(  0, 1));

  texCoord->push_back(osg::Vec2(0.2, 0));
  texCoord->push_back(osg::Vec2(0.4, 0));
  texCoord->push_back(osg::Vec2(0.4, 1));
  texCoord->push_back(osg::Vec2(0.2, 1));

  texCoord->push_back(osg::Vec2(0.4, 0));
  texCoord->push_back(osg::Vec2(0.6, 0));
  texCoord->push_back(osg::Vec2(0.6, 1));
  texCoord->push_back(osg::Vec2(0.4, 1));

  texCoord->push_back(osg::Vec2(0.6, 0));
  texCoord->push_back(osg::Vec2(0.8, 0));
  texCoord->push_back(osg::Vec2(0.8, 1));
  texCoord->push_back(osg::Vec2(0.6, 1));

  texCoord->push_back(osg::Vec2(0.8, 0));
  texCoord->push_back(osg::Vec2(  1, 0));
  texCoord->push_back(osg::Vec2(  1, 1));
  texCoord->push_back(osg::Vec2(0.8, 1));

  geom->setTexCoordArray(0, texCoord, osg::Array::Binding::BIND_OVERALL);

  imageTop = osgDB::readImageFile("./Resources/" + texTankType);
  texture->setImage(imageTop);
  texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE);
  texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
  osg::StateSet* state = geom->getOrCreateStateSet();
  state->setTextureAttributeAndModes(0, texture);
  state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

  geode->addDrawable(geom);
  tempMt->addChild(geode);
  this->addChild(tempMt);
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

  if (((a = map.find(_collisionPt1)) == map.end()) && ((b = map.find(_collisionPt2)) == map.end()))
    if (cpt1[0] <= _enemyTank->_x - 8 || cpt1[0] >= _enemyTank->_x + 8 ||
        cpt1[1] <= _enemyTank->_z - 8 || cpt1[1] >= _enemyTank->_z + 8)
      if (cpt2[0] <= _enemyTank->_x - 8 || cpt2[0] >= _enemyTank->_x + 8 ||
          cpt2[1] <= _enemyTank->_z - 8 || cpt2[1] >= _enemyTank->_z + 8)
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


  osg::Matrix mR;

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
        mR.makeRotate(0, osg::Vec3(0, -1, 0));
        tempMt->setMatrix(mR);
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
        tempMt->setMatrix(mR);
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
        tempMt->setMatrix(mR);
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
        tempMt->setMatrix(mR);
        break;
      }
    }
    _curDir = _goDir; // новое текущее направление
  }
  //dynamic_cast<tile*>(this->getChild(0))->setTexture(_texTankType + _texDir + _texChassis + "dl.png");
  //dynamic_cast<tile*>(this->getChild(1))->setTexture(_texTankType + _texDir + _texChassis + "dr.png");
  //dynamic_cast<tile*>(this->getChild(2))->setTexture(_texTankType + _texDir + _texChassis + "ul.png");
  //dynamic_cast<tile*>(this->getChild(3))->setTexture(_texTankType + _texDir + _texChassis + "ur.png");

  _texChassis == "_C1/" ? _texChassis = "_C2/" : _texChassis = "_C1/"; // меняем тип шасси

  osg::Matrix mT;
  
  //mT.makeTranslate(-8, 0, -8);
  
  //mT.makeTranslate(_x - _x0, 0, _z - _z0);
  //this->setMatrix(mT * mR);
  mT.makeTranslate(_x, 0, _z);
  //tempMt->setMatrix(mR);
  this->setMatrix(mT);
}

void tank::shoot()
{
  if (_timer->hasExpired())
  {
    _projectile = new projectile(_x - 4, 0, _z - 4, _curDir, 
      "projectile/" + _texDir + ".png", this, _enemyTank);
    this->getParent(0)->addChild(_projectile);
    _projectile->setName(this->getName() + " - projectile");
    _timer->setRemainingTime(SHOOT_TIMEOUT);
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

  if ((a = map.find(_collisionPt1)) != map.end())
    if ((*a).second == blockType::BRICK)
    {
      // уничтожаем блок
      toDelete.push_back(tileMap[_collisionPt1]);
      map.erase(a);
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

  if ((b = map.find(_collisionPt2)) != map.end())
    if ((*b).second == blockType::BRICK)
    {
      // уничтожаем блок
      toDelete.push_back(tileMap[_collisionPt2]);
      map.erase(b);
      projDel = true;
    }
    else if (((*b).second == blockType::ARMOR) || ((*b).second == blockType::BORDER))
    {
      projDel = true;
    }
    else
      bGo = true;
  else
    bGo = true;

  if (aGo && bGo)
    moving();

  if (projDel)
  {
    // уничтожаем снаряд
    toDelete.push_back(this);
    this->removeUpdateCallback(_clb);
  }

  //if ( ((a = map.find(_collisionPt1)) == map.end()) && ((b = map.find(_collisionPt2)) == map.end()) ) // если на пути ничего нет
  //  moving();
  //else if (((*a).second == blockType::WATER) || // если на пути вода, кусты или лед
  //  ((*a).second == blockType::BUSHES) || ((*a).second == blockType::ICE))
  //  moving();
  //else if ((*a).second == blockType::BRICK) // если на пути кирпич (разрушаемое)
  //{
  //  // уничтожаем блок
  //  toDelete.push_back(tileMap[_collisionPt]);
  //  map.erase(a);
  //  // уничтожаем снаряд
  //  toDelete.push_back(this);
  //  this->removeUpdateCallback(_clb);
  //  //_parentTank->_projectile = nullptr;
  //}
  //else // если на пути граница уровня или броня (неразрушаемое)
  //{
  //  // уничтожаем снаряд
  //  toDelete.push_back(this);
  //  this->removeUpdateCallback(_clb);
  //  //_parentTank->_projectile = nullptr;
  //}

  if (_z+6 >= _enemyTank->_z-8 && _z+2 <= _enemyTank->_z + 8)
    if (_x+6 >= _enemyTank->_x-8 && _x+2 <= _enemyTank->_x + 8) // есть попадание
    {
      // уничтожаем снаряд
      toDelete.push_back(this);
      this->removeUpdateCallback(_clb);
      //_parentTank->_projectile = nullptr;
      // уничтожаем танк
      //toDelete.push_back(_enemyTank);
      _enemyTank->_timer->setRemainingTime(-1);
      _enemyTank->removeUpdateCallback(_enemyTank->_clb);
      toDelete.push_back(_enemyTank);
      // останавливаем игру
      _parentTank->_timer->setRemainingTime(-1);
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