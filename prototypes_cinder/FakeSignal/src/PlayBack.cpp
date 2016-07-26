//
//  PlayBack.cpp
//  FakeSignal
//
//  Created by red paper heart on 7/14/16.
//
//

#include "PlayBack.hpp"
#include "cinder/Json.h"

PlayBack::PlayBack()
{
    mplayBackButton = Rectf(660,150,740,190);
}

void PlayBack::draw()
{
    switch (mButtonStatus){
        case GREY:
            gl::color(0, 0, 0,0.8);
            break;
        case START:
        case PAUSE:
            gl::color(0, 0, 1,0.5);
            break;
        default:break;
    }
    gl::drawSolidRect(mplayBackButton);
    
    switch (mButtonStatus){
        case GREY:
            gl::drawStringCentered("Start", vec2(700,170),Color(1,1,1));
            break;
        case START:
            gl::drawStringCentered("Pause", vec2(700,170),Color(0,0,0));
            break;
        case PAUSE:
            gl::drawStringCentered("Start", vec2(700,170),Color(0,0,0));
            break;
        default:break;
    }
    
    
    if(mIfPlayBack){
        gl::drawStringCentered(to_string( mVectCounter)+"/"+to_string(mFrameVec.size()), vec2(700,210),Color(0,0,0));
    }
}

float PlayBack::loadRecordedData(fs::path path)
{
    fs::path recordedData;
    
    if (fs::is_regular_file(path) && path.extension() == ".json") {
        recordedData = path;
    }else {
        app::console() << "It's not JSON file!" << endl;
        return 0;
    }
    
    string fileName = path.stem().string();
    mSignalVec.clear();
    mFrameVec.clear();
    mVectCounter = 0;
    
    
    //app::console() << "JSON file type" << " , " << mPlayBack <<  endl;
    
    //parsing json file. load data
    try{
        JsonTree doc (loadFile(recordedData));
        int i = 0;
        for (auto &signal:doc["osc_data"].getChildren()){
            
            string signalArr = signal["signal"].getValue<string>();
            int frameCnt = signal["frameCnt"].getValue<int>();
            string comment = signal["comment"].getValue<string>();
            float time = signal["time"].getValue<float>();
            
            if(i == 0 ) mPlaybackStartFrame = frameCnt-5;
            
            mSignalVec.push_back(signalArr);
            mFrameVec.push_back(frameCnt);
            mCommentVec.push_back(comment);
            mTimeVec.push_back(time);
            
            i++;
            
        }
        app::console() <<"data amount: " <<mFrameVec.size() << endl;
        
    }catch(Exception e){
        app::console() << e.what() << endl;
        return 0;
    }
    
    mIfPlayBack = true;
    mStartFrame = app::getElapsedFrames();
    mButtonStatus = START;
    mBreakTime = 0;
    
    //calculate the average framerate
    float avgFrameRate = 0;
    for (int i = 1; i<mFrameVec.size();i++){
        avgFrameRate+= (mFrameVec[i]-mFrameVec[i-1])/(mTimeVec[i]-mTimeVec[i-1]);
    }
    return avgFrameRate/(mFrameVec.size()-1);
    
}

void PlayBack::changeStatus()
{
    switch (mButtonStatus){
        case START:
            mButtonStatus = PAUSE;
            mIfPlayBack = false;
            mStopeFrame = app::getElapsedFrames();
            app::console() << "Stop Frame: "<< mStopeFrame << endl;
            break;
        case PAUSE:
            mButtonStatus = START;
            mIfPlayBack = true;
            mRestartFrame = app::getElapsedFrames();
            mBreakTime += mRestartFrame - mStopeFrame;
            app::console() << "Restart Frame: "<< mRestartFrame << "  Break Time:" << mBreakTime << endl;
            break;
        default:break;
    }
}

string PlayBack::playback()
{
    string returnStr = "";
    
    if(mIfPlayBack){
        
        //app::console() << app::getElapsedFrames() - mStartFrame << "   " << mFrameVec[mVectCounter]-mPlaybackStartFrame << endl;
        
        if ((app::getElapsedFrames() - mStartFrame - mBreakTime) == mFrameVec[mVectCounter]- mPlaybackStartFrame){
            
            app::console() << mVectCounter+1 << "/" << mFrameVec.size() << "   "<< mCommentVec[mVectCounter] << "  ";
            
            //sendMsg( mSignalVec[mVectCounter] );
            
            returnStr = mSignalVec[mVectCounter];
            mVectCounter++;
        }
        
        if( mVectCounter == mFrameVec.size() ){
            mVectCounter = 0;
            mStartFrame = app::getElapsedFrames() + 30;
            mBreakTime = 0;
        }
        
    }
    
    return returnStr;
}