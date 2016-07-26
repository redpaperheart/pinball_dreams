//
//  SensorRect.h
//  FakeSignal
//
//  Created by red paper heart on 6/29/16.
//
//

#ifndef SensorRect_h
#define SensorRect_h

#include <stdio.h>

using namespace ci;

class SensorRect{

public:
    typedef enum { PHOTOCELL, HALL_EFFECT, IR } SensorType;
    Color colorOn;
    Color colorOff;
    
    SensorRect(int id, SensorType type, std::string name, Rectf _rect);
    
    SensorType sensorType;
    std::string sensorName;
    Rectf rect;
    int index;                  // the index in the sensor array in SerialGraph
    bool mouseOver = false;
    bool status = false;        // false: off - true: on. change color base on status
    
};

#endif /* SensorRect_hpp */
