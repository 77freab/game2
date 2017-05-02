#include <QApplication>
#include <QBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QTreeWidget>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QTimer>

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

const int SIDE_PANEL_SIZE = 415;

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
    connect(&_viewerUpdateTimer, SIGNAL(timeout()), this, SLOT(update()));
    _viewerUpdateTimer.start(10);
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
    int swapControl = _tank[player]->GetControlDevice();

    for (int i = 0; i < _playerNum; i++)
    {
      if (_tank[i]->GetControlDevice() == controlDevice)
      {
        _tank[i]->SetControlDevice(swapControl);
        QPushButton* btn = dynamic_cast<QPushButton*>(_playersList->itemWidget(_playersList->topLevelItem(i), 3));
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

    _tank[player]->SetControlDevice(controlDevice);

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

      // ищем незанятый способ управленя танком
      for (int freeControl = -2; freeControl < _numJoysticks; freeControl++)
      {
        bool pr = true;
        for (auto it = _tank.cbegin(); it != _tank.cend(); it++)
        {
          if ((*it)->GetControlDevice() == freeControl)
            pr = false;
        }
        if (pr)
        {
          _tank.push_back(new tank(0, 0, std::to_string(player % 13), freeControl, &_tank, &_typeMap, &_tileMap, &_toDelete, &mapMaker));
          break;
        }
      }

      _tank.back()->setName(_scene->getName() + " - " + std::to_string(player) + " player tank");

      QTreeWidgetItem *item = new QTreeWidgetItem(_playersList);

      item->setIcon(0, QIcon("./Resources/heavyTank/" + QString::number(_playerNum % 13) + ".bmp"));
      item->setText(1, QString::fromLocal8Bit("0"));

      // тип танка
      QPushButton* tankTypeBtn = new QPushButton(QString::fromLocal8Bit("Легкий"));
      QMenu* tankTypeMenu = new QMenu;

      // кнопка смены типа танка на легкий
      QAction* act = new QAction;
      connect(act, &QAction::triggered, tankTypeBtn, [tankTypeBtn, act] { tankTypeBtn->setText(act->text()); });
      connect(act, &QAction::triggered, this, [this, player] 
      { 
        if (_tank[player]->GetType() != tank::type::LIGHT)
        {
          _tank[player]->SetNeedTypeChange();
          _playersList->itemWidget(_playersList->topLevelItem(player), 4)->setEnabled(true);
        }
      });
      act->setText(QString::fromLocal8Bit("Легкий"));
      tankTypeMenu->addAction(act);

      // кнопка смены типа танка на тяжелый
      act = new QAction;
      connect(act, &QAction::triggered, tankTypeBtn, [tankTypeBtn, act] { tankTypeBtn->setText(act->text()); });
      connect(act, &QAction::triggered, this, [this, player] 
      { 
        if (_tank[player]->GetType() != tank::type::HEAVY)
        {
          _tank[player]->SetNeedTypeChange();
          _playersList->itemWidget(_playersList->topLevelItem(player), 4)->setEnabled(true);
        }
      });
      act->setText(QString::fromLocal8Bit("Тяжелый"));
      tankTypeMenu->addAction(act);

      tankTypeBtn->setMenu(tankTypeMenu);
      _playersList->setItemWidget(item, 2, tankTypeBtn);

      // управление
      QPushButton* controlsBtn = new QPushButton(controlsName(_tank.back()->GetControlDevice()));
      // по умолчанию танки 0 и 1 управляются с WASD и стрелок соответственно
      if (_tank.back()->GetControlDevice() == -2)
        _wasdTank = _tank.back();
      if (_tank.back()->GetControlDevice() == -1)
        _arrowsTank = _tank.back();

      QMenu* controlsMenu = new QMenu;

      // кнопка смены управления на WASD
      act = new QAction;
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
      _playersList->setItemWidget(item, 3, controlsBtn);

      QPushButton* spawnBtn = new QPushButton("GO");
      _playersList->setItemWidget(item, 4, spawnBtn);

      // при убистве счетчик будет обновляться
      connect(_tank[player], &tank::smbdyKilled, this, [this, player](int killCount)
        { _playersList->topLevelItem(player)->setText(1, QString::number(killCount)); });
      // после уничтожения танк будет респавниться через какое-то время
      connect(_tank[player], &tank::iNeedRespawn, this, &ViewerWidget::spawnPlayer);
      // заспавнить по нажатию кнопки
      connect(spawnBtn, &QPushButton::clicked, this, [this, player] 
        { spawnPlayer(_tank[player].get());
          // делаем кнопку спавна неактивной
          _playersList->itemWidget(_playersList->topLevelItem(player), 4)->setEnabled(false); });

      _playerNum++;
      item->setTextAlignment(0, Qt::AlignCenter);
      item->setTextAlignment(1, Qt::AlignCenter);
      item->setTextAlignment(2, Qt::AlignCenter);
      item->setTextAlignment(3, Qt::AlignCenter);
      item->setTextAlignment(4, Qt::AlignCenter);
    }
  }

  void clearPlaceForTank(int x, int z)
  {
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
  }

  void spawnPlayer(osg::ref_ptr<tank> tank)
  {
    // если надо меняем тип
    if (tank->NeedTypeChange())
      tank->ChangeType();
    
    int x = rand() % (_mapSize[0] - 8) + 3;
    int z = rand() % (_mapSize[1] - 6) + 3;

    tank->SetXCoord(x * 8);
    tank->SetZCoord(z * 8);

    // расчищаем место дял спавна
    clearPlaceForTank(x, z);

    // перемещаем в точку спавна
    osg::Matrix m;
    m.makeTranslate(x * 8, 0, z * 8);
    tank->setMatrix(m);

    if (!_scene->asGroup()->containsNode(tank))
    {
      // добавляем на сцену а активируем
      _scene->asGroup()->addChild(tank.get());
      tank->Enable();
    }
  }

  void restart()
  {
    while (!_toDelete.empty())
    {
      _toDelete.front()->getParent(0)->removeChild(_toDelete.front());
      _toDelete.pop_front();
    }

    //_playerNum = 0; // смешной баг с ускорением

    osg::ref_ptr<osg::Node> temp;
    if ((temp = createScene()) != nullptr)
    {
      // отключаем все танки
      for (auto it = _tank.cbegin(); it != _tank.cend(); it++)
        (*it)->Disable();

      // делаем кнопку спавна опять активной и обнуляем счетчик убийств
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

  osg::ref_ptr<osg::Node> createScene()
  {
    osg::ref_ptr<osg::Group> scene = new osg::Group;
    scene->setName("main scene");
    int returnCode;
    if ((returnCode = mapMaker.createMap(scene, _typeMap, _tileMap, _fileName, _mapSize)) == -2)
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
      //osgUtil::Optimizer::TRISTRIP_GEOMETRY | // разделяет на triangle strips
      osgUtil::Optimizer::TESSELLATE_GEOMETRY |
      osgUtil::Optimizer::MERGE_GEODES |
      osgUtil::Optimizer::FLATTEN_BILLBOARDS |
      //osgUtil::Optimizer::TEXTURE_ATLAS_BUILDER | // текстуры танков артефачат
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
    //camera->setViewMatrixAsLookAt({ -100, -440, -100 }, { 128, 0, 112 }, { 1, 1, 1 });
    //camera->setProjectionMatrixAsPerspective(28.0f, 32./28, 1.0f, 10000.0f);

    _viewer->setSceneData(scene);
    //_viewer->addEventHandler(new osgViewer::StatsHandler); // ФПС и прочее
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
            _pressedKeys[119] = true;
            _wasdTank->SetMovingDirection(_up);
            break;
          }
          case(115) : // S
          {
            _pressedKeys[115] = true;
            _wasdTank->SetMovingDirection(_down);
            break;
          }
          case(97) : // A
          {
            _pressedKeys[97] = true;
            _wasdTank->SetMovingDirection(_left);
            break;
          }
          case(100) : // D
          {
            _pressedKeys[100] = true;
            _wasdTank->SetMovingDirection(_right);
            break;
          }
          case(32) : // SPACE
          {
            _wasdTank->Shoot();
            break;
          }
        }
                 ////////////////////////////////////////////////////////////
      if (_arrowsTank != nullptr)
        switch (ea.getKey())
        {
          case(osgGA::GUIEventAdapter::KEY_Up) : // up
          {
            _pressedKeys[65362] = true;
            _arrowsTank->SetMovingDirection(_up);
            break;
          }
          case(osgGA::GUIEventAdapter::KEY_Down) : // down
          {
            _pressedKeys[65364] = true;
            _arrowsTank->SetMovingDirection(_down);
            break;
          }
          case(osgGA::GUIEventAdapter::KEY_Left) : // left
          {
            _pressedKeys[65361] = true;
            _arrowsTank->SetMovingDirection(_left);
            break;
          }
          case(osgGA::GUIEventAdapter::KEY_Right) : // right
          {
            _pressedKeys[65363] = true;
            _arrowsTank->SetMovingDirection(_right);
            break;
          }
          case(osgGA::GUIEventAdapter::KEY_0) : // num0
          {
            _arrowsTank->Shoot(); 
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
          case(97)  : // A
          case(100) : // D
          {
            _pressedKeys[key] = false;
            // если это была единственная нажатая клавиша то танк останавливается
            if (_pressedKeys[119])
              _wasdTank->SetMovingDirection(_up);
            else if (_pressedKeys[115])
              _wasdTank->SetMovingDirection(_down);
            else if (_pressedKeys[97])
              _wasdTank->SetMovingDirection(_left);
            else if (_pressedKeys[100])
              _wasdTank->SetMovingDirection(_right);
            else
              _wasdTank->Stop();
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
            _pressedKeys[key] = false;
            // если это была единственная нажатая клавиша то танк останавливается
            if (_pressedKeys[65362])
              _arrowsTank->SetMovingDirection(_up);
            else if (_pressedKeys[65364])
              _arrowsTank->SetMovingDirection(_down);
            else if (_pressedKeys[65361])
              _arrowsTank->SetMovingDirection(_left);
            else if (_pressedKeys[65363])
              _arrowsTank->SetMovingDirection(_right);
            else
              _arrowsTank->Stop();
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

  virtual void paintEvent(QPaintEvent* event) override
  {
    _viewer->frame();

    while (!_toDelete.empty())
    {
      _toDelete.front()->getParent(0)->removeChild(_toDelete.front());
      _toDelete.pop_front();
    }

    int hAxis, vAxis;
    bool startBtn, fireBtn;
    // центр карты
    //int centerX = _mapSize[0] * 4;
    //int centerZ = _mapSize[1] * 4;
    // расположение камеры в пространстве по осям x и z
    //int camX = _viewer->getCamera()->getViewMatrix()(3, 2) + centerX; // 3 строка 0 столбец
    //int camZ = _viewer->getCamera()->getViewMatrix()(3, 0) + centerZ;
    int camX = _viewer->getCamera()->getViewMatrix()(3, 0);
    int camZ = _viewer->getCamera()->getViewMatrix()(3, 1);

    //if (camX >= camZ && camX < -camZ)
    if (camZ < 0)
    {
      
      //if (camZ < centerZ)
      if (camX < 0)
      {
        // камера снизу
        _up = direction::UP;
        _down = direction::DOWN;
        _left = direction::LEFT;
        _right = direction::RIGHT;
      }
      else
      {
        // камера слева
        _up = direction::RIGHT;
        _down = direction::LEFT;
        _left = direction::UP;
        _right = direction::DOWN;
      }
    }
    else
    {
      //if (camX < centerX)
      if (camX > 0)
      {
        // камера сверху
        _up = direction::DOWN;
        _down = direction::UP;
        _left = direction::RIGHT;
        _right = direction::LEFT;
      }
      else
      {
        // камера справа
        _up = direction::LEFT;
        _down = direction::RIGHT;
        _left = direction::DOWN;
        _right = direction::UP;
      }
    }

    for (auto it = _tank.cbegin(); it != _tank.end(); ++it)
    {
      // управление с клавиатуры обрабатывается не здесь
      if ((*it)->GetControlDevice() < 0)
        continue;

      (*it)->Stop();

      _joy = SDL_JoystickOpen((*it)->GetControlDevice());

      SDL_JoystickUpdate();

      hAxis = SDL_JoystickGetAxis(_joy, 0); // вертикальная ось - первая
      vAxis = SDL_JoystickGetAxis(_joy, SDL_JoystickNumAxes(_joy) - 1); // горизонтальная ось - последняя
      startBtn = SDL_JoystickGetButton(_joy, 9);
      fireBtn = SDL_JoystickGetButton(_joy, 2);

      if (vAxis < -20000) // UP button
        (*it)->SetMovingDirection(_up);

      if (vAxis > 20000) // DOWN button
        (*it)->SetMovingDirection(_down);

      if (hAxis < -20000) // LEFT button
        (*it)->SetMovingDirection(_left);

      if (hAxis > 20000) // RIGHT button
        (*it)->SetMovingDirection(_right);

      if (fireBtn) // FIRE button
        (*it)->Shoot();
    }
  }

private:
  QTimer _viewerUpdateTimer;
  osgQt::GLWidget* _viewerWidget;
  QHBoxLayout* _hLayout; // размещает вьювер + панель управления
  QVBoxLayout* _vLayout; // размещает компоненты панели управления
  QTreeWidget* _playersList;
  QPushButton* _restartBtn;
  QPushButton* _addPlayerBtn;
  QPushButton* _openMapBtn;
  QPlainTextEdit* _console;

  osg::ref_ptr<osg::Node> _scene;
  osg::ref_ptr<osgViewer::Viewer> _viewer;
  osg::Vec2i _mapSize; // размер карты в тайлах
  QString _fileName; // строка для имени файла с картой
  tile mapMaker;

  int _numJoysticks; // кол-во подключенных джойстиков
  tank* _wasdTank = nullptr; // ссылка на танк управляемый WASD
  tank* _arrowsTank = nullptr; // ссылка на танк управляемый стрелками
  std::map<int, bool> _pressedKeys; // определяет зажаты ли клавиши клавиатуры в текущий момент
  int _playerNum = 0; // текущее кол-во игроков
  SDL_Joystick* _joy;
  direction _up, _down, _left, _right; // направления движения зависящие от положения камеры

  std::vector<osg::ref_ptr<tank>> _tank; // вектор содержащий все танки
  std::list<osg::Node*> _toDelete; // очередь на удаление
  std::map<osg::Vec2i, blockType> _typeMap; // список координат и типов тайлов расположенных по эти координатам
  std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>> _tileMap; // список координат и ссылок на сами тайлы расположенные по этим координатам
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
