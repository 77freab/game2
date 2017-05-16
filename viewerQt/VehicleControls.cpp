#include "VehicleControls.h"

VehicleControls::VehicleControls( keyboardKey up,
                                  keyboardKey down,
                                  keyboardKey left,
                                  keyboardKey right,
                                  keyboardKey shot,
                                  Vehicle* vehicle) :
  _upKey(up),
  _downKey(down),
  _leftKey(left),
  _rightKey(right),
  _shotKey(shot),
  _vehicle(vehicle),
  _upDir(Vehicle::Direction::UP),
  _downDir(Vehicle::Direction::DOWN),
  _leftDir(Vehicle::Direction::LEFT),
  _rightDir(Vehicle::Direction::RIGHT)
{}

void VehicleControls::SetMovingDirections(Vehicle::Direction up,
                                          Vehicle::Direction down,
                                          Vehicle::Direction left,
                                          Vehicle::Direction right)
{
  _upDir = up;
  _downDir = down;
  _leftDir = left;
  _rightDir = right;
}

void VehicleControls::CheckPressedKey(keyboardKey pressedKey)
{
  if (_vehicle != nullptr)
  {
    if (pressedKey == _upKey)
    {
      _pressedKeys[_upKey] = true;
      _vehicle->SetMovingDirection(_upDir);
    }
    else if (pressedKey == _downKey)
    {
      _pressedKeys[_downKey] = true;
      _vehicle->SetMovingDirection(_downDir);
    }
    else if (pressedKey == _leftKey)
    {
      _pressedKeys[_leftKey] = true;
      _vehicle->SetMovingDirection(_leftDir);
    }
    else if (pressedKey == _rightKey)
    {
      _pressedKeys[_rightKey] = true;
      _vehicle->SetMovingDirection(_rightDir);
    }
    else if (pressedKey == _shotKey)
    {
      _vehicle->Shoot();
    }
  }
}

void VehicleControls::CheckReleasedKey(keyboardKey releasedKey)
{
  if (_vehicle != nullptr)
  {
    if (releasedKey == _upKey ||
      releasedKey == _downKey ||
      releasedKey == _leftKey ||
      releasedKey == _rightKey)
    {
      _pressedKeys[releasedKey] = false;
      // if there is at least one key held, then vehicle will move to that direction
      if (_pressedKeys[_upKey]) _vehicle->SetMovingDirection(_upDir);
      else if (_pressedKeys[_downKey]) _vehicle->SetMovingDirection(_downDir);
      else if (_pressedKeys[_leftKey]) _vehicle->SetMovingDirection(_leftDir);
      else if (_pressedKeys[_rightKey]) _vehicle->SetMovingDirection(_rightDir);
      else _vehicle->Stop();
    }
  }
}