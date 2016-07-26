#pragma once

#include "cinder/Timeline.h"
#include "Object.h"

class Dot : public rph::Object {
public:
    Dot(ci::vec2 pos, float radius, ci::ColorA color) ;
    ~Dot(){};
    
    virtual void draw() override;
    void animate();
    
protected:
    
    ci::vec2        mPos;
    ci::Anim<float> mRadius;
    ci::ColorA      mColor;

};