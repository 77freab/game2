//#include "tank.h"
//#include <osg/Texture2D>
//#include <osg/ref_ptr>
//#include <osg/Geode>
//#include <osg/Geometry>
//#include <osg/MatrixTransform>
//#include <osgDB/ReadFile>
//
//#include <vector>
//#include <functional>
//#include <map>
//#include <QApplication>
//#include <QtMath>
//#include <QTimer>
//#include <QDeadlineTimer>
//
////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////TANK//////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
//
//const int SHOT_TIMEOUT = 300; // задержка между выстрелами в мс
//
//class tankCallback : public osg::NodeCallback
//{
//public:
//  void operator()(osg::Node*, osg::NodeVisitor*) override;
//private:
//  bool delay = false;
//};
//
//// конструктор
//tank::tank(int x, int z, int playerNum, int controlDevice, 
//  std::vector<osg::ref_ptr<tank>>* tank,
//  std::map<osg::Vec2i, blockType>* typeMap, 
//  std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>>* tileMap, 
//  std::list<osg::Node*>* toDelete, 
//  ViewerWidget* ViewerWindow)
//  : _delayBetweenShots(new QDeadlineTimer(SHOT_TIMEOUT)), _rMt(new MatrixTransform), _typeMap(typeMap),
//  _tileMap(tileMap), _toDelete(toDelete), _tank(tank), _controlDevice(controlDevice), _player(playerNum), 
//  _x(x), _z(z), _currentType(type::LIGHT), _ViewerWindow(ViewerWindow)
//{
//  this->setDataVariance(osg::Object::DYNAMIC);
//
//  osg::Matrix m; // перемещаем в точку спавна
//  m.makeTranslate(_x, 0, _z);
//  this->setMatrix(m); // наследуется от MatrixTransform для перемещения
//
//  osg::ref_ptr<osg::Node> node = // читаем модельку
//    osgDB::readNodeFile("./Resources/lightTank/bradle.3ds.15.scale.90,90,0.rot");
//  osg::ref_ptr<osg::Image> image = // читаем текстуру
//    osgDB::readImageFile("./Resources/lightTank/" + std::to_string(_player % 13) + ".png");
//
//  // устанавливаем текстуру
//  osg::StateSet* state = node->getOrCreateStateSet();
//  osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
//  texture->setImage(image.get());
//  state->setTextureAttributeAndModes(0, texture.get());
//  
//  _rMt->addChild(node.get()); // дополнительный MatrixTransform для поворота
//  this->addChild(_rMt.get());
//}
//
//// изменить тип танка
//void tank::ChangeType()
//{
//  _rMt->removeChildren(0, _rMt->getNumChildren());
//
//  osg::ref_ptr<osg::Node> node;
//  osg::ref_ptr<osg::Image> image;
//  
//  if (_typeForChange == type::LIGHT)
//  {
//    node = // читаем модельку
//      osgDB::readNodeFile("./Resources/lightTank/bradle.3ds.15.scale.90,90,0.rot");
//    image = // читаем текстуру
//      osgDB::readImageFile("./Resources/lightTank/" + std::to_string(_player % 13) + ".png");
//    _currentType = type::LIGHT;
//  }
//  else if (_typeForChange == type::HEAVY)
//  {
//    node = // читаем модельку
//      osgDB::readNodeFile("./Resources/heavyTank/Leopard 2A4.3ds.5.scale.90,180,0.rot");
//    image = // читаем текстуру
//      osgDB::readImageFile("./Resources/heavyTank/" + std::to_string(_player % 13) + ".bmp");
//    _currentType = type::HEAVY;
//  }
//  else if (_typeForChange == type::MOTO)
//  {
//    node = // читаем модельку
//      osgDB::readNodeFile("./Resources/motoTank/Scooter.3ds.15.scale.90,0,0.rot");
//    image = // читаем текстуру
//      osgDB::readImageFile("./Resources/motoTank/" + std::to_string(_player % 13) + ".bmp");
//    _currentType = type::MOTO;
//  }
//
//  // устанавливаем текстуру
//  osg::StateSet* state = node->getOrCreateStateSet();
//  osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
//  texture->setImage(image.get());
//  state->setTextureAttributeAndModes(0, texture.get());
//
//  _rMt->addChild(node.get());
//
//  _needTypeChange = false;
//}
//
//const int tank::GetPlayerNum() const
//{
//  return _player;
//}
//
//const int tank::AddKill()
//{
//  return ++_killCount;
//}
//
//const int tank::GetControlDevice() const
//{
//  return _controlDevice;
//}
//
//void tank::SetControlDevice(int cd)
//{
//  _controlDevice = cd;
//}
//
//const bool tank::NeedTypeChange() const
//{
//  return _needTypeChange;
//}
//
//void tank::SetNeedTypeChange(type newType)
//{
//  _needTypeChange = true;
//  _typeForChange = newType;
//}
//
//const tank::type tank::GetType() const
//{
//  return _currentType;
//}
//
//void tank::SetXCoord(int x)
//{
//  _x = x;
//}
//
//void tank::SetZCoord(int z)
//{
//  _z = z;
//}
//
//const int tank::GetXCoord() const
//{
//  return _x;
//}
//
//const int tank::GetZCoord() const
//{
//  return _z;
//}
//
//void tank::SetMovingDirection(direction dir)
//{
//  _go = true;
//  _goDir = dir;
//}
//
//void tank::Stop()
//{
//  _go = false;
//}
//
//const bool tank::IsEnabled() const
//{
//  return _enabled;
//}
//
//// просчет коллизий и движение
//void tank::Move()
//{
//  // вычисление точек коллизии в зависимости от направления танка
//  osg::Vec2i tankCollizionPt1, tankCollizionPt2;
//  osg::Vec2i tileCollizionPt1, tileCollizionPt2;
//  switch (_goDir)
//  {
//    case(direction::UP) :
//    {
//      tankCollizionPt1 = { _x - 7, _z + 7 };
//      tankCollizionPt2 = { _x + 7, _z + 7 };
//      tileCollizionPt1 = { (_x - 8) / 8, (_z + 8) / 8 };
//      tileCollizionPt2 = { static_cast<int>(ceil((_x - 1) / 8.)), (_z + 8) / 8 };
//      break;
//    }
//    case(direction::DOWN) :
//    {
//      tankCollizionPt1 = { _x - 7, _z - 7 };
//      tankCollizionPt2 = { _x + 7, _z - 7 };
//      tileCollizionPt1 = { (_x - 8) / 8, (_z - 8) / 8 };
//      tileCollizionPt2 = { static_cast<int>(ceil((_x - 1) / 8.)), (_z - 8) / 8 };
//      break;
//    }
//    case(direction::LEFT) :
//    {
//      tankCollizionPt1 = { _x - 7, _z + 7 };
//      tankCollizionPt2 = { _x - 7, _z - 7 };
//      tileCollizionPt1 = { (_x - 8) / 8, (_z - 8) / 8 };
//      tileCollizionPt2 = { (_x - 8) / 8, static_cast<int>(ceil((_z) / 8.)) };
//      break;
//    }
//    case(direction::RIGHT) :
//    {
//      tankCollizionPt1 = { _x + 7, _z + 7 };
//      tankCollizionPt2 = { _x + 7, _z - 7 };
//      tileCollizionPt1 = { (_x + 8) / 8, (_z - 8) / 8 };
//      tileCollizionPt2 = { (_x + 8) / 8, static_cast<int>(ceil((_z) / 8.)) };
//      break;
//    }
//  }
//
//  std::map<osg::Vec2i, blockType>::const_iterator a, b;
//  bool aGo = false, bGo = false, tStop = false;
//
//  // определение коллизий с блоками
//  if ((a = _typeMap->find(tileCollizionPt1)) != _typeMap->end())
//  {
//    if ((*a).second == blockType::ICE)
//    {
//      aGo = true;
//      // ICE !!!!!!!!!!!!!
//    }
//    if ((*a).second == blockType::BUSHES)
//      aGo = true;
//  }
//  else
//    aGo = true;
//  if ((b = _typeMap->find(tileCollizionPt2)) != _typeMap->end())
//  {
//    if ((*b).second == blockType::ICE)
//    {
//      bGo = true;
//      // ICE !!!!!!!!!!!!!
//    }
//    if ((*b).second == blockType::BUSHES)
//      bGo = true;
//  }
//  else
//    bGo = true;
//
//  // если не было коллизий с блоками определяет коллизии с другими танками
//  if (aGo && bGo)
//  {
//    // цикл по всем танкам
//    for (auto it = _tank->cbegin(); it != _tank->end(); ++it)
//    {
//      if ((*it).get() != this && (*it)->_enabled)
//      {
//        if (tankCollizionPt1[0] > (*it)->_x - 8 && tankCollizionPt1[0] < (*it)->_x + 8 &&
//          tankCollizionPt1[1] > (*it)->_z - 8 && tankCollizionPt1[1] < (*it)->_z + 8)
//          tStop = true; // впереди танк
//        if (tankCollizionPt2[0] > (*it)->_x - 8 && tankCollizionPt2[0] < (*it)->_x + 8 &&
//          tankCollizionPt2[1] > (*it)->_z - 8 && tankCollizionPt2[1] < (*it)->_z + 8)
//          tStop = true;
//      }
//    }
//    // впереди чисто, двигаемся
//    if (!tStop)
//    {
//      if (_currentType == type::HEAVY)
//      {
//        if (_goDir == direction::UP)
//          _z++;
//        if (_goDir == direction::DOWN)
//          _z--;
//        if (_goDir == direction::LEFT)
//          _x--;
//        if (_goDir == direction::RIGHT)
//          _x++;
//      }
//      else if ((_currentType == type::LIGHT))
//      {
//        if (_goDir == direction::UP)
//          _z+=2;
//        if (_goDir == direction::DOWN)
//          _z-=2;
//        if (_goDir == direction::LEFT)
//          _x-=2;
//        if (_goDir == direction::RIGHT)
//          _x+=2;
//      }
//      else if ((_currentType == type::MOTO))
//      {
//        if (_goDir == direction::UP)
//          _z += 3;
//        if (_goDir == direction::DOWN)
//          _z -= 3;
//        if (_goDir == direction::LEFT)
//          _x -= 3;
//        if (_goDir == direction::RIGHT)
//          _x += 3;
//      }
//    }
//  }
//
//  osg::Matrix mR;
//
//  if (_curDir != _goDir) // если танк поворачивает
//  {
//    switch (_goDir)
//    {
//      case(direction::UP) :
//      {
//        // при повороте позиция танка слегка "исправляется" для легкого вхождения в повороты
//        if (_x % 8 >= 4) 
//          _x = (_x / 8) * 8 + 8;
//        else
//          _x = (_x / 8) * 8;
//        // собственно поворот
//        mR.makeRotate(0, osg::Vec3(0, -1, 0));
//        _rMt->setMatrix(mR);
//        break;
//      }
//      case(direction::DOWN) :
//      {
//        if (_x % 8 >= 4)
//          _x = (_x / 8) * 8 + 8;
//        else
//          _x = (_x / 8) * 8;
//        mR.makeRotate(osg::PI, osg::Vec3(0, -1, 0));
//        _rMt->setMatrix(mR);
//        break;
//      }
//      case(direction::LEFT) :
//      {
//        if (_z % 8 >= 4)
//          _z = (_z / 8) * 8 + 8;
//        else
//          _z = (_z / 8) * 8;
//        mR.makeRotate(osg::PI_2, osg::Vec3(0, -1, 0));
//        _rMt->setMatrix(mR);
//        break;
//      }
//      case(direction::RIGHT) :
//      {
//        if (_z % 8 >= 4)
//          _z = (_z / 8) * 8 + 8;
//        else
//          _z = (_z / 8) * 8;
//        mR.makeRotate(-osg::PI_2, osg::Vec3(0, -1, 0));
//        _rMt->setMatrix(mR);
//        break;
//      }
//    }
//    _curDir = _goDir; // новое текущее направление
//  }
//
//  // перемещаем танк
//  osg::Matrix mT;
//  mT.makeTranslate(_x, 0, _z);
//  this->setMatrix(mT);
//}
//
//void tank::Enable()
//{
//  _clb = new tankCallback;
//  this->setUpdateCallback(_clb);
//  _delayBetweenShots->setRemainingTime(SHOT_TIMEOUT);
//  _enabled = true;
//}
//
//void tank::Disable()
//{
//  this->removeUpdateCallback(_clb); // чтоб не мог двигаться
//  _clb = nullptr;
//  _delayBetweenShots->setRemainingTime(-1); // чтоб не мог стрелять
//  _enabled = false; // чтоб коллизии для него не расчитывались
//}
//
//const direction tank::CurDir() const
//{
//  return _curDir;
//}
//
//const bool tank::NeedToGo() const
//{
//  return _go;
//}
//
//// стрельба
//void tank::Shoot()
//{
//  // обеспечиваем задержку при стрельбе
//  if (_delayBetweenShots->hasExpired())
//  {
//    if (_currentType == type::MOTO)
//    {
//      osg::ref_ptr<bomb> droppedBomb = new bomb(_x, -4, _z, this, _tank, _typeMap, _tileMap, _toDelete, _ViewerWindow);
//      this->getParent(0)->addChild(droppedBomb.get());
//      droppedBomb->setName(this->getName() + " - bomb");
//    }
//    else
//    {
//      osg::ref_ptr<projectile> prj = new projectile(_x - 4, -4, _z - 4, _curDir, this,
//        _tank, _typeMap, _tileMap, _toDelete, _ViewerWindow);
//      this->getParent(0)->addChild(prj.get());
//      prj->setName(this->getName() + " - projectile");
//    }
//    // обновляем таймер
//    _delayBetweenShots->setRemainingTime(SHOT_TIMEOUT);
//  }
//}
//
//void tankCallback::operator()(osg::Node* nd, osg::NodeVisitor* ndv)
//{
//  tank* tnk = dynamic_cast<tank*>(nd);
//  if (!delay)
//    if (tnk->NeedToGo())
//      tnk->Move();
//  delay = !delay;
//  traverse(nd, ndv);
//}