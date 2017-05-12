#pragma once

#include <osgViewer/ViewerEventHandlers>
#include "vehicle.h"

class VehicleControls
{
  using keyboardKey = osgGA::GUIEventAdapter::KeySymbol;
public:
  VehicleControls(keyboardKey up, 
                  keyboardKey down, 
                  keyboardKey left, 
                  keyboardKey right,
                  keyboardKey shot,
                  Vehicle* vehicle = nullptr);
  void SetMovingDirections( Vehicle::direction up,
                            Vehicle::direction down,
                            Vehicle::direction left,
                            Vehicle::direction right);
  void CheckPressedKey(keyboardKey pressedKey);
  void CheckReleasedKey(keyboardKey releasedKey);
  inline void SetVehicle(Vehicle* vehicle)
  {
    _vehicle = vehicle;
  }
  inline const Vehicle* GetVehicle() const
  {
    return _vehicle.get();
  }
private:
  keyboardKey _upKey, _downKey, _leftKey, _rightKey, _shotKey;
  Vehicle::direction _upDir, _downDir, _leftDir, _rightDir; // direction for vehicles depending on camera positon
  std::map<keyboardKey, bool> _pressedKeys; // determine if there are held keyboard keys in current moment
  osg::ref_ptr<Vehicle> _vehicle;
};