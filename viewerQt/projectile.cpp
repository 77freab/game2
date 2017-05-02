#include "projectile.h"
#include "bang.h"
#include <QTimer>

void projectileCallback::operator()(osg::Node* nd, osg::NodeVisitor* ndv)
{
  projectile* prj = dynamic_cast<projectile*>(nd);
  if (!delay)
    prj->move();
  delay = !delay;
  traverse(nd, ndv);
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
  if (parentTank->GetType() == tank::type::LIGHT)
    prjSpeed = 4;
  else if (parentTank->GetType() == tank::type::HEAVY)
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
      if ((*it).get() != _parentTank && (*it)->Enabled())
        if (_z + 6 >= (*it)->GetZCoord() - 8 && _z + 2 <= (*it)->GetZCoord() + 8)
          if (_x + 6 >= (*it)->GetXCoord() - 8 && _x + 2 <= (*it)->GetXCoord() + 8) // есть попадание
          {
            // уничтожить снаряд
            projDel = true;

            // если попали в лоб тяжелому танку - нет пробития
            osg::ref_ptr<tank> attackedEnemy = *it;
            if (!(attackedEnemy->GetType() == tank::type::HEAVY &&
              abs(static_cast<int>(attackedEnemy->CurDir()) - static_cast<int>(_dir)) == 2))
            {
              // создаем взрыв
              bang* bng = new bang((*it)->GetXCoord(), 4, (*it)->GetZCoord(), _toDelete);
              _parentTank->getParent(0)->addChild(bng);

              // уничтожаем танк
              attackedEnemy->Disable(); // отключаем его
              _toDelete->push_back(attackedEnemy); // ставим в очередь на удаление со сцены

              // увеличиваем число убийств
              emit _parentTank->smbdyKilled(_parentTank->AddKill());

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