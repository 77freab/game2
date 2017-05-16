#pragma once

#include <osgViewer/ViewerEventHandlers>
#include "vehicle.h"

class VehicleControls
{
  typedef osgGA::GUIEventAdapter::KeySymbol keyboardKey;
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

  //! send moving/shooting commands to vehicle according to pressed keys
  void CheckPressedKey(keyboardKey pressedKey);
  //! stops vehicle if all keys are released
  void CheckReleasedKey(keyboardKey releasedKey);

  inline void SetVehicle(Vehicle* vehicle);
  inline const Vehicle* GetVehicle() const;

private:
  keyboardKey _upKey, 
              _downKey, 
              _leftKey, 
              _rightKey, 
              _shotKey;
  //! directions for vehicles depending on camera positon
  Vehicle::direction  _upDir, 
                      _downDir, 
                      _leftDir, 
                      _rightDir;
  //! determine if there are held keyboard keys in current moment
  std::map<keyboardKey, bool> _pressedKeys;
  osg::ref_ptr<Vehicle> _vehicle;
};

inline void VehicleControls::SetVehicle(Vehicle* vehicle)
{
  _vehicle = vehicle;
}

inline const Vehicle* VehicleControls::GetVehicle() const
{
  return _vehicle.get();
}