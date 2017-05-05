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

// �����������
projectile::projectile(int x, int y, int z, int speed, direction dir, vehicle* parentVehicle,
  std::vector<osg::ref_ptr<vehicle>>* vehicles, std::map<osg::Vec2i, blockType>* typeMap,
  std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>>* tileMap,
  std::list<osg::Node*>* toDelete, ViewerWidget* ViewerWindow)
  : _dir(dir), _x(x), _y(y), _z(z), _speed(speed), _clb(new projectileCallback),
  _parentVehicle(parentVehicle), _vehicles(vehicles), _typeMap(typeMap), _tileMap(tileMap),
  _toDelete(toDelete), _ViewerWindow(ViewerWindow)
{
  this->setDataVariance(osg::Object::DYNAMIC);
  this->setUpdateCallback(_clb);

  osg::Matrix m; // ���������� � ����� ������
  m.makeTranslate(_x, _y, _z);
  this->setMatrix(m); // ����������� �� MatrixTransform ��� �����������
  
  // ������ �������� ���� ��� �� ��������� �����
  if (_textureImage == nullptr)
    _textureImage = osgDB::readImageFile("./Resources/projectile/Bullet_U.bmp");

  switch (_dir)
  {
    case(direction::UP) :
    {
      // ������������ �������� ��� ����������� �����������
      _model = osgDB::readNodeFile
        ("./Resources/projectile/Bullet.3ds.-90,0,0.rot.4,-4,4.trans");
      _tileCollizionPt1 = { (_x + 2) / 8, (_z + 8) / 8 };
      _tileCollizionPt2 = { (_x + 6) / 8, (_z + 8) / 8 };
      moving = [this]
      {
        _z += _speed;
        _tileCollizionPt1[1] = (_z + 8) / 8;
        _tileCollizionPt2[1] = (_z + 8) / 8;
        _mT.makeTranslate(_x, _y, _z);
        this->setMatrix(_mT);
      };
      break;
    }
    case(direction::DOWN) :
    {
      // ������������ �������� ��� ����������� �����������
      _model = osgDB::readNodeFile
        ("./Resources/projectile/Bullet.3ds.90,0,0.rot.4,4,4.trans");
      _tileCollizionPt1 = { (_x + 2) / 8, (_z) / 8 };
      _tileCollizionPt2 = { (_x + 6) / 8, (_z) / 8 };
      moving = [this]
      {
        _z -= _speed;
        _tileCollizionPt1[1] = (_z) / 8;
        _tileCollizionPt2[1] = (_z) / 8;
        _mT.makeTranslate(_x, _y, _z);
        this->setMatrix(_mT);
      };
      break;
    }
    case(direction::LEFT) :
    {
      // ������������ �������� ��� ����������� �����������
      _model = osgDB::readNodeFile
        ("./Resources/projectile/Bullet.3ds.0,0,-90.rot.4,0,0.trans");
      _tileCollizionPt1 = { (_x) / 8, (_z + 2) / 8 };
      _tileCollizionPt2 = { (_x) / 8, (_z + 6) / 8 };
      moving = [this]
      {
        _x -= _speed;
        _tileCollizionPt1[0] = (_x) / 8;
        _tileCollizionPt2[0] = (_x) / 8;
        _mT.makeTranslate(_x, _y, _z);
        this->setMatrix(_mT);
      };
      break;
    }
    case(direction::RIGHT) :
    {
      // ������������ �������� ��� ����������� �����������
      _model = osgDB::readNodeFile
        ("./Resources/projectile/Bullet.3ds.0,0,90.rot.4,0,0.trans");
      _tileCollizionPt1 = { (_x + 8) / 8, (_z + 2) / 8 };
      _tileCollizionPt2 = { (_x + 8) / 8, (_z + 6) / 8 };
      moving = [this]
      {
        _x += _speed;
        _tileCollizionPt1[0] = (_x + 8) / 8;
        _tileCollizionPt2[0] = (_x + 8) / 8;
        _mT.makeTranslate(_x, _y, _z);
        this->setMatrix(_mT);
      };
      break;
    }
  }

  // ������������� ��������
  osg::StateSet* state = _model->getOrCreateStateSet();
  osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
  texture->setImage(_textureImage.get());
  state->setTextureAttributeAndModes(0, texture.get());

  this->addChild(_model.get());
}

// ������� ��������
void projectile::TryToMove()
{
  std::map<osg::Vec2i, blockType>::const_iterator a, b;
  bool aGo = false, bGo = false, projDel = false;

  if ((a = _typeMap->find(_tileCollizionPt1)) != _typeMap->end())
    if ((*a).second == blockType::BRICK)
    {
      // ���������� ����
      _toDelete->push_back((*_tileMap)[_tileCollizionPt1]);
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

  if ((b = _typeMap->find(_tileCollizionPt2)) != _typeMap->end())
    if ((*b).second == blockType::BRICK)
    {
      // ���������� ����
      _toDelete->push_back((*_tileMap)[_tileCollizionPt2]);
      _typeMap->erase(b);
      projDel = true;
    }
    else if (((*b).second == blockType::ARMOR) || ((*b).second == blockType::BORDER))
    {
      // ���������� ������
      projDel = true;
    }
    else
      bGo = true;
  else
    bGo = true;

  if (!projDel)
    for (auto it = _vehicles->cbegin(); it != _vehicles->end(); ++it)
    {
      if ((*it).get() != _parentVehicle && (*it)->IsEnabled())
        if (_z + 6 >= (*it)->GetZCoord() - 8 && _z + 2 <= (*it)->GetZCoord() + 8)
          if (_x + 6 >= (*it)->GetXCoord() - 8 && _x + 2 <= (*it)->GetXCoord() + 8) // ���� ���������
          {
            osg::ref_ptr<vehicle> attackedEnemy = *it;

            if (attackedEnemy->IsEnabled())
            {
              // ���������� ������
              projDel = true;
              // ���� ������ � ��� �������� ����� - ��� ��������
              if (!(attackedEnemy->GetType() == vehicle::type::HEAVY &&
                abs(static_cast<int>(attackedEnemy->GetCurDir()) - static_cast<int>(_dir)) == 2))
              {
                // ������� �����
                bang* bng = new bang((*it)->GetXCoord(), 4, (*it)->GetZCoord(), _toDelete);
                _parentVehicle->getParent(0)->addChild(bng);

                // ���������� ����
                attackedEnemy->Disable(); // ��������� ���
                _toDelete->push_back(attackedEnemy); // ������ � ������� �� �������� �� �����

                // ����������� ����� �������
                QApplication::postEvent(_ViewerWindow,
                  new vehicleKilledSomebody(_parentVehicle->GetPlayerNum(), _parentVehicle->AddKill()));

                // ����� �����-�� ����� ��������� �������������
                ViewerWidget* vw = _ViewerWindow;
                QTimer::singleShot(3000, vw, [attackedEnemy, vw]
                {
                  QApplication::postEvent(vw, new vehicleNeedRespawn(attackedEnemy.get()));
                });
              }
            }
          }
    }

  if (projDel)
  {
    // ���������� ������
    this->removeUpdateCallback(_clb);
    _toDelete->push_back(this);
  }

  if (aGo && bGo && !projDel)
    moving();
}