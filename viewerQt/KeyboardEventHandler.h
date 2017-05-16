#pragma once

#include <osgViewer/ViewerEventHandlers>
#include "vehicle.h"
#include "VehicleControls.h"

class KeyboardEventHandler : public osgGA::GUIEventHandler
{
public:
  KeyboardEventHandler(std::vector<VehicleControls*>& vehicleControls);

  KeyboardEventHandler(const KeyboardEventHandler&) = delete;
  KeyboardEventHandler& operator=(const KeyboardEventHandler&) = delete;

  virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter&) override;

protected:
  virtual ~KeyboardEventHandler() {}

private:
  std::vector<VehicleControls*>& _vehicleControls;
};