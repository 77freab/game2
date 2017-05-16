#include <osg/Texture2D>
#include <osgDB/ReadFile>

#include "bomb.h"
#include "motorcycle.h"

const int MAX_NUM_BOMBS = 5;

Motorcycle::Motorcycle( int x, 
                        int z, 
                        int playerNum, 
                        int controlDevice,
                        std::vector<osg::ref_ptr<Vehicle>>& vehicles,
                        std::vector<std::vector<osg::ref_ptr<Tile>>>& tileMap,
                        std::list<osg::Node*>& toDelete,
                        ViewerWidget& ViewerWindow, 
                        int killCount) : 
  Vehicle(x, z, 3, Type::MOTO, playerNum, controlDevice, 
  killCount, vehicles, tileMap, toDelete, ViewerWindow)
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

  getRotationMt()->addChild(model.get());
}

// shooting
void Motorcycle::Shoot()
{
  // checking for delay in shooting
  if (getShotDelayTimer()->hasExpired() && _numBombs < MAX_NUM_BOMBS)
  {
    _numBombs++;
    osg::ref_ptr<Bomb> droppedBomb = new Bomb(GetXCoord(), 
                                              -4, 
                                              GetZCoord(), 
                                              *this, 
                                              GetVehicleList(),
                                              GetTileMap(),
                                              GetToDeleteList(),
                                              GetMainWindow());
    getParent(0)->addChild(droppedBomb.get());
    droppedBomb->setName(getName() + " - bomb");
    // updating the delay timer
    getShotDelayTimer()->setRemainingTime(SHOT_TIMEOUT);
  }
}