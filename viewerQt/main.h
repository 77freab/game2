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

class KeyboardEventHandler;
class VehicleControls;

//! custom event for updating killcount
class VehicleKilledSomebody : public QEvent
{
public:
  VehicleKilledSomebody(const int player, const int killCount);
  int GetKillCount() const;
  int GetPlayer() const;
private:
  int _killCount;
  int _player;
};

//! custom event for player respawn
class VehicleNeedRespawn : public QEvent
{
public:
  VehicleNeedRespawn(const osg::ref_ptr<Vehicle> vehicle);
  osg::ref_ptr<Vehicle> GetVehicle() const;
private:
  osg::ref_ptr<Vehicle> _vehicle;
};

//! main window
class ViewerWidget : public QWidget
{
public:
  ViewerWidget( QWidget* parent = 0, 
                Qt::WindowFlags f = 0,
                osgViewer::ViewerBase::ThreadingModel threadingModel = osgViewer::Viewer::SingleThreaded);
private:
  void addControlDeviceButton(QPushButton* parentBtn, 
                              QMenu& menu, 
                              int player, 
                              int controlDevice);
  void vehicleControlsInit();
  void loadMap();
  QString controlsName(int controlDevice);
  void changeControls(int player, int controlDevice);
  void addPlayer();
  void clearPlaceForVehicle(int x, int z);
  void spawnPlayer(osg::ref_ptr<Vehicle> vehicle);
  void restart();
  osg::ref_ptr<osg::Node> createScene();
  osgQt::GLWidget* addViewWidget(osgQt::GraphicsWindowQt* gw, osg::Node* scene);
  osgQt::GraphicsWindowQt* createGraphicsWindow(int x, int y, int w, int h);
  void testSDLJoystick();
  bool event(QEvent* event) override;

private:
  QTimer _viewerUpdateTimer;
  osgQt::GLWidget* _viewerWidget;
  QHBoxLayout* _hLayout; //!< holding viewer + gui
  QVBoxLayout* _vLayout; //!< holding components of gui
  QTreeWidget* _playersList;
  QPushButton* _restartBtn;
  QPushButton* _addPlayerBtn;
  QPushButton* _openMapBtn;
  QPlainTextEdit* _console;

  osg::ref_ptr<osg::Node> _scene;
  osg::ref_ptr<osgViewer::Viewer> _viewer;
  osg::Vec2i _mapSize; //!< map size in tiles
  QString _fileName; //!< string for map file name
  MapBuilder mapMaker;

  int _numJoysticks; //!< number of connected joystics
  int _playerNum = 0; //!< current players number
  SDL_Joystick* _joy;
  //! direction for vehicles depending on camera positon
  Vehicle::Direction _up, _down, _left, _right;
  KeyboardEventHandler* _keyboardEventHandler;

  //! vector containing all vihecles
  std::vector<osg::ref_ptr<Vehicle>> _vehicles;
  //! vector of objects handling keyboard control of vehicles
  std::vector<VehicleControls*> _vehicleControls;
  //! queue for deleting osg referensed objects
  std::list<osg::Node*> _toDelete;
  //! 2D vector of pointers to tiles located on coordinates same as indexes of vector
  std::vector<std::vector<osg::ref_ptr<Tile>>> _tileMap;
};