#pragma once

#include <osgViewer/ViewerEventHandlers>
#include "vehicle.h"
#include "VehicleControls.h"

class KeyboardEventHandler : public osgGA::GUIEventHandler
{
public:
  KeyboardEventHandler(std::vector<VehicleControls*>& vehicleControls);
  bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter&) override;
private:
  std::vector<VehicleControls*>& _vehicleControls;
};