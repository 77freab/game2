#include <QTimer>
#include <QApplication>
#include <QBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QLineEdit>

#include <osgViewer/ViewerEventHandlers>

#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>

#include <osgQt/GraphicsWindowQt>

#include <osgUtil/Optimizer>

#include <iostream>
#include <list>

#include "tank.h"

#include <QtTest/QTest> 
#include <QDebug> 

#include <SDL.h>
#undef main

std::list<osg::Node*> toDelete;



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

    if (testSDLJoystick())
    {
      _viewer->setThreadingModel(threadingModel);
      _viewer->setKeyEventSetsDone(0);
      _scene = createScene();
      _widget = addViewWidget(createGraphicsWindow(0, 0, 800, 700), _scene->asNode());
    }

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
    _scene = createScene();
    osgQt::GLWidget* widget = addViewWidget(createGraphicsWindow(0, 0, 800, 700), _scene->asNode());;
    _hLayout->replaceWidget(_widget, widget);
    _widget = widget;
  }

  osg::ref_ptr<osg::Group> createScene()
  {
    osg::ref_ptr<osg::Group> scene = new osg::Group;
    scene->setName("main scene");
    void createMap(osg::ref_ptr<osg::Group> scene);
    createMap(scene);
    _p1Tank = new tank(80, 8, "yellow/T1_");
    scene->addChild(_p1Tank);
    _p1Tank->setName(scene->getName() + " - 1st player tank");
    _p2Tank = new tank(144, 200, "green/T1_");
    scene->addChild(_p2Tank);
    _p2Tank->setName(scene->getName() + " - 2nd player tank");
    _p1Tank->setEnemy(_p2Tank.get());
    _p2Tank->setEnemy(_p1Tank.get());
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
    return scene;
  }

  osgQt::GLWidget* addViewWidget(osgQt::GraphicsWindowQt* gw, osg::Node* scene)
  {
    //osgViewer::View* view = new osgViewer::View;
    //addView(view);

    osg::Camera* camera = _viewer->getCamera();
    camera->setGraphicsContext(gw);

    const osg::GraphicsContext::Traits* traits = gw->getTraits();

    camera->setClearColor(osg::Vec4(0, 0, 0, 1));
    camera->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));
    //camera->setViewMatrixAsLookAt({ 128, -440, 112 }, { 128, 0, 112 }, { 0, 0, 1 });
    camera->setViewMatrixAsLookAt({ -100, -440, -100 }, { 128, 0, 112 }, { 1, 1, 1 });
    camera->setProjectionMatrixAsPerspective(28.0f, 32./28, 1.0f, 10000.0f);

    _viewer->setSceneData(scene);
    //_viewer->addEventHandler(new osgViewer::StatsHandler);
    //_viewer->addEventHandler(this);
    //_viewer->setCameraManipulator(new osgGA::TrackballManipulator);
    gw->setTouchEventsEnabled(true);
    return gw->getGLWidget();
  }

  osgQt::GraphicsWindowQt* createGraphicsWindow(int x, int y, int w, int h, const std::string& name = "", bool windowDecoration = false)
  {
    osg::DisplaySettings* ds = osg::DisplaySettings::instance().get();
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->windowName = name;
    traits->windowDecoration = windowDecoration;
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
  //      // если это была единственная нажатая клавиша то танк останавливается
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
  //      // если это была единственная нажатая клавиша то танк останавливается
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

  bool testSDLJoystick()
  {
    // Initialize the joystick subsystem
    SDL_InitSubSystem(SDL_INIT_JOYSTICK);

    // Check for joystick
    if (SDL_NumJoysticks() > 0)
    {
      _console->insertPlainText(QString::fromLocal8Bit("Найдено ") + QString::number(SDL_NumJoysticks()) + QString::fromLocal8Bit(" джойстиков\n"));
      // Open joystick
      _joy1 = SDL_JoystickOpen(0);
      _joy2 = SDL_JoystickOpen(1);

      if (_joy1)
      {
        _console->insertPlainText(
          "Opened Joystick 0\nName: " + QString(SDL_JoystickName(_joy1)) +
          "\nNumber of Axes: " + QString::number(SDL_JoystickNumAxes(_joy1)) +
          "\nNumber of Buttons: " + QString::number(SDL_JoystickNumButtons(_joy1)) +
          "\nNumber of Balls: " + QString::number(SDL_JoystickNumBalls(_joy1)) + "\n\n"
          );
      }
      if (_joy2)
      {
        _console->insertPlainText(
          "Opened Joystick 0\nName: " + QString(SDL_JoystickName(_joy2)) +
          "\nNumber of Axes: " + QString::number(SDL_JoystickNumAxes(_joy2)) +
          "\nNumber of Buttons: " + QString::number(SDL_JoystickNumButtons(_joy2)) +
          "\nNumber of Balls: " + QString::number(SDL_JoystickNumBalls(_joy2)) + "\n\n"
          );
      }
      return true;
    }
    else
      _console->insertPlainText(QString::fromLocal8Bit("Не найден ни один джойстик\n"));
    return false;
  }

  virtual void paintEvent(QPaintEvent* event)
  {
    _viewer->frame();

    while (!toDelete.empty())
    {
      toDelete.front()->getParent(0)->removeChild(toDelete.front());
      toDelete.pop_front();
    }

    _p1Tank->stop();
    _p2Tank->stop();

    SDL_JoystickUpdate();

    int hAxisP1, vAxisP1;
    bool startBtnP1, fireBtnP1;

    hAxisP1 = SDL_JoystickGetAxis(_joy1, 0);
    vAxisP1 = SDL_JoystickGetAxis(_joy1, 1);
    startBtnP1 = SDL_JoystickGetButton(_joy1, 9);
    fireBtnP1 = SDL_JoystickGetButton(_joy1, 2);

    if (vAxisP1 < -20000) // UP button
      _p1Tank->moveTo(direction::LEFT);
    if (vAxisP1 > 20000) // DOWN button
      _p1Tank->moveTo(direction::RIGHT);
    if (hAxisP1 < -20000) // LEFT button
      _p1Tank->moveTo(direction::DOWN);
    if (hAxisP1 > 20000) // RIGHT button
      _p1Tank->moveTo(direction::UP);
    if (fireBtnP1) // FIRE button
      _p1Tank->shoot();

    ////////////////////////////////////////////////////////////

    int hAxisP2, vAxisP2;
    bool startBtnP2, fireBtnP2;

    hAxisP2 = SDL_JoystickGetAxis(_joy2, 0);
    vAxisP2 = SDL_JoystickGetAxis(_joy2, 1);
    startBtnP2 = SDL_JoystickGetButton(_joy2, 9);
    fireBtnP2 = SDL_JoystickGetButton(_joy2, 2);

    if (vAxisP2 < -20000) // UP button
      _p2Tank->moveTo(direction::LEFT);
    if (vAxisP2 > 20000) // DOWN button
      _p2Tank->moveTo(direction::RIGHT);
    if (hAxisP2 < -20000) // LEFT button
      _p2Tank->moveTo(direction::DOWN);
    if (hAxisP2 > 20000) // RIGHT button
      _p2Tank->moveTo(direction::UP);
    if (fireBtnP2) // FIRE button
      _p2Tank->shoot();
  }

protected:

  QTimer _timer;
  osgQt::GLWidget* _widget;
  QHBoxLayout* _hLayout;
  QVBoxLayout* _vLayout;
  QPushButton* _btn;
  QPlainTextEdit* _console;
  osg::ref_ptr<osg::Group> _scene;
  osg::ref_ptr<tank> _p1Tank;
  osg::ref_ptr<tank> _p2Tank;
  std::map<int, bool> _pressedKeysP1;
  std::map<int, bool> _pressedKeysP2;
  osg::ref_ptr<osgViewer::Viewer> _viewer;
  SDL_Joystick* _joy1;
  SDL_Joystick* _joy2;
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
