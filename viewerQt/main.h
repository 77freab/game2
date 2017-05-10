#pragma once

#include <QBoxLayout>
#include <QEvent>
#include <QWidget>
#include <QPushButton>
#include <QTimer>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QTreeWidget>

#include <osgQt/GraphicsWindowQt>
#include <osgViewer/ViewerEventHandlers>

#include <SDL.h>

#include "mapBuilder.h"
#include "lightTank.h"
#include "heavyTank.h"
#include "motorcycle.h"

class keyboardEventHandler;

// custom event for updating killcount
class vehicleKilledSomebody : public QEvent
{
public:
  vehicleKilledSomebody(const int player, const int killCount);
  int GetKillCount() const;
  int GetPlayer() const;
private:
  int _killCount;
  int _player;
};

// custom event for player respawn
class vehicleNeedRespawn : public QEvent
{
public:
  vehicleNeedRespawn(const osg::ref_ptr<vehicle> vehicle);
  osg::ref_ptr<vehicle> GetVehicle() const;
private:
  osg::ref_ptr<vehicle> _vehicle;
};

// main window
class ViewerWidget : public QWidget
{
public:
  ViewerWidget(QWidget* parent = 0, Qt::WindowFlags f = 0,
    osgViewer::ViewerBase::ThreadingModel threadingModel = osgViewer::Viewer::SingleThreaded);
private:
  void loadMap();
  QString controlsName(int controlDevice);
  void changeControls(int player, int controlDevice);
  void addPlayer();
  void clearPlaceForVehicle(int x, int z);
  void spawnPlayer(osg::ref_ptr<vehicle> vehicle);
  void restart();
  osg::ref_ptr<osg::Node> createScene();
  osgQt::GLWidget* addViewWidget(osgQt::GraphicsWindowQt* gw, osg::Node* scene);
  osgQt::GraphicsWindowQt* createGraphicsWindow(int x, int y, int w, int h);
  void testSDLJoystick();
  bool event(QEvent* event) override;

  QTimer _viewerUpdateTimer;
  osgQt::GLWidget* _viewerWidget;
  QHBoxLayout* _hLayout; // holding viewer + gui
  QVBoxLayout* _vLayout; // holding components of gui
  QTreeWidget* _playersList;
  QPushButton* _restartBtn;
  QPushButton* _addPlayerBtn;
  QPushButton* _openMapBtn;
  QPlainTextEdit* _console;

  osg::ref_ptr<osg::Node> _scene;
  osg::ref_ptr<osgViewer::Viewer> _viewer;
  osg::Vec2i _mapSize; // map size in tiles
  QString _fileName; // string for map file name
  mapBuilder mapMaker;

  int _numJoysticks; // number of connected joystics
  int _playerNum = 0; // current players number
  SDL_Joystick* _joy;
  direction _up, _down, _left, _right; // direction for vehicles depending on camera positon
  keyboardEventHandler* _keyboardEventHandler;

  std::vector<osg::ref_ptr<vehicle>> _vehicles; // vector containing all vihecles
  std::list<osg::Node*> _toDelete; // queue for deleting osg referensed objects
  std::map<osg::Vec2i, blockType> _typeMap; // list of coordinates and types of tiles located on these coordinates
  std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>> _tileMap; // list of coordinates and pointers to tiles located on these coordinates
};