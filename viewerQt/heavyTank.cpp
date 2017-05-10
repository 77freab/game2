#include <osg/Texture2D>
#include <osgDB/ReadFile>

#include "heavyTank.h"
#include "projectile.h"

heavyTank::heavyTank(int x, int z, int playerNum, int controlDevice,
  std::vector<osg::ref_ptr<vehicle>>* vehicles,
  std::map<osg::Vec2i, blockType>* typeMap,
  std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>>* tileMap,
  std::list<osg::Node*>* toDelete,
  ViewerWidget* ViewerWindow)
  : vehicle(x, z, 1, type::HEAVY, playerNum, controlDevice, vehicles, typeMap, tileMap, toDelete, ViewerWindow)
{
  setDataVariance(osg::Object::DYNAMIC);

  // move tank to spawn place
  osg::Matrix m;
  m.makeTranslate(GetXCoord(), 0, GetZCoord());
  setMatrix(m); // inherited from MatrixTransform for transition

  // reading model
  osg::ref_ptr<osg::Node> model = osgDB::readNodeFile
    ("./Resources/heavyTank/Leopard 2A4.3ds.5.scale.90,180,0.rot");
  // reading texture
  osg::ref_ptr<osg::Image> image = osgDB::readImageFile
    ("./Resources/heavyTank/" + std::to_string(GetPlayerNum() % COLORED_TEXTURES_NUM) + ".bmp");
  
  // setting the texture
  osg::StateSet* state = model->getOrCreateStateSet();
  osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
  texture->setImage(image.get());
  state->setTextureAttributeAndModes(0, texture.get());

  getRotateMT()->addChild(model.get());
}

// shooting
void heavyTank::Shoot()
{
  // checking for delay in shooting
  if (getShotDelayTimer()->hasExpired())
  {
    osg::ref_ptr<projectile> prj = new projectile(GetXCoord() - 4, -4, GetZCoord() - 4, 
      2, GetCurDir(), this, _vehicles, _typeMap, _tileMap, _toDelete, _ViewerWindow);
    getParent(0)->addChild(prj.get());
    prj->setName(getName() + " - projectile");
    // updating the delay timer
    getShotDelayTimer()->setRemainingTime(SHOT_TIMEOUT);
  }
}