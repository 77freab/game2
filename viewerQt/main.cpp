#include <QApplication>
#include <QBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QTreeWidget>
#include <QFileDialog>

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

//extern std::map<osg::Vec2i, blockType> map;
//std::list<osg::Node*> toDelete;

class ViewerWidget : public QWidget, public osgGA::GUIEventHandler
{
public:
  ViewerWidget(QWidget* parent = 0, Qt::WindowFlags f = 0, osgViewer::ViewerBase::ThreadingModel threadingModel = osgViewer::Viewer::SingleThreaded) : QWidget(parent, f), _viewer(new osgViewer::Viewer)
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
    _restartBtn->setMaximumWidth(250);
    _restartBtn->setMinimumHeight(50);
    _restartBtn->setFont(font);

    _vLayout->addWidget(_restartBtn);

    _openMapBtn = new QPushButton("Open map");
    _openMapBtn->setMaximumWidth(250);

    _vLayout->addWidget(_openMapBtn);

    _console = new QPlainTextEdit;
    _console->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    _console->setTextInteractionFlags(Qt::NoTextInteraction);
    _console->setMaximumWidth(250);

    _vLayout->addWidget(_console);

    _treeWidget = new QTreeWidget;
    _treeWidget->setMaximumWidth(250);
    _treeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _treeWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    QTreeWidgetItem *header = new QTreeWidgetItem();
    _treeWidget->setHeaderItem(header);
    header->setText(0, QString::fromLocal8Bit("»грок"));
    header->setText(1, QString::fromLocal8Bit(" ол-во убийств"));
    header->setText(2, QString::fromLocal8Bit("«аспавнить"));
    header->setTextAlignment(0, Qt::AlignCenter);
    header->setTextAlignment(1, Qt::AlignCenter);
    header->setTextAlignment(2, Qt::AlignCenter);
    _treeWidget->setColumnWidth(0, 50);
    _treeWidget->setColumnWidth(1, 100);
    _treeWidget->setColumnWidth(2, 80);

    _vLayout->addWidget(_treeWidget);

    _addPlayerBtn = new QPushButton;
    _addPlayerBtn->setText(QString::fromLocal8Bit("ƒобавить игрока"));

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
  }

  void addPlayer()
  {
    QTreeWidgetItem *item = new QTreeWidgetItem(_treeWidget);
    int player = _playerNum;
    item->setIcon(0, QIcon("./Resources/tank/tank" + QString::number(_playerNum % 13) + ".bmp"));
    item->setText(1, QString::fromLocal8Bit("0"));
    QPushButton* spawnBtn = new QPushButton("GO");
    _treeWidget->setItemWidget(item, 2, spawnBtn);
    connect(spawnBtn, &QPushButton::clicked, this, [this, player] { spawnPlayer(player); });
    _playerNum++;
    item->setTextAlignment(0, Qt::AlignCenter);
    item->setTextAlignment(1, Qt::AlignCenter);
    item->setTextAlignment(2, Qt::AlignCenter);
  }

  void spawnPlayer(const int player)
  {
    int x = rand() % (MAP_SIZE[0] - 8) + 3;
    int z = rand() % (MAP_SIZE[1] - 6) + 3;

    // если на месте спавна танка есть блоки, удал€ем их
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

    // добавл€ем танк
    _tank.push_back(new tank(x * 8, z * 8, std::to_string(player % 13), player, &_tank, &_typeMap, &_tileMap, &_toDelete));
    _scene->asGroup()->addChild(_tank.back());
    _tank.back()->setName(_scene->getName() + " - " + std::to_string(player) + " player tank");

    // при убистве счетчик будет обновл€тьс€
    connect(_tank.back(), &tank::smbdyKilled, this, [this, player](int killCount) 
      { _treeWidget->topLevelItem(player)->setText(1, QString::number(killCount)); });
    // после уничтожени€ танк будет респавнитьс€ через какое-то врем€
    connect(_tank.back(), &tank::enemyNeedRespawn, this, &ViewerWidget::spawnPlayer);

    // делаем кнопку спавна неактивной
    _treeWidget->itemWidget(_treeWidget->topLevelItem(player), 2)->setEnabled(false);
  }

  void restart()
  {
    _typeMap.clear();
    _tileMap.clear();
    _toDelete.clear();
    _tank.clear();
    _playerNum = 0;
    // делаем кнопку спавна оп€ть активной и обнул€ем счетчик убийств
    for (int i = 0; i < _treeWidget->topLevelItemCount(); i++)
    {
      QTreeWidgetItem* tempItem = _treeWidget->topLevelItem(i);
      tempItem->setText(1, "0");
      _treeWidget->itemWidget(tempItem, 2)->setEnabled(true);
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
      //osgUtil::Optimizer::TRISTRIP_GEOMETRY | // раздел€ет на triangle strips
      osgUtil::Optimizer::TESSELLATE_GEOMETRY |
      osgUtil::Optimizer::OPTIMIZE_TEXTURE_SETTINGS |
      osgUtil::Optimizer::MERGE_GEODES |
      osgUtil::Optimizer::FLATTEN_BILLBOARDS |
      //osgUtil::Optimizer::TEXTURE_ATLAS_BUILDER | // текстуры танков артефачат
      osgUtil::Optimizer::STATIC_OBJECT_DETECTION |
      osgUtil::Optimizer::FLATTEN_STATIC_TRANSFORMS_DUPLICATING_SHARED_SUBGRAPHS |
      //osgUtil::Optimizer::INDEX_MESH | // раздел€ет на triangles
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
    //_viewer->addEventHandler(this);
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

  //bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter&)
  //{
  //  switch (ea.getEventType())
  //  {
  //  case(osgGA::GUIEventAdapter::KEYDOWN) :
  //  {
  //    switch (ea.getKey())
  //    {
  //    case(119) : // W
  //    {
  //      _pressedKeysP1[119] = true;
  //      _p1Tank->moveTo(direction::UP);
  //      break;
  //    }
  //    case(115) : // S
  //    {
  //      _pressedKeysP1[115] = true;
  //      _p1Tank->moveTo(direction::DOWN);
  //      break;
  //    }
  //    case(97) : // A
  //    {
  //      _pressedKeysP1[97] = true;
  //      _p1Tank->moveTo(direction::LEFT);
  //      break;
  //    }
  //    case(100) : // D
  //    {
  //      _pressedKeysP1[100] = true;
  //      _p1Tank->moveTo(direction::RIGHT);
  //      break;
  //    }
  //    case(32) : // SPACE
  //    {
  //      _p1Tank->shoot();
  //      break;
  //    }
  //               ////////////////////////////////////////////////////////////
  //    case(osgGA::GUIEventAdapter::KEY_Up) : // up
  //    {
  //      _pressedKeysP2[65362] = true;
  //      _p2Tank->moveTo(direction::UP);
  //      break;
  //    }
  //    case(osgGA::GUIEventAdapter::KEY_Down) : // down
  //    {
  //      _pressedKeysP2[65364] = true;
  //      _p2Tank->moveTo(direction::DOWN);
  //      break;
  //    }
  //    case(osgGA::GUIEventAdapter::KEY_Left) : // left
  //    {
  //      _pressedKeysP2[65361] = true;
  //      _p2Tank->moveTo(direction::LEFT);
  //      break;
  //    }
  //    case(osgGA::GUIEventAdapter::KEY_Right) : // right
  //    {
  //      _pressedKeysP2[65363] = true;
  //      _p2Tank->moveTo(direction::RIGHT);
  //      break;
  //    }
  //    case(osgGA::GUIEventAdapter::KEY_KP_Insert) : // num0
  //    {
  //      _p2Tank->shoot();
  //      break;
  //    }
  //    }
  //    return true;
  //  }
  //  case(osgGA::GUIEventAdapter::KEYUP) :
  //  {
  //    int key = ea.getKey();
  //    switch (key)
  //    {
  //    case(119) : // W
  //    case(115) : // S
  //    case(97) : // A
  //    case(100) : // D
  //    {
  //      _pressedKeysP1[key] = false;
  //      // если это была единственна€ нажата€ клавиша то танк останавливаетс€
  //      if (_pressedKeysP1[119])
  //        _p1Tank->moveTo(direction::UP);
  //      else if (_pressedKeysP1[115])
  //        _p1Tank->moveTo(direction::DOWN);
  //      else if (_pressedKeysP1[97])
  //        _p1Tank->moveTo(direction::LEFT);
  //      else if (_pressedKeysP1[100])
  //        _p1Tank->moveTo(direction::RIGHT);
  //      else
  //        _p1Tank->stop();
  //      break;
  //    }
  //    case(65362) : // up
  //    case(65364) : // down
  //    case(65361) : // left
  //    case(65363) : // right
  //    {
  //      _pressedKeysP2[key] = false;
  //      // если это была единственна€ нажата€ клавиша то танк останавливаетс€
  //      if (_pressedKeysP2[65362])
  //        _p2Tank->moveTo(direction::UP);
  //      else if (_pressedKeysP2[65364])
  //        _p2Tank->moveTo(direction::DOWN);
  //      else if (_pressedKeysP2[65361])
  //        _p2Tank->moveTo(direction::LEFT);
  //      else if (_pressedKeysP2[65363])
  //        _p2Tank->moveTo(direction::RIGHT);
  //      else
  //        _p2Tank->stop();
  //      break;
  //    }
  //    }
  //    return true;
  //  }
  //  default:
  //    return false;
  //  }
  //}

  void testSDLJoystick()
  {
    // Initialize the joystick subsystem
    SDL_InitSubSystem(SDL_INIT_JOYSTICK);

    // Check for joystick
    _console->insertPlainText(QString::fromLocal8Bit("Ќайдено ") + QString::number(SDL_NumJoysticks()) + QString::fromLocal8Bit(" джойстиков:\n\n"));
    for (int joyNum = 0; joyNum < SDL_NumJoysticks(); joyNum++)
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
    if (SDL_NumJoysticks() == 0)
      _console->insertPlainText(QString::fromLocal8Bit("Ќе найден ни один джойстик\n"));
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

      _hAxis = SDL_JoystickGetAxis(_joy, 0); // вертикальна€ ось - перва€
      _vAxis = SDL_JoystickGetAxis(_joy, SDL_JoystickNumAxes(_joy) - 1); // горизонтальна€ ось - последн€€
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
  QString _fileName;
  std::list<osg::ref_ptr<tank>> _tank;
  //std::map<int, bool> _pressedKeysP1; // дл€ игры с клавиатуры
  //std::map<int, bool> _pressedKeysP2;
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
  viewWidget->setGeometry(100, 100, 1080, 700);
  viewWidget->setWindowTitle("World of Tanks 2");
  viewWidget->show();
  return app.exec();
}
