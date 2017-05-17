#include <osg/Texture2D>
#include <osgDB/ReadFile>

#include "lightTank.h"
#include "projectile.h"

LightTank::LightTank( int x, 
                      int z, 
                      int playerNum, 
                      int controlDevice,
                      std::vector<osg::ref_ptr<Vehicle>>& vehicles,
                      std::vector<std::vector<osg::ref_ptr<Tile>>>& tileMap,
                      std::list<osg::Node*>& toDelete,
                      ViewerWidget& ViewerWindow, 
                      int killCount) : 
  Vehicle(x, z, 2, Type::LIGHT, playerNum, controlDevice, 
  killCount, vehicles, tileMap, toDelete, ViewerWindow)
{
  setDataVariance(osg::Object::DYNAMIC);

  // move tank to spawn place
  osg::Matrix m;
  m.makeTranslate(GetXCoord(), 0, GetZCoord());
  setMatrix(m); // inherited from MatrixTransform for transition

  // reading model
  osg::ref_ptr<osg::Node> model = osgDB::readNodeFile
    ("./Resources/lightTank/bradle.3ds.15.scale.90,90,0.rot");
  // reading texture
  osg::ref_ptr<osg::Image> image = osgDB::readImageFile
    ("./Resources/lightTank/" + std::to_string(GetPlayerNum() % COLORED_TEXTURES_NUM) + ".png");

  // setting the texture
  osg::StateSet* state = model->getOrCreateStateSet();
  osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
  texture->setImage(image.get());
  state->setTextureAttributeAndModes(0, texture.get());

  getRotationMatrix()->addChild(model.get());
}

// shooting
void LightTank::Shoot()
{
  // checking for delay in shooting
  if (getShotDelayTimer()->hasExpired())
  {
    osg::ref_ptr<Projectile> prj = new Projectile(GetXCoord() - 4, 
                                                  -4, 
                                                  GetZCoord() - 4,
                                                  4, 
                                                  GetCurDir(), 
                                                  *this, 
                                                  GetVehicleList(),
                                                  GetTileMap(),
                                                  GetToDeleteList(),
                                                  GetMainWindow());
    getParent(0)->addChild(prj.get());
    prj->setName(getName() + " - projectile");
    // updating the delay timer
    getShotDelayTimer()->setRemainingTime(SHOT_TIMEOUT);
  }
}