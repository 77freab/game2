#pragma once

#include <QEvent>
#include <QWidget>
#include <QBoxLayout>
#include <QPushButton>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QTreeWidget>
#include <QTimer>

#include <osgQt/GraphicsWindowQt>
#include <osgViewer/ViewerEventHandlers>

#include <SDL.h>

class tank;
class keyboardEventHandler;
enum class direction;

// ��������� ������� ��� ���������� �������� �������
class tankKilledSomebody : public QEvent
{
public:
  tankKilledSomebody(const int player, const int killCount);
  int GetKillCount() const;
  int GetPlayer() const;
private:
  int _killCount;
  int _player;
};

// ��������� ������� � ������������� ����������� �����
class tankNeedRespawn : public QEvent
{
public:
  tankNeedRespawn(const osg::ref_ptr<tank> tank);
  osg::ref_ptr<tank> GetTank() const;
private:
  osg::ref_ptr<tank> _tank;
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
  void clearPlaceForTank(int x, int z);
  void spawnPlayer(osg::ref_ptr<tank> tank);
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

  std::vector<osg::ref_ptr<tank>> _tank; // ������ ���������� ��� �����
  std::list<osg::Node*> _toDelete; // ������� �� ��������
  std::map<osg::Vec2i, blockType> _typeMap; // ������ ��������� � ����� ������ ������������� �� ��� �����������
  std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>> _tileMap; // ������ ��������� � ������ �� ���� ����� ������������� �� ���� �����������
};