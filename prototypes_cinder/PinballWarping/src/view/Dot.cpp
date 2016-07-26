#include "Dot.h"

Dot::Dot(ci::vec2 pos, float radius, ci::ColorA color) : mColor(color), mRadius(radius),mPos(pos){

}

void Dot::animate(){
    ci::app::timeline().apply(&mRadius, 0.0f, 0.5f).finishFn( std::bind(&Dot::die, this) );
}

void Dot::draw(){
    ci::gl::color(mColor);
    ci::gl::drawSolidCircle(mPos, mRadius);
}

