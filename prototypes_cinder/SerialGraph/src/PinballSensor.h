//
//  PinballSensor.h
//  SerialGraph
//
//  Created by Adrià Navarro López on 7/20/15.
//
//

#define BUFFER_CAPACITY 200

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Shape2d.h"
#include "cinder/Utilities.h"
#include "cinder/Rand.h"

#include <boost/circular_buffer.hpp>

using namespace ci;

class PinballSensor {
  public:
    typedef enum {HALL, PHOTO, PLUNGER } Type;
    static std::string typeToString(Type type);
    
    PinballSensor(int id, std::string name, Type type);
    void reset();
    void updateGraph();
    void draw(Rectf graphRect, float padding);
    bool addValue(int val); // returns true if there was a change in the sensor after adding the value
    
    const Type &getType()           { return mType; }
    const Color &getColor()         { return mColor; }
    const std::string &getName()    { return mName; }
    const bool getState()           { return mBuffer.empty() ? false : mBuffer.back().state; }
    const bool getStable()          { return mBuffer.empty() ? true : mBuffer.back().stable; }
    
    // if set to true, thresholds will be max delta * sDeltaThreshPct
    static bool sCalibrating;
    static float sDeltaThreshPct;   // this acts as a sensitivity setting
    static float sDeltaGraphScale;
    const static int sFramesForStability = 5;
    
    // for hall effects
    int mMaxFrameDelta = 0;
    int mMaxAvgDelta = 0;
    float mVariance = 0;
    float mDeltaThreshold = 512;
    int mFramesLookingStable = 0;
    
    // for photocells and plunger
    int mAvgVal = 0;
    int mMinVal = 1024;
    int mMaxVal = 0;
    float mThreshold = 512;
    
  private:
    int mId;
    Type mType;
    std::string mName;

    // data history
    typedef struct {
        int val;
        float delta;
        bool trigger;
        bool state;
        bool stable;
    } SensorFrame;
    boost::circular_buffer<SensorFrame> mBuffer;
    
    // graph
    Color               mColor;
    Shape2d             mGraph;
    Shape2d             mDeltaGraph;
    gl::VertBatchRef    mGraphPoints;
    gl::VertBatchRef    mTriggerLines;
    gl::VertBatchRef    mStateLines;
};