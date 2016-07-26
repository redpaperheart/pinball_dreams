//
//  PlayBack.hpp
//  FakeSignal
//
//  Created by red paper heart on 7/14/16.
//
//

#ifndef PlayBack_hpp
#define PlayBack_hpp

#include <stdio.h>

#endif /* PlayBack_hpp */

using namespace ci;
using namespace std;

class PlayBack{

public:
    typedef enum {GREY,START,PAUSE} playBackStatus;
    playBackStatus mButtonStatus = GREY;
    Rectf mplayBackButton;
    
    bool mIfPlayBack = false;
    int mStartFrame = 0;             // loop playback
    int mPlaybackStartFrame = 0;    // the first framecount in the data is too big
    vector<string> mSignalVec;
    vector<int> mFrameVec;
    vector<string> mCommentVec;
    vector<float> mTimeVec;
    
    int mVectCounter = 0;
    int mStopeFrame, mRestartFrame, mBreakTime = 0;
    
    PlayBack();
    void draw();
    void changeStatus();
    float loadRecordedData(fs::path path);
    string playback();
};
