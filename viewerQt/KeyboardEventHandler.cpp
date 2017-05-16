#include "KeyboardEventHandler.h"

KeyboardEventHandler::KeyboardEventHandler(std::vector<VehicleControls*>& vehicleControls)
  : _vehicleControls(vehicleControls)
{}

bool KeyboardEventHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter&)
{
  using keyboard = osgGA::GUIEventAdapter; //!< to make constants shorter

  switch (ea.getEventType())
  {
    case(keyboard::KEYDOWN) :
    {
      for (VehicleControls* vc : _vehicleControls)
      {
        vc->CheckPressedKey(static_cast<keyboard::KeySymbol>(ea.getKey()));
      }
      return true;
    }
    case(keyboard::KEYUP) :
    {
      for (VehicleControls* vc : _vehicleControls)
      {
        vc->CheckReleasedKey(static_cast<keyboard::KeySymbol>(ea.getKey()));
      }
      return true;
    }
    default:
      return false;
  }
}