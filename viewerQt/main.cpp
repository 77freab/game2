#include <QApplication>
#include <QBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QTreeWidget>
#include <QFileDialog>
#include <QMenu>

#include <osgViewer/ViewerEventHandlers>
#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>
#include <osgQt/GraphicsWindowQt>
#include <osgUtil/Optimizer>

#include <list>
#include <cstdlib>
#include <ctime>

#include "tank.h"

#include <SDL.h>
#undef main

const int SIDE_PANEL_SIZE = 405;

class ViewerWidget : public QWidget, public osgGA::GUIEventHandler
{
public:
  ViewerWidget(QWidget* parent = 0, Qt::WindowFlags f = 0, osgViewer::ViewerBase::ThreadingModel threadingModel = osgViewer::Viewer::SingleThreaded)
    : QWidget(parent, f), _viewer(new osgViewer::Viewer)
  {
    srand(time(NULL));
    _viewer->setThreadingModel(threadingModel);
    _viewer->setKeyEventSetsDone(0);
    _fileName = "./Resources/maps/original1.xml";

    _hLayout = new QHBoxLayout;

    _scene = createScene();
    _widget = addViewWidget(createGraphicsWindow(0, 0, 800, 700), _scene.get());
    
    _hLayout->addWidget(_widget);

    _vLayout = new QVBoxLayout;

    _restartBtn = new QPushButton("RESTART");
    QFont font;
    font.setPointSize(20);
    _restartBtn->setMaximumWidth(SIDE_PANEL_SIZE);
    _restartBtn->setMinimumHeight(50);
    _restartBtn->setFont(font);

    _vLayout->addWidget(_restartBtn);

    _openMapBtn = new QPushButton("Open map");
    _openMapBtn->setMaximumWidth(SIDE_PANEL_SIZE);

    _vLayout->addWidget(_openMapBtn);

    _console = new QPlainTextEdit;
    _console->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    _console->setTextInteractionFlags(Qt::NoTextInteraction);
    _console->setMaximumWidth(SIDE_PANEL_SIZE);

    _vLayout->addWidget(_console);

    _treeWidget = new QTreeWidget;
    _treeWidget->setMaximumWidth(SIDE_PANEL_SIZE);
    _treeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _treeWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    QTreeWidgetItem *header = new QTreeWidgetItem();
    _treeWidget->setHeaderItem(header);
    header->setText(0, QString::fromLocal8Bit("Игрок"));
    header->setText(1, QString::fromLocal8Bit("Убийства"));
    header->setText(2, QString::fromLocal8Bit("Тип танка"));
    header->setText(3, QString::fromLocal8Bit("Управление"));
    header->setText(4, QString::fromLocal8Bit("Заспавнить"));
    header->setTextAlignment(0, Qt::AlignCenter);
    header->setTextAlignment(1, Qt::AlignCenter);
    header->setTextAlignment(2, Qt::AlignCenter);
    header->setTextAlignment(3, Qt::AlignCenter);
    header->setTextAlignment(4, Qt::AlignCenter);
    _treeWidget->setColumnWidth(0, 50);
    _treeWidget->setColumnWidth(1, 65);
    _treeWidget->setColumnWidth(2, 70);
    _treeWidget->setColumnWidth(3, 120);
    _treeWidget->setColumnWidth(4, 80);

    _vLayout->addWidget(_treeWidget);

    _addPlayerBtn = new QPushButton;
    _addPlayerBtn->setText(QString::fromLocal8Bit("Добавить игрока"));

    _vLayout->addWidget(_addPlayerBtn);

    _hLayout->addLayout(_vLayout);

    testSDLJoystick();

    setLayout(_hLayout);

    connect(_addPlayerBtn, &QPushButton::clicked, this, &ViewerWidget::addPlayer);
    connect(_restartBtn, &QPushButton::clicked, this, &ViewerWidget::restart);
    connect(_openMapBtn, &QPushButton::clicked, this, &ViewerWidget::loadMap);
    connect(&_timer, SIGNAL(timeout()), this, SLOT(update()));
    _timer.start(10);
  }

  void loadMap()
  {
    _fileName = QFileDialog::getOpenFileName(this, "Open map","./Resources/maps","XML files (*.xml)");
    if (_fileName != nullptr)
      restart();
  }

  QString controlsName(int controlDevice)
  {
    if (controlDevice == -2)
      return QString("WASD + Space");
    else if (controlDevice == -1)
      return QString("Arrows + Num 0");
    else
      return QString("Joystick " + QString::number(controlDevice));
  }

  void changeControls(int player, int controlDevice)
  {
    // controlDevice = -2 is WASD + Space
    // controlDevice = -1 is Arrows + Num 0
    int swapControl = _tank[player]->_joyNum;

    for (int i = 0; i < _playerNum; i++)
    {
      if (_tank[i]->_joyNum == controlDevice)
      {
        _tank[i]->_joyNum = swapControl;
        QPushButton* btn = dynamic_cast<QPushButton*>(_treeWidget->itemWidget(_treeWidget->topLevelItem(i), 3));
        btn->setText(controlsName(swapControl));
        if (swapControl == -2)
          _wasdTank = _tank[i];
        else if (swapControl == -1)
          _arrowsTank = _tank[i];
      }
      else if (swapControl == -2)
        _wasdTank = nullptr;
      else if (swapControl == -1)
        _arrowsTank = nullptr;
    }

    _tank[player]->_joyNum = controlDevice;

    if (controlDevice == -2)
      _wasdTank = _tank[player];
    else if (controlDevice == -1)
      _arrowsTank = _tank[player];
  }

  void addPlayer()
  {
    // нельзя добавить игроков больше чем методов ввода (в т.ч. с клавиатуры)
    if (_playerNum - 2 < _numJoysticks)
    {
      int player = _playerNum;

      int x = rand() % (MAP_SIZE[0] - 8) + 3;
      int z = rand() % (MAP_SIZE[1] - 6) + 3;

      _tank.push_back(new tank(x * 8, z * 8, std::to_string(player % 13), player - 2, &_tank, &_typeMap, &_tileMap, &_toDelete));
      _tank.back()->setName(_scene->getName() + " - " + std::to_string(player) + " player tank");

      QTreeWidgetItem *item = new QTreeWidgetItem(_treeWidget);

      item->setIcon(0, QIcon("./Resources/tank/tank" + QString::number(_playerNum % 13) + ".bmp"));
      item->setText(1, QString::fromLocal8Bit("0"));

      QPushButton* tankTypeBtn = new QPushButton(QString::fromLocal8Bit("Легкий")); // тип танка
      _treeWidget->setItemWidget(item, 2, tankTypeBtn);

      QPushButton* controlsBtn = new QPushButton(controlsName(player - 2)); // управление
      // по умолчанию танки 0 и 1 управляются с WASD и стрелок соответственно
      if (player == 0)
        _wasdTank = _tank.back();
      if (player == 1)
        _arrowsTank = _tank.back();

      QMenu* controlsMenu = new QMenu;

      // кнопка смены управления на WASD
      QAction* act = new QAction;
      connect(act, &QAction::triggered, controlsBtn, [controlsBtn, act] { controlsBtn->setText(act->text()); });
      connect(act, &QAction::triggered, this, [this, player] { changeControls(player, -2); });
      act->setText(QString::fromLocal8Bit("WASD + Space"));
      controlsMenu->addAction(act);

      // кнопка смены управления на стрелки
      act = new QAction;
      connect(act, &QAction::triggered, controlsBtn, [controlsBtn, act] { controlsBtn->setText(act->text()); });
      connect(act, &QAction::triggered, this, [this, player] { changeControls(player, -1); });
      act->setText(QString::fromLocal8Bit("Arrows + Num 0"));
      controlsMenu->addAction(act);

      // кнопки смены управления на любой из джойстиков
      for (int i = 0; i < _numJoysticks; i++)
      {
        act = new QAction;
        connect(act, &QAction::triggered, controlsBtn, [controlsBtn, act] { controlsBtn->setText(act->text()); });
        connect(act, &QAction::triggered, this, [this, player, i] { changeControls(player, i); });
        act->setText(QString::fromLocal8Bit("Joystick ") + QString::number(i));
        controlsMenu->addAction(act);
      }

      controlsBtn->setMenu(controlsMenu);
      _treeWidget->setItemWidget(item, 3, controlsBtn);

      QPushButton* spawnBtn = new QPushButton("GO");
      _treeWidget->setItemWidget(item, 4, spawnBtn);

      connect(spawnBtn, &QPushButton::clicked, this, [this, player] { spawnPlayer(player); });
      _playerNum++;
      item->setTextAlignment(0, Qt::AlignCenter);
      item->setTextAlignment(1, Qt::AlignCenter);
      item->setTextAlignment(2, Qt::AlignCenter);
      item->setTextAlignment(3, Qt::AlignCenter);
      item->setTextAlignment(4, Qt::AlignCenter);
    }
  }

  void spawnPlayer(const int player)
  {
    int x = _tank[player]->_x/8;
    int z = _tank[player]->_z/8;

    // если на месте спавна танка есть блоки, удаляем их
    std::map<osg::Vec2i, blockType>::const_iterator a;
    if ((a = _typeMap.find({ x - 1, z })) != _typeMap.end())
    {
      _toDelete.push_back(_tileMap[{ x - 1, z }]);
      _typeMap.erase(a);
    }
    if ((a = _typeMap.find({ x, z - 1 })) != _typeMap.end())
    {
      _toDelete.push_back(_tileMap[{ x, z - 1 }]);
      _typeMap.erase(a);
    }
    if ((a = _typeMap.find({ x - 1, z - 1 })) != _typeMap.end())
    {
      _toDelete.push_back(_tileMap[{ x - 1, z - 1 }]);
      _typeMap.erase(a);
    }
    if ((a = _typeMap.find({ x, z })) != _typeMap.end())
    {
      _toDelete.push_back(_tileMap[{ x, z }]);
      _typeMap.erase(a);
    }

    // добавляем танк
    _scene->asGroup()->addChild(_tank[player]);
    _tank[player]->enable();

    // при убистве счетчик будет обновляться
    connect(_tank[player], &tank::smbdyKilled, this, [this, player](int killCount)
      { _treeWidget->topLevelItem(player)->setText(1, QString::number(killCount)); });
    // после уничтожения танк будет респавниться через какое-то время
    connect(_tank[player], &tank::enemyNeedRespawn, this, &ViewerWidget::spawnPlayer);

    // делаем кнопку спавна неактивной
    _treeWidget->itemWidget(_treeWidget->topLevelItem(player), 4)->setEnabled(false);
  }

  void restart()
  {
    _typeMap.clear();
    _tileMap.clear();
    _toDelete.clear();
    _tank.clear();

    // отключаем все танки
    for (int i = 0; i < _playerNum; i++)
    {
      _tank[i]->disable();
    }

    _playerNum = 0;
    
    // делаем кнопку спавна опять активной и обнуляем счетчик убийств
    for (int i = 0; i < _treeWidget->topLevelItemCount(); i++)
    {
      QTreeWidgetItem* tempItem = _treeWidget->topLevelItem(i);
      tempItem->setText(1, "0");
      _treeWidget->itemWidget(tempItem, 4)->setEnabled(true);
    }

    _scene = createScene();
    osgQt::GLWidget* widget = addViewWidget(createGraphicsWindow(0, 0, 800, 700), _scene.get());;
    _hLayout->replaceWidget(_widget, widget);
    _widget = widget;
  }

  osg::ref_ptr<osg::Node> createScene()
  {
    osg::ref_ptr<osg::Group> scene = new osg::Group;
    scene->setName("main scene");
    osg::Vec2i createMap(osg::ref_ptr<osg::Group> scene,
      std::map<osg::Vec2i, blockType>& typeMap,
      std::map<osg::Vec2i, tile*>& tileMap, QString fileName);
    MAP_SIZE = createMap(scene, _typeMap, _tileMap, _fileName);

    osgUtil::Optimizer opt;
    opt.optimize(scene,
      osgUtil::Optimizer::FLATTEN_STATIC_TRANSFORMS |
      osgUtil::Optimizer::REMOVE_REDUNDANT_NODES |
      osgUtil::Optimizer::REMOVE_LOADED_PROXY_NODES |
      osgUtil::Optimizer::COMBINE_ADJACENT_LODS |
      osgUtil::Optimizer::SHARE_DUPLICATE_STATE |
      osgUtil::Optimizer::MERGE_GEOMETRY |
      osgUtil::Optimizer::CHECK_GEOMETRY |
      osgUtil::Optimizer::MAKE_FAST_GEOMETRY |
      osgUtil::Optimizer::SPATIALIZE_GROUPS |
      osgUtil::Optimizer::COPY_SHARED_NODES |
      //osgUtil::Optimizer::TRISTRIP_GEOMETRY | // разделяет на triangle strips
      osgUtil::Optimizer::TESSELLATE_GEOMETRY |
      osgUtil::Optimizer::OPTIMIZE_TEXTURE_SETTINGS |
      osgUtil::Optimizer::MERGE_GEODES |
      osgUtil::Optimizer::FLATTEN_BILLBOARDS |
      //osgUtil::Optimizer::TEXTURE_ATLAS_BUILDER | // текстуры танков артефачат
      osgUtil::Optimizer::STATIC_OBJECT_DETECTION |
      osgUtil::Optimizer::FLATTEN_STATIC_TRANSFORMS_DUPLICATING_SHARED_SUBGRAPHS |
      //osgUtil::Optimizer::INDEX_MESH | // разделяет на triangles
      osgUtil::Optimizer::VERTEX_POSTTRANSFORM |
      osgUtil::Optimizer::VERTEX_PRETRANSFORM
      );
    return scene.get();
  }

  osgQt::GLWidget* addViewWidget(osgQt::GraphicsWindowQt* gw, osg::Node* scene)
  {
    osg::Camera* camera = _viewer->getCamera();
    camera->setGraphicsContext(gw);

    const osg::GraphicsContext::Traits* traits = gw->getTraits();

    camera->setClearColor(osg::Vec4(0, 0, 0, 1));
    camera->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));
    camera->setViewMatrixAsLookAt({ -100, -440, -100 }, { 128, 0, 112 }, { 1, 1, 1 });
    camera->setProjectionMatrixAsPerspective(28.0f, 32./28, 1.0f, 10000.0f);

    _viewer->setSceneData(scene);
    //_viewer->addEventHandler(new osgViewer::StatsHandler);
    _viewer->addEventHandler(this);
    _viewer->setCameraManipulator(new osgGA::TrackballManipulator);
    gw->setTouchEventsEnabled(true);
    return gw->getGLWidget();
  }

  osgQt::GraphicsWindowQt* createGraphicsWindow(int x, int y, int w, int h)
  {
    osg::DisplaySettings* ds = osg::DisplaySettings::instance().get();
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->x = x;
    traits->y = y;
    traits->width = w;
    traits->height = h;
    traits->doubleBuffer = true;
    traits->alpha = ds->getMinimumNumAlphaBits();
    traits->stencil = ds->getMinimumNumStencilBits();
    traits->sampleBuffers = ds->getMultiSamples();
    traits->samples = ds->getNumMultiSamples();

    return new osgQt::GraphicsWindowQt(traits.get());
  }

  bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter&)
  {
    switch (ea.getEventType())
    {
    case(osgGA::GUIEventAdapter::KEYDOWN) :
    {
      if (_wasdTank != nullptr)
        switch (ea.getKey())
        {
          case(119) : // W
          {
            _pressedKeysP1[119] = true;
            _wasdTank->moveTo(direction::UP);
            break;
          }
          case(115) : // S
          {
            _pressedKeysP1[115] = true;
            _wasdTank->moveTo(direction::DOWN);
            break;
          }
          case(97) : // A
          {
            _pressedKeysP1[97] = true;
            _wasdTank->moveTo(direction::LEFT);
            break;
          }
          case(100) : // D
          {
            _pressedKeysP1[100] = true;
            _wasdTank->moveTo(direction::RIGHT);
            break;
          }
          case(32) : // SPACE
          {
            _wasdTank->shoot();
            break;
          }
        }
                 ////////////////////////////////////////////////////////////
      if (_arrowsTank != nullptr)
        switch (ea.getKey())
        {
          case(osgGA::GUIEventAdapter::KEY_Up) : // up
          {
            _pressedKeysP2[65362] = true;
            _arrowsTank->moveTo(direction::UP);
            break;
          }
          case(osgGA::GUIEventAdapter::KEY_Down) : // down
          {
            _pressedKeysP2[65364] = true;
            _arrowsTank->moveTo(direction::DOWN);
            break;
          }
          case(osgGA::GUIEventAdapter::KEY_Left) : // left
          {
            _pressedKeysP2[65361] = true;
            _arrowsTank->moveTo(direction::LEFT);
            break;
          }
          case(osgGA::GUIEventAdapter::KEY_Right) : // right
          {
            _pressedKeysP2[65363] = true;
            _arrowsTank->moveTo(direction::RIGHT);
            break;
          }
          case(osgGA::GUIEventAdapter::KEY_0) : // num0
          {
            _arrowsTank->shoot(); 
            break;
          }
        }
      return true;
    }
    case(osgGA::GUIEventAdapter::KEYUP) :
    {
      int key = ea.getKey();
      if (_wasdTank != nullptr)
        switch (key)
        {
          case(119) : // W
          case(115) : // S
          case(97) : // A
          case(100) : // D
          {
            _pressedKeysP1[key] = false;
            // если это была единственная нажатая клавиша то танк останавливается
            if (_pressedKeysP1[119])
              _wasdTank->moveTo(direction::UP);
            else if (_pressedKeysP1[115])
              _wasdTank->moveTo(direction::DOWN);
            else if (_pressedKeysP1[97])
              _wasdTank->moveTo(direction::LEFT);
            else if (_pressedKeysP1[100])
              _wasdTank->moveTo(direction::RIGHT);
            else
              _wasdTank->stop();
            break;
          }
        }
      if (_arrowsTank != nullptr)
        switch (key)
        {
          case(65362) : // up
          case(65364) : // down
          case(65361) : // left
          case(65363) : // right
          {
            _pressedKeysP2[key] = false;
            // если это была единственная нажатая клавиша то танк останавливается
            if (_pressedKeysP2[65362])
              _arrowsTank->moveTo(direction::UP);
            else if (_pressedKeysP2[65364])
              _arrowsTank->moveTo(direction::DOWN);
            else if (_pressedKeysP2[65361])
              _arrowsTank->moveTo(direction::LEFT);
            else if (_pressedKeysP2[65363])
              _arrowsTank->moveTo(direction::RIGHT);
            else
              _arrowsTank->stop();
            break;
          }
        }
      return true;
    }
    default:
      return false;
    }
  }

  void testSDLJoystick()
  {
    // Initialize the joystick subsystem
    SDL_InitSubSystem(SDL_INIT_JOYSTICK);
    _numJoysticks = SDL_NumJoysticks();

    // Check for joystick
    _console->insertPlainText(QString::fromLocal8Bit("Найдено ") + QString::number(_numJoysticks) + QString::fromLocal8Bit(" джойстиков:\n\n"));
    for (int joyNum = 0; joyNum < _numJoysticks; joyNum++)
    {
      _joy = SDL_JoystickOpen(joyNum);
      if (_joy)
      {
        _console->insertPlainText(
          "Opened Joystick " + QString::number(joyNum) +
          "\nName: " + QString(SDL_JoystickName(_joy)) +
          "\nNumber of Axes: " + QString::number(SDL_JoystickNumAxes(_joy)) +
          "\nNumber of Buttons: " + QString::number(SDL_JoystickNumButtons(_joy)) +
          "\nNumber of Balls: " + QString::number(SDL_JoystickNumBalls(_joy)) + "\n\n"
          );
      }
    }
    if (_numJoysticks == 0)
      _console->insertPlainText(QString::fromLocal8Bit("Не найден ни один джойстик\n"));
  }

  virtual void paintEvent(QPaintEvent* event)
  {
    _viewer->frame();

    while (!_toDelete.empty())
    {
      _toDelete.front()->getParent(0)->removeChild(_toDelete.front());
      _toDelete.pop_front();
    }

    for (auto it = _tank.cbegin(); it != _tank.end(); ++it)
    {
      if ((*it)->_joyNum < 0)
        continue;
      (*it)->stop();

      _joy = SDL_JoystickOpen((*it)->_joyNum);

      _x = _viewer->getCamera()->getViewMatrix()(3, 0);
      _z = _viewer->getCamera()->getViewMatrix()(3, 1);

      if (_x < 0 && _z < 0)
      {
        _up = direction::UP;
        _down = direction::DOWN;
        _left = direction::LEFT;
        _right = direction::RIGHT;
      }
      if (_x > 0 && _z < 0)
      {
        _up = direction::RIGHT;
        _down = direction::LEFT;
        _left = direction::UP;
        _right = direction::DOWN;
      }
      if (_x > 0 && _z > 0)
      {
        _up = direction::DOWN;
        _down = direction::UP;
        _left = direction::RIGHT;
        _right = direction::LEFT;
      }
      if (_x < 0 && _z > 0)
      {
        _up = direction::LEFT;
        _down = direction::RIGHT;
        _left = direction::DOWN;
        _right = direction::UP;
      }

      SDL_JoystickUpdate();

      _hAxis = SDL_JoystickGetAxis(_joy, 0); // вертикальная ось - первая
      _vAxis = SDL_JoystickGetAxis(_joy, SDL_JoystickNumAxes(_joy) - 1); // горизонтальная ось - последняя
      _startBtn = SDL_JoystickGetButton(_joy, 9);
      _fireBtn = SDL_JoystickGetButton(_joy, 2);

      if (_vAxis < -20000) // UP button
        (*it)->moveTo(_up);

      if (_vAxis > 20000) // DOWN button
        (*it)->moveTo(_down);

      if (_hAxis < -20000) // LEFT button
        (*it)->moveTo(_left);

      if (_hAxis > 20000) // RIGHT button
        (*it)->moveTo(_right);

      if (_fireBtn) // FIRE button
        (*it)->shoot();
    }
  }

protected:
  QTimer _timer;
  osgQt::GLWidget* _widget;
  QHBoxLayout* _hLayout;
  QVBoxLayout* _vLayout;
  QTreeWidget* _treeWidget;
  QPushButton* _restartBtn;
  QPushButton* _addPlayerBtn;
  QPushButton* _openMapBtn;
  QPlainTextEdit* _console;
  int _numJoysticks;
  QString _fileName;
  std::vector<osg::ref_ptr<tank>> _tank;
  tank* _wasdTank = nullptr;
  tank* _arrowsTank = nullptr;
  std::map<int, bool> _pressedKeysP1; // для игры с клавиатуры
  std::map<int, bool> _pressedKeysP2;
  osg::ref_ptr<osgViewer::Viewer> _viewer;
  osg::ref_ptr<osg::Node> _scene;
  int _playerNum = 0;
  osg::Vec2i MAP_SIZE;

  SDL_Joystick* _joy;
  int _hAxis, _vAxis;
  bool _startBtn, _fireBtn;
  direction _up, _down, _left, _right;
  int _x, _z;

  std::map<osg::Vec2i, blockType> _typeMap;
  std::map<osg::Vec2i, tile*> _tileMap;
  std::list<osg::Node*> _toDelete;
};

int main(int argc, char** argv)
{
  osg::ArgumentParser arguments(&argc, argv);

#if QT_VERSION >= 0x050000
  // Qt5 is currently crashing and reporting "Cannot make QOpenGLContext current in a different thread" when the viewer is run multi-threaded, this is regression from Qt4
  osgViewer::ViewerBase::ThreadingModel threadingModel = osgViewer::ViewerBase::SingleThreaded;
#else
  osgViewer::ViewerBase::ThreadingModel threadingModel = osgViewer::ViewerBase::CullDrawThreadPerContext;
#endif

  while (arguments.read("--SingleThreaded")) threadingModel = osgViewer::ViewerBase::SingleThreaded;
  while (arguments.read("--CullDrawThreadPerContext")) threadingModel = osgViewer::ViewerBase::CullDrawThreadPerContext;
  while (arguments.read("--DrawThreadPerContext")) threadingModel = osgViewer::ViewerBase::DrawThreadPerContext;
  while (arguments.read("--CullThreadPerCameraDrawThreadPerContext")) threadingModel = osgViewer::ViewerBase::CullThreadPerCameraDrawThreadPerContext;

#if QT_VERSION >= 0x040800
  // Required for multithreaded QGLWidget on Linux/X11, see http://blog.qt.io/blog/2011/06/03/threaded-opengl-in-4-8/
  if (threadingModel != osgViewer::ViewerBase::SingleThreaded)
    QApplication::setAttribute(Qt::AA_X11InitThreads);
#endif

  QApplication app(argc, argv);
  ViewerWidget* viewWidget = new ViewerWidget(0, Qt::Widget, threadingModel);
  viewWidget->setGeometry(100, 100, 800 + 30 + SIDE_PANEL_SIZE, 700);
  viewWidget->setWindowTitle("World of Tanks 2");
  viewWidget->show();
  return app.exec();
}
