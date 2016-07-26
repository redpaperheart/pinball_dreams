//
//  PinballSensor.cpp
//  SerialGraph
//
//  Created by Adrià Navarro López on 7/20/15.
//
//

#include "PinballSensor.h"

bool PinballSensor::sCalibrating = false;
float PinballSensor::sDeltaThreshPct = 0.35;
float PinballSensor::sDeltaGraphScale = 3.5f;

std::string PinballSensor::typeToString(Type type)
{
    if (type == HALL) return "hall";
    if (type == PHOTO) return "photo";
    if (type == PLUNGER) return "plunger";
    return "unknown";
}

PinballSensor::PinballSensor(int id, std::string name, Type type)
{
    mId = id;
    mType = type;
    mName = name;
    mColor = Color(CM_HSV, randFloat(), 0.8f, 1.0f);
    mBuffer.set_capacity(BUFFER_CAPACITY);
    reset();
}

bool PinballSensor::addValue(int val)
{
    // calculate averages and deltas
    mAvgVal = mBuffer.empty() ? val : 0.99f * mAvgVal + 0.01f * (float)val;
    int deltaFrame = mBuffer.empty() ? 0 : glm::abs(val - mBuffer.back().val);
    float deltaAvg = glm::abs((float)val - mAvgVal);
    
    // if calibrating, calculate delta distances and min/max to update thresholds
    if (sCalibrating) {
        mMinVal = glm::min(mMinVal, val);
        mMaxVal = glm::max(mMaxVal, val);
        mMaxFrameDelta = glm::max(mMaxFrameDelta, deltaFrame);
        mMaxAvgDelta = math<float>::max(mMaxAvgDelta, deltaAvg);
        mThreshold = float(mMinVal + mMaxVal) / 2.0f;
    }
    
    // calculate thresholds
    mVariance = 0.95f * mVariance + 0.05f * deltaAvg;
//    mDeltaThreshold = mMaxAvgDelta * sDeltaThreshPct;
    mDeltaThreshold = std::max(mVariance * 1.15f, mMaxAvgDelta * sDeltaThreshPct);
    
    // detect changes
    bool triggered = false;
    bool state = false;
    bool stable = true;
    
    if (mType == HALL) {
        if (mBuffer.empty()) {
            mVariance = deltaAvg;
        }
        else {
            if (deltaAvg > mDeltaThreshold) {
                stable = false;
                state = true;
                mFramesLookingStable = 0;
            }
            else {
                mFramesLookingStable ++;
                if (mFramesLookingStable > 5) {
                    stable = true;
                    state = false;
                }
                else {
                    stable = false;
                    state = true;
                }
            }
            triggered = state && !mBuffer.back().state; // trigger: change from low to high
        }
    }
    else if (mType == PHOTO) {
        if (mBuffer.empty()) {
            state = true;
            triggered = true;
        }
        else {
            state = val > mThreshold;
            stable = true;
            triggered = (state != mBuffer.back().state); // trigger: any state change
        }
    }
    else if (mType == PLUNGER) {
        if (mBuffer.empty()) {
            state = false;
            stable = true;
            triggered = false;
        }
        else {
            // we consider it unstable when we're pulling it
            stable = val < mThreshold;
            
            // when we let it go it goes from unstable to stable so we trigger it
            // and make state 1 for just a frame
            state = stable && !mBuffer.back().stable; //
            triggered = state;
        }
    }
    
    SensorFrame sf = {val, deltaAvg, triggered, state, stable};
    mBuffer.push_back(sf);
    
    return triggered;
}

void PinballSensor::reset()
{
    mMinVal = 1024;
    mMaxVal = 0;
    mAvgVal = 512;
    mMaxFrameDelta = 0;
    mMaxAvgDelta = 0;
    mFramesLookingStable = 0;
    mBuffer.clear();
}

void PinballSensor::draw(Rectf graphRect, float padding)
{
    gl::ScopedBlendAlpha alpha;
    
    // graph background
    Color gray = Color::gray(0.5);
    gl::color(gray);
    gl::drawSolidRect(graphRect);
    
    std::string title = "SENSOR: " + toString(mId) + ": " + mName;
    
    if (padding > 30) {
        gl::drawStringRight("1024", graphRect.getUpperLeft() - vec2(15, 0), gray);
        gl::drawStringRight("0", graphRect.getLowerLeft() - vec2(15, 0), gray);
        gl::drawStringRight(toString(BUFFER_CAPACITY), graphRect.getLowerRight() + vec2(0, 15), gray);
        
    }
    if (padding > 20) {
        std::string data =
        "AVG: " + toString((int) mAvgVal) + "     " +
        "VAR: " + toString((int) mVariance) + "     " +
        "MIN: " + toString((int) mMinVal) + "     " +
        "MAX: " + toString((int) mMaxVal) + "     " +
        "DELTA: " + toString((int) mBuffer.empty() ? 0 : mBuffer.back().delta ) + "     " +
        "THRESH: " + toString(mType == HALL ? (int)mDeltaThreshold : (int)mThreshold);
        
        gl::drawString(title, graphRect.getUpperLeft() - vec2(0, 15));
        gl::drawStringRight(data, graphRect.getUpperRight() - vec2(0, 15));
        
        if (!mBuffer.empty()) {
            float y = lmap((float) mBuffer.back().val, 0.0f, 1024.0f, graphRect.y2, graphRect.y1);
            gl::drawStringRight(toString(mBuffer.back().val), vec2(graphRect.x1 - 15, y));
        }
    }
    else {
        gl::drawString(title, graphRect.getUpperLeft() + vec2(1, 5));
    }
    
    // graph
    gl::ScopedMatrices mat;
    gl::translate(graphRect.getLowerLeft());
    gl::scale(graphRect.getSize());
    
    gl::color(mColor);
    gl::drawSolid(mGraph);
    
    gl::color(0, 0, 0);
    if (mStateLines) mStateLines->draw();
    
    gl::color(1, 1, 1);
    gl::draw(mDeltaGraph);
    
    if (mType == HALL)
        gl::drawLine(vec2(0, -mDeltaThreshold * sDeltaGraphScale / 1024.f), vec2(1, -mDeltaThreshold * sDeltaGraphScale / 1024.f));
    else
        gl::drawLine(vec2(0, -mThreshold / 1024.f), vec2(1, -mThreshold / 1024.0f));
    
    if (mGraphPoints) mGraphPoints->draw();
    if (mTriggerLines) mTriggerLines->draw();
    
    
}

void PinballSensor::updateGraph()
{
    if (mBuffer.empty()) return ;
    
    mGraphPoints = gl::VertBatch::create(GL_POINTS);
    mTriggerLines = gl::VertBatch::create(GL_LINES);
    mStateLines = gl::VertBatch::create(GL_LINE_STRIP);
    mGraph.clear();
    mGraph.moveTo(0, 0);
    mDeltaGraph.clear();
    mDeltaGraph.moveTo(0, 0);
    
    for (int i = 0; i < mBuffer.size(); i++) {
        float pct = float(i) / float(BUFFER_CAPACITY - 1);
        vec2 point( pct, -float(mBuffer.at(i).val / 1024.f) );
        mGraph.lineTo(point);
        mGraphPoints->vertex(point);
        
        vec2 deltaPoint( pct, -float(mBuffer.at(i).delta * sDeltaGraphScale/ 1024.f) );
        mDeltaGraph.lineTo(deltaPoint);
        
        if (mBuffer.at(i).trigger) {
            mTriggerLines->vertex(pct, 0);
            mTriggerLines->vertex(pct, -1);
        }
        mStateLines->vertex(pct, mBuffer.at(i).state ? -0.85 : -0.15);
    }
    
    mGraph.lineTo(float(mBuffer.size()) / float(BUFFER_CAPACITY), 0 );
    mGraph.close();
}
