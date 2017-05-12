#include <cstdlib>
#include <ctime>

#include <QApplication>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>

#include <osgGA/TrackballManipulator>
#include <osgUtil/Optimizer>

#include "main.h"
#include "KeyboardEventHandler.h"
#include "VehicleControls.h"

#undef main

const int SIDE_PANEL_SIZE = 415;
const int NUM_KEYBOARD_CONTROLS = 5;
const QEvent::Type VEHICLE_KILLED_SOMEBODY_EVENT = static_cast<QEvent::Type>(QEvent::User + 1);
const QEvent::Type VEHICLE_NEED_RESPAWN_EVENT = static_cast<QEvent::Type>(QEvent::User + 2);

// custom event for updating killcount
VehicleKilledSomebody::VehicleKilledSomebody(const int player, const int killCount)
  : QEvent(VEHICLE_KILLED_SOMEBODY_EVENT), _killCount(killCount), _player(player)
{  }

int VehicleKilledSomebody::GetKillCount() const
{
  return _killCount;
}

int VehicleKilledSomebody::GetPlayer() const
{
  return _player;
}
// end

// custom event for player respawn
VehicleNeedRespawn::VehicleNeedRespawn(const osg::ref_ptr<Vehicle> vehicle)
  : QEvent(VEHICLE_NEED_RESPAWN_EVENT), _vehicle(vehicle)
{  }

osg::ref_ptr<Vehicle> VehicleNeedRespawn::GetVehicle() const
{
  return _vehicle;
}
// end

// main window - constructor
ViewerWidget::ViewerWidget( QWidget* parent, 
                            Qt::WindowFlags f,
                            osgViewer::ViewerBase::ThreadingModel threadingModel) : 
  QWidget(parent, f), 
  _viewer(new osgViewer::Viewer),
  _vehicleControls(NUM_KEYBOARD_CONTROLS, nullptr),
  _keyboardEventHandler(new KeyboardEventHandler(_vehicleControls))
{
  srand(time(NULL));

  vehicleControlsInit();

  _viewer->setThreadingModel(threadingModel);
  _viewer->setKeyEventSetsDone(0);
  _fileName = "./Resources/maps/original1.xml";

  _hLayout = new QHBoxLayout;

  _scene = createScene();
  _viewerWidget = addViewWidget(createGraphicsWindow(0, 0, 800, 700), _scene.get());
    
  _hLayout->addWidget(_viewerWidget);

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

  _playersList = new QTreeWidget;
  _playersList->setMaximumWidth(SIDE_PANEL_SIZE);
  _playersList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  _playersList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  QTreeWidgetItem *header = new QTreeWidgetItem();
  _playersList->setHeaderItem(header);
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
  _playersList->setColumnWidth(0, 50);
  _playersList->setColumnWidth(1, 65);
  _playersList->setColumnWidth(2, 80);
  _playersList->setColumnWidth(3, 120);
  _playersList->setColumnWidth(4, 80);

  _vLayout->addWidget(_playersList);

  _addPlayerBtn = new QPushButton;
  _addPlayerBtn->setText(QString::fromLocal8Bit("Добавить игрока"));

  _vLayout->addWidget(_addPlayerBtn);

  _hLayout->addLayout(_vLayout);

  testSDLJoystick();

  setLayout(_hLayout);

  connect(_addPlayerBtn, &QPushButton::clicked, this, &ViewerWidget::addPlayer);
  connect(_restartBtn, &QPushButton::clicked, this, &ViewerWidget::restart);
  connect(_openMapBtn, &QPushButton::clicked, this, &ViewerWidget::loadMap);
  connect(&_viewerUpdateTimer, &QTimer::timeout, 
    this, static_cast<void (ViewerWidget::*) ()>(&ViewerWidget::update));

  _viewerUpdateTimer.start(10);
}

// map loading
void ViewerWidget::loadMap()
{
  _fileName = QFileDialog::getOpenFileName(this, "Open map","./Resources/maps","XML files (*.xml)");
  if (_fileName != nullptr) restart();
}

void ViewerWidget::vehicleControlsInit()
{
  using keyboard = osgGA::GUIEventAdapter; // to make constants shorter

  _vehicleControls[0] = new VehicleControls(keyboard::KEY_W,
                                            keyboard::KEY_S,
                                            keyboard::KEY_A,
                                            keyboard::KEY_D,
                                            keyboard::KEY_Space);

  _vehicleControls[1] = new VehicleControls(keyboard::KEY_Up,
                                            keyboard::KEY_Down,
                                            keyboard::KEY_Left,
                                            keyboard::KEY_Right,
                                            keyboard::KEY_0);

  _vehicleControls[2] = new VehicleControls(keyboard::KEY_Y,
                                            keyboard::KEY_H,
                                            keyboard::KEY_G,
                                            keyboard::KEY_J,
                                            keyboard::KEY_N);

  _vehicleControls[3] = new VehicleControls(keyboard::KEY_O,
                                            keyboard::KEY_L,
                                            keyboard::KEY_K,
                                            keyboard::KEY_Semicolon,
                                            keyboard::KEY_Period);

  _vehicleControls[4] = new VehicleControls(keyboard::KEY_8,
                                            keyboard::KEY_5,
                                            keyboard::KEY_4,
                                            keyboard::KEY_6,
                                            keyboard::KEY_Plus);
}

// accept number of control device, 
// return string with the name of this control device
QString ViewerWidget::controlsName(int controlDevice)
{
  // controlDevice = -5 is "8546"   + '+'
  // controlDevice = -4 is "OLK;"   + '.'
  // controlDevice = -3 is "YHGJ"   + 'N'
  // controlDevice = -2 is "Arrows" + 'Num_0'
  // controlDevice = -1 is "WSAD"   + 'Space'

  if (controlDevice == -1) return QString("WSAD + Space");
  else if (controlDevice == -2) return QString("Arrows + Num_0");
  else if (controlDevice == -3) return QString("YHGJ + N");
  else if (controlDevice == -4) return QString("\"OLK;\" + \'.\'");
  else if (controlDevice == -5) return QString("8546 + \'+\'");
  else return QString("Joystick " + QString::number(controlDevice));
}

// changing control type for specific player
void ViewerWidget::changeControls(int player, int controlDevice)
{
  int swapControl = _vehicles[player]->GetControlDevice();

  for (int i = 0; i < _playerNum; i++)
  {
    if (_vehicles[i]->GetControlDevice() == controlDevice)
    {
      _vehicles[i]->SetControlDevice(swapControl);
      QPushButton* btn = dynamic_cast<QPushButton*>(_playersList->itemWidget(_playersList->topLevelItem(i), 3));
      btn->setText(controlsName(swapControl));
      if (swapControl < 0)
      {
        _vehicleControls[abs(swapControl) - 1]->SetVehicle(_vehicles[i].get());
      }
    }
    else if (swapControl < 0)
    {
      _vehicleControls[abs(swapControl) - 1]->SetVehicle(nullptr);
    }
  }

  _vehicles[player]->SetControlDevice(controlDevice);

  if (controlDevice < 0)
  {
    _vehicleControls[abs(controlDevice) - 1]->SetVehicle(_vehicles[player].get());
  }
}

// called when user press "add player" button
void ViewerWidget::addPlayer()
{
  // max number of players cannot be greater then number of control devices (including keyboard)
  if (_playerNum - NUM_KEYBOARD_CONTROLS < _numJoysticks)
  {
    int player = _playerNum;

    // looking for unoccupied control device (begining from keyboard)
    for (int freeControl = -NUM_KEYBOARD_CONTROLS; freeControl < _numJoysticks; freeControl++)
    {
      bool pr = true;
      for (Vehicle* curVehicle : _vehicles)
      {
        if (curVehicle->GetControlDevice() == freeControl) pr = false;
      }
      if (pr)
      {
        _vehicles.push_back(new LightTank(0, 0, player, freeControl,
          _vehicles, _tileMap, _toDelete, *this, 0));
        break;
      }
    }

    _vehicles.back()->setName(_scene->getName() + " - " + std::to_string(player) + " player vehicle");

    QTreeWidgetItem *item = new QTreeWidgetItem(_playersList);

    item->setIcon(0, QIcon("./Resources/heavyTank/" + QString::number(_playerNum % Vehicle::COLORED_TEXTURES_NUM) + ".bmp"));
    item->setText(1, QString::fromLocal8Bit("0"));

    // button showing list of vehicle types
    QPushButton* vehicleTypeBtn = new QPushButton(QString::fromLocal8Bit("Легкий"));
    QMenu* vehicleTypeMenu = new QMenu;

    // button for type changing to light tank
    QAction* act = new QAction;
    connect(act, &QAction::triggered, vehicleTypeBtn, [vehicleTypeBtn, act] { vehicleTypeBtn->setText(act->text()); });
    connect(act, &QAction::triggered, this, [this, player] 
    { 
      if (_vehicles[player]->GetType() != Vehicle::type::LIGHT)
      {
        // saving some information before deleting old vehicle
        int controlDevice = _vehicles[player]->GetControlDevice();
        int killCount = _vehicles[player]->GetKillCount();

        _vehicles[player]->Disable();

        if (_vehicles[player]->getNumParents() != 0)
        {
          _vehicles[player]->getParent(0)->removeChild(_vehicles[player]);
        }

        _vehicles[player] = new LightTank(0, 0, player, controlDevice,
          _vehicles, _tileMap, _toDelete, *this, killCount);

        if (controlDevice < 0)
        {
          _vehicleControls[abs(controlDevice) - 1]->SetVehicle(_vehicles[player].get());
        }

        _playersList->itemWidget(_playersList->topLevelItem(player), 4)->setEnabled(true);
      }
    });
    act->setText(QString::fromLocal8Bit("Легкий"));
    vehicleTypeMenu->addAction(act);

    // button for type changing to heavy tank
    act = new QAction;
    connect(act, &QAction::triggered, vehicleTypeBtn, [vehicleTypeBtn, act] { vehicleTypeBtn->setText(act->text()); });
    connect(act, &QAction::triggered, this, [this, player] 
    { 
      if (_vehicles[player]->GetType() != Vehicle::type::HEAVY)
      {
        // saving some information before deleting old vehicle
        int controlDevice = _vehicles[player]->GetControlDevice();
        int killCount = _vehicles[player]->GetKillCount();

        _vehicles[player]->Disable();

        if (_vehicles[player]->getNumParents() != 0)
        {
          _vehicles[player]->getParent(0)->removeChild(_vehicles[player]);
        }

        _vehicles[player] = new HeavyTank(0, 0, player, controlDevice,
          _vehicles, _tileMap, _toDelete, *this, killCount);

        if (controlDevice < 0)
        {
          _vehicleControls[abs(controlDevice) - 1]->SetVehicle(_vehicles[player].get());
        }

        _playersList->itemWidget(_playersList->topLevelItem(player), 4)->setEnabled(true);
      }
    });
    act->setText(QString::fromLocal8Bit("Тяжелый"));
    vehicleTypeMenu->addAction(act);

    // button for type changing to motorcycle
    act = new QAction;
    connect(act, &QAction::triggered, vehicleTypeBtn, [vehicleTypeBtn, act] { vehicleTypeBtn->setText(act->text()); });
    connect(act, &QAction::triggered, this, [this, player]
    {
      if (_vehicles[player]->GetType() != Vehicle::type::MOTO)
      {
        // saving some information before deleting old vehicle
        int controlDevice = _vehicles[player]->GetControlDevice();
        int killCount = _vehicles[player]->GetKillCount();

        _vehicles[player]->Disable();

        if (_vehicles[player]->getNumParents() != 0)
        {
          _vehicles[player]->getParent(0)->removeChild(_vehicles[player]);
        }

        _vehicles[player] = new Motorcycle(0, 0, player, controlDevice,
          _vehicles, _tileMap, _toDelete, *this, killCount);

        if (controlDevice < 0)
        {
          _vehicleControls[abs(controlDevice) - 1]->SetVehicle(_vehicles[player].get());
        }

        _playersList->itemWidget(_playersList->topLevelItem(player), 4)->setEnabled(true);
      }
    });
    act->setText(QString::fromLocal8Bit("Мотоцикл"));
    vehicleTypeMenu->addAction(act);

    vehicleTypeBtn->setMenu(vehicleTypeMenu);
    _playersList->setItemWidget(item, 2, vehicleTypeBtn);

    // by default vehicle controls are set up to keyboard
    const int controlDevice = _vehicles.back()->GetControlDevice();
    if (controlDevice < 0)
    {
      _vehicleControls[abs(controlDevice) - 1]->SetVehicle(_vehicles.back().get());
    }

    // button showing list of control devices
    QPushButton* controlsBtn = new QPushButton(controlsName(controlDevice));

    QMenu* controlsMenu = new QMenu;

    // adding buttons to list of control devices
    for (int i = -NUM_KEYBOARD_CONTROLS; i < _numJoysticks; i++)
    {
      addControlDeviceButton(controlsBtn, *controlsMenu, player, i);
    }

    controlsBtn->setMenu(controlsMenu);
    _playersList->setItemWidget(item, 3, controlsBtn);

    QPushButton* spawnBtn = new QPushButton("GO");
    _playersList->setItemWidget(item, 4, spawnBtn);

    // player will be spawned by button press
    connect(spawnBtn, &QPushButton::clicked, this, [this, player] 
      { 
        spawnPlayer(_vehicles[player]);
        // after spawn button become deactivated
        _playersList->itemWidget(_playersList->topLevelItem(player), 4)->setEnabled(false); 
      });

    _playerNum++;
    item->setTextAlignment(0, Qt::AlignCenter);
    item->setTextAlignment(1, Qt::AlignCenter);
    item->setTextAlignment(2, Qt::AlignCenter);
    item->setTextAlignment(3, Qt::AlignCenter);
    item->setTextAlignment(4, Qt::AlignCenter);
  }
}

// adding button to list of vehicle controls
void ViewerWidget::addControlDeviceButton(QPushButton* parentBtn, QMenu& menu, int player, int controlDevice)
{
  QAction* act = new QAction;
  connect(act, &QAction::triggered, parentBtn, [parentBtn, act] { parentBtn->setText(act->text()); });
  connect(act, &QAction::triggered, this, [this, player, controlDevice] { changeControls(player, controlDevice); });
  act->setText(controlsName(controlDevice));
  menu.addAction(act);
}

// remove tiles on vehicle spawn point if there are any
void ViewerWidget::clearPlaceForVehicle(int x, int z)
{
  if (_tileMap[x - 1][z] != nullptr)
  {
    _toDelete.push_back(_tileMap[x - 1][z]);
    _tileMap[x - 1][z] = nullptr;
  }
  if (_tileMap[x][z - 1] != nullptr)
  {
    _toDelete.push_back(_tileMap[x][z - 1]);
    _tileMap[x][z - 1] = nullptr;
  }
  if (_tileMap[x - 1][z - 1] != nullptr)
  {
    _toDelete.push_back(_tileMap[x - 1][z - 1]);
    _tileMap[x - 1][z - 1] = nullptr;
  }
  if (_tileMap[x][z] != nullptr)
  {
    _toDelete.push_back(_tileMap[x][z]);
    _tileMap[x][z] = nullptr;
  }
}

// place vehicle on game area
void ViewerWidget::spawnPlayer(osg::ref_ptr<Vehicle> vehicle)
{
  int x = rand() % (_mapSize[0] - 8) + 3;
  int z = rand() % (_mapSize[1] - 6) + 3;

  vehicle->SetXCoord(x * 8);
  vehicle->SetZCoord(z * 8);

  // clean spawn point
  clearPlaceForVehicle(x, z);

  // moving vehicle to spawn point
  osg::Matrix m;
  m.makeTranslate(x * 8, 0, z * 8);
  vehicle->setMatrix(m);

  // if vehicle not on the scene already
  if (!_scene->asGroup()->containsNode(vehicle))
  {
    // adding and activating it
    _scene->asGroup()->addChild(vehicle.get());
    vehicle->Enable();
  }
}

// game restart
void ViewerWidget::restart()
{
  while (!_toDelete.empty())
  {
    _toDelete.front()->getParent(0)->removeChild(_toDelete.front());
    _toDelete.pop_front();
  }

  osg::ref_ptr<osg::Node> temp;
  if ((temp = createScene()) != nullptr)
  {
    // disabling all vehicles
    for (Vehicle* curVehicle : _vehicles) curVehicle->Disable();

    // making spawn button active and reseting kill counter
    for (int i = 0; i < _playersList->topLevelItemCount(); i++)
    {
      QTreeWidgetItem* tempItem = _playersList->topLevelItem(i);
      tempItem->setText(1, "0");
      _playersList->itemWidget(tempItem, 4)->setEnabled(true);
    }
    _scene = temp;
    osgQt::GLWidget* newViewerWidget = addViewWidget(createGraphicsWindow(0, 0, 800, 700), _scene.get());;
    _hLayout->replaceWidget(_viewerWidget, newViewerWidget);
    _viewerWidget = newViewerWidget;
  }
}

// scene creating
osg::ref_ptr<osg::Node> ViewerWidget::createScene()
{
  osg::ref_ptr<osg::Group> scene = new osg::Group;
  scene->setName("main scene");
  int returnCode;
  if ((returnCode = mapMaker.CreateMap(scene, _tileMap, _fileName, _mapSize)) == -2)
  {
    QMessageBox::warning(this, "file error",
      QString::fromLocal8Bit("Кривой файл"), QMessageBox::Ok);
    return nullptr;
  }
  else if (returnCode == -1)
  {
    QMessageBox::warning(this, "file error",
      QString::fromLocal8Bit("Не могу открыть файл"), QMessageBox::Ok);
    return nullptr;
  }

  osgUtil::Optimizer opt;
  opt.optimize(scene,
    osgUtil::Optimizer::DEFAULT_OPTIMIZATIONS |
    osgUtil::Optimizer::FLATTEN_STATIC_TRANSFORMS_DUPLICATING_SHARED_SUBGRAPHS |
    osgUtil::Optimizer::SPATIALIZE_GROUPS |
    osgUtil::Optimizer::COPY_SHARED_NODES |
    osgUtil::Optimizer::TESSELLATE_GEOMETRY |
    osgUtil::Optimizer::MERGE_GEODES |
    osgUtil::Optimizer::FLATTEN_BILLBOARDS |
    osgUtil::Optimizer::TEXTURE_ATLAS_BUILDER |
    osgUtil::Optimizer::VERTEX_POSTTRANSFORM |
    osgUtil::Optimizer::VERTEX_PRETRANSFORM
    );
  return scene.get();
}

// making viewer widget
osgQt::GLWidget* ViewerWidget::addViewWidget(osgQt::GraphicsWindowQt* gw, osg::Node* scene)
{
  osg::Camera* camera = _viewer->getCamera();
  camera->setGraphicsContext(gw);

  const osg::GraphicsContext::Traits* traits = gw->getTraits();

  camera->setClearColor(osg::Vec4(0, 0, 0, 1));
  camera->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));

  _viewer->setSceneData(scene);
  //_viewer->addEventHandler(new osgViewer::StatsHandler); // FPS and stats
  _viewer->addEventHandler(_keyboardEventHandler);
  _viewer->setCameraManipulator(new osgGA::TrackballManipulator);
  gw->setTouchEventsEnabled(true);
  return gw->getGLWidget();
}

osgQt::GraphicsWindowQt* ViewerWidget::createGraphicsWindow(int x, int y, int w, int h)
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

// testing connected joystics
void ViewerWidget::testSDLJoystick()
{
  // Initialize the joystick subsystem
  SDL_InitSubSystem(SDL_INIT_JOYSTICK);
  _numJoysticks = SDL_NumJoysticks();

  // Check for joystick
  _console->insertPlainText(QString::fromLocal8Bit("Найдено ") + 
    QString::number(_numJoysticks) + QString::fromLocal8Bit(" джойстиков:\n\n"));

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
  {
    _console->insertPlainText(QString::fromLocal8Bit("Не найден ни один джойстик\n"));
  }
}

// event handling
bool ViewerWidget::event(QEvent* event)
{
  if (event->type() == QEvent::Paint)
  {
    _viewer->frame();

    while (!_toDelete.empty())
    {
      _toDelete.front()->getParent(0)->removeChild(_toDelete.front());
      _toDelete.pop_front();
    }

    int hAxis, vAxis;
    bool startBtn, fireBtn;
    int camX = _viewer->getCamera()->getViewMatrix()(3, 0);
    int camZ = _viewer->getCamera()->getViewMatrix()(3, 1);

    if (camZ < 0)
    {

      if (camX < 0)
      {
        // camera from below of game area
        _up = Vehicle::direction::UP;
        _down = Vehicle::direction::DOWN;
        _left = Vehicle::direction::LEFT;
        _right = Vehicle::direction::RIGHT;
      }
      else
      {
        // camera to the left from game area
        _up = Vehicle::direction::RIGHT;
        _down = Vehicle::direction::LEFT;
        _left = Vehicle::direction::UP;
        _right = Vehicle::direction::DOWN;
      }
    }
    else
    {
      if (camX > 0)
      {
        // camera from above of game area
        _up = Vehicle::direction::DOWN;
        _down = Vehicle::direction::UP;
        _left = Vehicle::direction::RIGHT;
        _right = Vehicle::direction::LEFT;
      }
      else
      {
        // camera to the right from game area
        _up = Vehicle::direction::LEFT;
        _down = Vehicle::direction::RIGHT;
        _left = Vehicle::direction::DOWN;
        _right = Vehicle::direction::UP;
      }
    }

    for (VehicleControls* vc : _vehicleControls)
    {
      vc->SetMovingDirections(_up, _down, _left, _right);
    }

    for (Vehicle* curVehicle : _vehicles)
    {
      // keyboard control devices handled in "keyboardEventHandler" class
      if (curVehicle->GetControlDevice() < 0) continue;

      curVehicle->Stop();

      _joy = SDL_JoystickOpen(curVehicle->GetControlDevice());

      SDL_JoystickUpdate();

      hAxis = SDL_JoystickGetAxis(_joy, 0); // vertical axis is the first one (on joystics i use)
      vAxis = SDL_JoystickGetAxis(_joy, SDL_JoystickNumAxes(_joy) - 1); // horizontal axis is the last one
      startBtn = SDL_JoystickGetButton(_joy, 9);
      fireBtn = SDL_JoystickGetButton(_joy, 2);

      // UP button
      if (vAxis < -20000) curVehicle->SetMovingDirection(_up);
      // DOWN button
      if (vAxis > 20000) curVehicle->SetMovingDirection(_down);
      // LEFT button
      if (hAxis < -20000) curVehicle->SetMovingDirection(_left);
      // RIGHT button
      if (hAxis > 20000) curVehicle->SetMovingDirection(_right);
      // FIRE button
      if (fireBtn) curVehicle->Shoot();
    }
    return true;
  }
  else if (event->type() == VEHICLE_KILLED_SOMEBODY_EVENT)
  {
    // updating killcount
    VehicleKilledSomebody* ev = static_cast<VehicleKilledSomebody *>(event);
    _playersList->topLevelItem(ev->GetPlayer())->setText(1,
      QString::number(ev->GetKillCount()));
    return true;
  }
  else if (event->type() == VEHICLE_NEED_RESPAWN_EVENT)
  {
    // spawning destroyed vehicle
    spawnPlayer(static_cast<VehicleNeedRespawn *>(event)->GetVehicle());
    return true;
  }
  return QWidget::event(event);
}

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
  {
    QApplication::setAttribute(Qt::AA_X11InitThreads);
  }
#endif

  QApplication app(argc, argv);
  ViewerWidget* viewWidget = new ViewerWidget(0, Qt::Widget, threadingModel);
  viewWidget->setGeometry(100, 100, 800 + 30 + SIDE_PANEL_SIZE, 700);
  viewWidget->setWindowTitle("World of Tanks 2");
  viewWidget->show();
  return app.exec();
}
