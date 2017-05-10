#include <osg/Texture2D>
#include <osgDB/ReadFile>

#include "lightTank.h"
#include "projectile.h"

lightTank::lightTank(int x, int z, int playerNum, int controlDevice,
  std::vector<osg::ref_ptr<vehicle>>* vehicles,
  std::map<osg::Vec2i, blockType>* typeMap,
  std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>>* tileMap,
  std::list<osg::Node*>* toDelete,
  ViewerWidget* ViewerWindow)
  : vehicle(x, z, 2, type::LIGHT, playerNum, controlDevice, vehicles, typeMap, tileMap, toDelete, ViewerWindow)
{
  setDataVariance(osg::Object::DYNAMIC);

  // ���������� � ����� ������
  osg::Matrix m;
  m.makeTranslate(GetXCoord(), 0, GetZCoord());
  setMatrix(m); // ����������� �� MatrixTransform ��� �����������

  // ������ ��������
  osg::ref_ptr<osg::Node> model = osgDB::readNodeFile
    ("./Resources/lightTank/bradle.3ds.15.scale.90,90,0.rot");
  // ������ ��������
  osg::ref_ptr<osg::Image> image = osgDB::readImageFile
    ("./Resources/lightTank/" + std::to_string(GetPlayerNum() % 13) + ".png");

  // ������������� ��������
  osg::StateSet* state = model->getOrCreateStateSet();
  osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
  texture->setImage(image.get());
  state->setTextureAttributeAndModes(0, texture.get());

  getRotateMT()->addChild(model.get());
}

// ��������
void lightTank::Shoot()
{
  // ������������ �������� ��� ��������
  if (getShotDelayTimer()->hasExpired())
  {
    osg::ref_ptr<projectile> prj = new projectile(GetXCoord() - 4, -4, GetZCoord() - 4,
      4, GetCurDir(), this, _vehicles, _typeMap, _tileMap, _toDelete, _ViewerWindow);
    getParent(0)->addChild(prj.get());
    prj->setName(getName() + " - projectile");
    // ��������� ������
    getShotDelayTimer()->setRemainingTime(SHOT_TIMEOUT);
  }
}