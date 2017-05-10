#include <QApplication>
#include <QTimer>
#include <QtMath>

#include <osgDB/ReadFile>

#include "bomb.h"
#include "bang.h"
#include "main.h"

const int BOMB_BANG_DELAY = 100;

class bombCallback : public osg::NodeCallback
{
public:
  void operator()(osg::Node* nd, osg::NodeVisitor* ndv) override;
private:
  int _delay = 0;
};

void bombCallback::operator()(osg::Node* nd, osg::NodeVisitor* ndv)
{
  if (_delay == BOMB_BANG_DELAY)
  {
    bomb* bmb = dynamic_cast<bomb*>(nd);
    bmb->Explode();
  }
  _delay++;
  traverse(nd, ndv);
}

// �����������
bomb::bomb(int x, int y, int z, vehicle* parentVehicle,
  std::vector<osg::ref_ptr<vehicle>>* vehicles, std::map<osg::Vec2i, blockType>* typeMap,
  std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>>* tileMap,
  std::list<osg::Node*>* toDelete, ViewerWidget* ViewerWindow)
  //  : _x(qFloor(x / 16.) * 16 + 8), _y(y), _z(qFloor(z / 16.) * 16 + 8), 
  : _x(qFloor(x / 8.) * 8), _y(y), _z(qFloor(z / 8.) * 8),
  _clb(new bombCallback), _vehicles(vehicles), _typeMap(typeMap), _tileMap(tileMap),
  _toDelete(toDelete), _ViewerWindow(ViewerWindow), _parentVehicle(parentVehicle)
{
  setUpdateCallback(_clb);

  // ���������� � ����� ������
  osg::Matrix m;
  m.makeTranslate(_x, _y, _z);
  setMatrix(m); // ����������� �� MatrixTransform ��� �����������

  // ������ ��������
  osg::ref_ptr<osg::Node> node = osgDB::readNodeFile
    ("./Resources/bomb/Bomb.3ds.90,0,0.rot.40.scale");
  // ������ ��������
  osg::ref_ptr<osg::Image> image = osgDB::readImageFile
    ("./Resources/bomb/Albedo.png");

  // ������������� ��������
  osg::StateSet* state = node->getOrCreateStateSet();
  osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
  texture->setImage(image.get());
  state->setTextureAttributeAndModes(0, texture.get());

  addChild(node.get());
}

bool bomb::destroyTilesAt(int x, int z)
{
  std::map<osg::Vec2i, blockType>::const_iterator a;
  bool stop = false;

  // ���� �� ������ ����������� ���� ����
  if ((a = _typeMap->find({ x, z })) != _typeMap->end())
    if ((*a).second == blockType::BRICK)
    {
      // ���������� ����
      _toDelete->push_back((*_tileMap)[{ x, z }]);
      _typeMap->erase(a);
      stop = true;
    }
    else
      stop = true;

  return stop;
}

void bomb::destroyVehiclesAt(int fromX, int toX, int fromZ, int toZ)
{
  // �� ������
  for (auto it = _vehicles->cbegin(); it != _vehicles->end(); ++it)
  {
    int vehicleX = (*it)->GetXCoord();
    int vehicleZ = (*it)->GetZCoord();
    if (vehicleX > fromX && vehicleX < toX)
      if (vehicleZ > fromZ && vehicleZ < toZ)
      {
        vehicle* attackedEnemy = (*it).get();

        if (attackedEnemy->IsEnabled())
        {
          // ������� �����
          bang* bng = new bang((*it)->GetXCoord(), -4, (*it)->GetZCoord(), _toDelete);
          getParent(0)->addChild(bng);

          // ���������� ����
          attackedEnemy->Disable(); // ��������� ���
          _toDelete->push_back(attackedEnemy); // ������ � ������� �� �������� �� �����

          // ����������� ����� ������� (������������ �� ���������)
          if (_parentVehicle != attackedEnemy)
            QApplication::postEvent(_ViewerWindow,
            new vehicleKilledSomebody(_parentVehicle->GetPlayerNum(), _parentVehicle->AddKill()));

          // ����� �����-�� ����� ��������� �������������
          ViewerWidget* vw = _ViewerWindow;
          QTimer::singleShot(3000, vw, [attackedEnemy, vw]
          {
            QApplication::postEvent(vw, new vehicleNeedRespawn(attackedEnemy));
          });
        }
      }
  }
}

void bomb::Explode()
{
  std::map<osg::Vec2i, blockType>::const_iterator a;
  bool stop = false;
  int bombX = _x / 8;
  int bombZ = _z / 8;
  int fromX, toX, fromZ, toZ;
  bool pr1, pr2;

  // ���������� ������ ���������� ���� � ������ ������� �� �����
  // ������
  for (toZ = bombZ - 1; toZ < bombZ + 5; toZ++)
  {
    // ������� �����
    bang* bng = new bang(_x, -3, toZ * 8, _toDelete);
    getParent(0)->addChild(bng);
    // ���������� �����
    pr1 = destroyTilesAt(bombX - 1, toZ);
    pr2 = destroyTilesAt(bombX, toZ);
    if (pr1 || pr2)
      break;
  }
  // �����
  for (fromZ = bombZ - 2; fromZ > bombZ - 6; fromZ--)
  {
    // ������� �����
    bang* bng = new bang(_x, -3, fromZ * 8, _toDelete);
    getParent(0)->addChild(bng);
    // ���������� �����
    pr1 = destroyTilesAt(bombX - 1, fromZ);
    pr2 = destroyTilesAt(bombX, fromZ);
    if (pr1 || pr2)
      break;
  }
  // ���������� ����� �� ��������� (������ � ����� �� �����)
  destroyVehiclesAt(_x - 8, _x + 8, fromZ * 8, toZ * 8);
  // �����
  for (fromX = bombX - 2; fromX > bombX - 6; fromX--)
  {
    // ������� �����
    bang* bng = new bang(fromX * 8, -3, _z, _toDelete);
    getParent(0)->addChild(bng);
    // ���������� �����
    pr1 = destroyTilesAt(fromX, bombZ - 1);
    pr2 = destroyTilesAt(fromX, bombZ);
    if (pr1 || pr2)
      break;
  }
  // ������
  for (toX = bombX + 1; toX < bombX + 5; toX++)
  {
    // ������� �����
    bang* bng = new bang(toX * 8, -3, _z, _toDelete);
    getParent(0)->addChild(bng);
    // ���������� �����
    pr1 = destroyTilesAt(toX, bombZ - 1);
    pr2 = destroyTilesAt(toX, bombZ);
    if (pr1 || pr2)
      break;
  }
  // ���������� ����� �� ����������� (����� � ������ �� �����)
  destroyVehiclesAt(fromX * 8, toX * 8, _z - 8, _z + 8);

  // ���������� ������
  removeUpdateCallback(_clb);
  _toDelete->push_back(this);
  dynamic_cast<motorcycle*>(_parentVehicle)->BombExploded();
}