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

// ��������� ������� ��� ���������� �������� �������
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

// ��������� ������� � ������������� ����������� �����
class vehicleNeedRespawn : public QEvent
{
public:
  vehicleNeedRespawn(const osg::ref_ptr<vehicle> vehicle);
  osg::ref_ptr<vehicle> GetVehicle() const;
private:
  osg::ref_ptr<vehicle> _vehicle;
};

// ������� ����
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
  QHBoxLayout* _hLayout; // ��������� ������ + ������ ����������
  QVBoxLayout* _vLayout; // ��������� ���������� ������ ����������
  QTreeWidget* _playersList;
  QPushButton* _restartBtn;
  QPushButton* _addPlayerBtn;
  QPushButton* _openMapBtn;
  QPlainTextEdit* _console;

  osg::ref_ptr<osg::Node> _scene;
  osg::ref_ptr<osgViewer::Viewer> _viewer;
  osg::Vec2i _mapSize; // ������ ����� � ������
  QString _fileName; // ������ ��� ����� ����� � ������
  mapBuilder mapMaker;

  int _numJoysticks; // ���-�� ������������ ����������
  int _playerNum = 0; // ������� ���-�� �������
  SDL_Joystick* _joy;
  direction _up, _down, _left, _right; // ����������� �������� ��������� �� ��������� ������
  keyboardEventHandler* _keyboardEventHandler;

  std::vector<osg::ref_ptr<vehicle>> _vehicles; // ������ ���������� ��� �����
  std::list<osg::Node*> _toDelete; // ������� �� ��������
  std::map<osg::Vec2i, blockType> _typeMap; // ������ ��������� � ����� ������ ������������� �� ��� �����������
  std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>> _tileMap; // ������ ��������� � ������ �� ���� ����� ������������� �� ���� �����������
};