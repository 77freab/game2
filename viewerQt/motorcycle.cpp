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

  // move motorcycle to spawn place
  osg::Matrix m;
  m.makeTranslate(GetXCoord(), 0, GetZCoord());
  setMatrix(m); // inherited from MatrixTransform for transition

  // reading model
  osg::ref_ptr<osg::Node> model = osgDB::readNodeFile
    ("./Resources/motoTank/Scooter.3ds.15.scale.90,0,0.rot");
  // reading texture
  osg::ref_ptr<osg::Image> image = osgDB::readImageFile
    ("./Resources/motoTank/" + std::to_string(GetPlayerNum() % COLORED_TEXTURES_NUM) + ".bmp");

  // setting the texture
  osg::StateSet* state = model->getOrCreateStateSet();
  osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
  texture->setImage(image.get());
  state->setTextureAttributeAndModes(0, texture.get());

  getRotateMT()->addChild(model.get());
}

// shooting
void motorcycle::Shoot()
{
  // checking for delay in shooting
  if (getShotDelayTimer()->hasExpired() && _numBombs < MAX_NUM_BOMBS)
  {
    _numBombs++;
    osg::ref_ptr<bomb> droppedBomb = new bomb(GetXCoord(), -4, GetZCoord(), 
      this, _vehicles, _typeMap, _tileMap, _toDelete, _ViewerWindow);
    getParent(0)->addChild(droppedBomb.get());
    droppedBomb->setName(getName() + " - bomb");
    // updating the delay timer
    getShotDelayTimer()->setRemainingTime(SHOT_TIMEOUT);
  }
}