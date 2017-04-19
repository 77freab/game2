#include <QTimer>
#include <QApplication>
#include <QBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QtWidgets/QPlainTextEdit>

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
    QFont font;
    font.setPointSize(20);

    _btn = new QPushButton("RESTART");
    _btn->setMaximumWidth(250);
    _btn->setMinimumHeight(50);
    _btn->setFont(font);

    _console = new QPlainTextEdit;
    _console->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    _console->setTextInteractionFlags(Qt::NoTextInteraction);
    _console->setMaximumWidth(250);

    QLocale::setDefault(QLocale(QLocale::Russian, QLocale::Russia));

    connect(_btn, &QPushButton::clicked, this, &ViewerWidget::restart);

    testSDLJoystick();

    _viewer->setThreadingModel(threadingModel);
    _viewer->setKeyEventSetsDone(0);
    _scene = createScene();
    _widget = addViewWidget(createGraphicsWindow(0, 0, 800, 700), _scene->asNode());

    _hLayout = new QHBoxLayout;
    _vLayout = new QVBoxLayout;
    _hLayout->addWidget(_widget);
    _hLayout->addLayout(_vLayout);
    _vLayout->addWidget(_btn);
    _vLayout->addWidget(_console);
    setLayout(_hLayout);

    connect(&_timer, SIGNAL(timeout()), this, SLOT(update()));
    _timer.start(10);
  }

  void restart()
  {
    _typeMap.clear();
    _tileMap.clear();
    _toDelete.clear();
    _tank.clear();

    _scene = createScene();
    osgQt::GLWidget* widget = addViewWidget(createGraphicsWindow(0, 0, 800, 700), _scene->asNode());;
    _hLayout->replaceWidget(_widget, widget);
    _widget = widget;
  }

  osg::ref_ptr<osg::Group> createScene()
  {
    osg::ref_ptr<osg::Group> scene = new osg::Group;
    scene->setName("main scene");
    void createMap(osg::ref_ptr<osg::Group> scene, 
      std::map<osg::Vec2i, blockType>& typeMap,
      std::map<osg::Vec2i, tile*>& tileMap);
    createMap(scene, _typeMap, _tileMap);
    srand(time(NULL));
    for (int i = 0; i < 2; i++)
    {
      int x = rand() % (27 - 3) + 3;
      int z = rand() % (26 - 2) + 2;

      std::map<osg::Vec2i, blockType>::const_iterator a;
      if ((a = _typeMap.find({ x - 1, z })) != _typeMap.end())
      {
        _toDelete.push_back(_tileMap[{ x - 1, z }]);
        _typeMap.erase(a);
      }
      if ((a = _typeMap.find({ x, z - 1 })) != _typeMap.end())
      {
        _toDelete.push_back(_tileMap[{ x, z-1 }]);
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

      _tank.push_back(new tank(x * 8, z * 8, std::to_string(i), i, &_tank, &_typeMap, &_tileMap, &_toDelete)); //88, 16 | 152, 208
      scene->addChild(_tank.back());
      _tank.back()->setName(scene->getName() + " - " + std::to_string(i) + " player tank");
    }
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
    return scene;
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
    _console->insertPlainText(QString::fromLocal8Bit("Ќайдено ") + QString::number(SDL_NumJoysticks()) + QString::fromLocal8Bit(" джойстиков\n\n"));
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

    //for (int joyNum = 0; joyNum < SDL_NumJoysticks(); joyNum++)
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
  QPushButton* _btn;
  QPlainTextEdit* _console;
  osg::ref_ptr<osg::Group> _scene;
  std::list<osg::ref_ptr<tank>> _tank;
  //std::map<int, bool> _pressedKeysP1; // дл€ игры с клавиатуры
  //std::map<int, bool> _pressedKeysP2;
  osg::ref_ptr<osgViewer::Viewer> _viewer;

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
  viewWidget->show();
  return app.exec();
}
