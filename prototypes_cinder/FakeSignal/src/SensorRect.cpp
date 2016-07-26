//
//  SensorRect.cpp
//  FakeSignal
//
//  Created by red paper heart on 6/29/16.
//
//

#include "SensorRect.h"

using namespace ci;


SensorRect::SensorRect(int id, SensorType type, std::string name, Rectf _rect)
{
    index=id;
    sensorType = type;
    sensorName = name;
    rect = _rect;
    status = (sensorType == PHOTOCELL); // photocells start ON
    
    colorOn  = (sensorType == PHOTOCELL ? Color(1, 1, 0) : Color(1,0,0));
    colorOff = Color(1, 1, 1);
    
}