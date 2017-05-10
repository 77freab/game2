#include <osg/Texture2D>
#include <osgDB/ReadFile>

#include "bomb.h"
#include "motorcycle.h"

motorcycle::motorcycle(int x, int z, int playerNum, int controlDevice,
  std::vector<osg::ref_ptr<vehicle>>* vehicles,
  std::map<osg::Vec2i, blockType>* typeMap,
  std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>>* tileMap,
  std::list<osg::Node*>* toDelete,
  ViewerWidget* ViewerWindow)
  : vehicle(x, z, 3, type::MOTO, playerNum, controlDevice, vehicles, typeMap, tileMap, toDelete, ViewerWindow)
{
  setDataVariance(osg::Object::DYNAMIC);

  // перемещаем в точку спавна
  osg::Matrix m;
  m.makeTranslate(GetXCoord(), 0, GetZCoord());
  setMatrix(m); // наследуетс€ от MatrixTransform дл€ перемещени€

  // читаем модельку
  osg::ref_ptr<osg::Node> model = osgDB::readNodeFile
    ("./Resources/motoTank/Scooter.3ds.15.scale.90,0,0.rot");
  // читаем текстуру
  osg::ref_ptr<osg::Image> image = osgDB::readImageFile
    ("./Resources/motoTank/" + std::to_string(GetPlayerNum() % COLORED_TEXTURES_NUM) + ".bmp");

  // устанавливаем текстуру
  osg::StateSet* state = model->getOrCreateStateSet();
  osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
  texture->setImage(image.get());
  state->setTextureAttributeAndModes(0, texture.get());

  getRotateMT()->addChild(model.get());
}

// стрельба
void motorcycle::Shoot()
{
  // обеспечиваем задержку при стрельбе
  if (getShotDelayTimer()->hasExpired() && _numBombs < MAX_NUM_BOMBS)
  {
    _numBombs++;
    osg::ref_ptr<bomb> droppedBomb = new bomb(GetXCoord(), -4, GetZCoord(), 
      this, _vehicles, _typeMap, _tileMap, _toDelete, _ViewerWindow);
    getParent(0)->addChild(droppedBomb.get());
    droppedBomb->setName(getName() + " - bomb");
    // обновл€ем таймер
    getShotDelayTimer()->setRemainingTime(SHOT_TIMEOUT);
  }
}