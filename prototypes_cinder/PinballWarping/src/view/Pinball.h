//
//  Pinball.h
//  SerialTest
//
//  Created by Eric on 2/11/15.
//
//

#pragma once

#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Utilities.h"

// boost error hack: http://forum.libcinder.org/topic/notificationcenter-in-cinder #define BOOST_INTEL_STDCXX0X
#include <boost/signals2.hpp>

#include "ObjectContainer.h"
#include "Dot.h"
#include "Location.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class Pinball {
public:
    static Pinball* getInstance();
    void setup();
    void update(string str);
    void draw();
    void drawImage();
    
    void mouseMove( MouseEvent event );
	void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );
	void mouseUp( MouseEvent event );
    
    void createFromJson(fs::path jsonFile);
    void saveJson();
    
    Location selectedLoc();
    
    float mFps;
    bool enableImage;
    bool mIsInEditingMode = false;
    bool mDebug = false;

    void keyShiftLoc(Location location, int direction);
    string changeThreshold(Location location, int delta);
    
private:
    void serialSplit(string serialString);
    
    vector<string> mPinBallEvents;
    vector<bool> mPinballStates;
    std::map<Location::Id, Location> mDebugImgLocations;
    
    rph::ObjectContainer mDots;
    
    gl::TextureRef  mPinballImage;
    
    vec2 mMouseLoc;
    int mDebugLocId;
    int totalLoc;
    
};