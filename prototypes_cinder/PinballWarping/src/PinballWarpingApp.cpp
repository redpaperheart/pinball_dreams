#include <sstream>

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"

#include "Warp.h"
#include "Osc.h"

#include "Pinball.h"

using namespace ci;
using namespace ci::app;
using namespace ph::warping;
using namespace std;

//how many characters from serial we can read at one time
#define BUFSIZE 100

//how many sensors are being read and sent from Arduino!
#define pinBallLength = 14

class PinballWarpingApp : public App {
public:
	void setup() override;
	void cleanup() override;
	void update() override;
	void draw() override;
	void resize() override;
    
	void mouseMove( MouseEvent event ) override;
	void mouseDown( MouseEvent event ) override;
	void mouseDrag( MouseEvent event ) override;
	void mouseUp( MouseEvent event ) override;
	void keyDown( KeyEvent event ) override;
	void keyUp( KeyEvent event ) override;
    
    void setupWarps();
    void setupOsc();
    void setupParams();
    
    bool    mDrawImage = true;
    float   mFps = 0.0f;
    string  mLastString = "";
    string  mThreshSend = "";
 
    Pinball     mPinball;
    Location    debugLoc;
    WarpList	mWarps;

    params::InterfaceGlRef mParams;
    unique_ptr<osc::ReceiverUdp> mReceiver;
    
};

void PinballWarpingApp::setup()
{
    // the image right now is 460 * 768)
    setWindowSize( 600, 1000 );
    
    mPinball.setup();
    debugLoc = mPinball.selectedLoc();
    
    setupWarps();
    setupParams();
    setupOsc();
}

void PinballWarpingApp::setupWarps()
{
    fs::path settings = getAssetPath( "" ) / "warps.xml";
    
    if( fs::exists( settings ) ) {
        // load warp settings from file if one exists
        mWarps = Warp::readSettings( loadFile( settings ) );
    }
    else {
        // otherwise create a warp from scratch
        mWarps.push_back( WarpPerspectiveBilinear::create() );
    }
    
    try {
        Warp::setSize( mWarps, getWindowSize()); //vec2(400, 800) );
    }
    catch( const std::exception &e ) {
        console() << e.what() << std::endl;
    }
}

void PinballWarpingApp::setupOsc()
{
    mReceiver = make_unique<osc::ReceiverUdp>(10001);
    mReceiver->setListener("/pinball_status/", [&]( const osc::Message &msg ) {
        string signal_str = msg[0].string();
        mPinball.update(signal_str);
        console() << "signal_str: " + signal_str << "\n";
    });
    mReceiver->bind();
    mReceiver->listen();
}

void PinballWarpingApp::setupParams()
{
    mParams = params::InterfaceGl::create("params", ivec2(300,300));
    mParams->addParam("FPS", &mFps);
    mParams->addSeparator();
    mParams->addParam("Draw Image", &mDrawImage);
    mParams->addParam("Debugging", &mPinball.mDebug);
    mParams->addParam("Editing Locations", &mPinball.mIsInEditingMode).updateFn([this] {
        if (!mPinball.mDebug)
            mPinball.saveJson();
    });
  
}

void PinballWarpingApp::update()
{
    mFps = getAverageFps();
}

void PinballWarpingApp::draw()
{
	// clear the window and set the drawing color to white
	gl::clear(Color(0, 0, 0));
	gl::enableAlphaBlending( false );
    gl::color(1, 1, 1);
    
	if (mDrawImage)
        mPinball.drawImage();
    
    // iterate over the warps and draw their content
    for( auto &warp : mWarps ) {
        warp->begin();
        gl::clear(ColorA(0, 0, 0, 0));
        mPinball.draw();
        warp->end();
    }
    
    if(mParams->isVisible()) mParams->draw();
    
    gl::drawString(mLastString, vec2(1600, 150), ColorA(1, 1, 1, 1));
}

void PinballWarpingApp::resize()
{
	// tell the warps our window has been resized, so they properly scale up or down
	Warp::handleResize( mWarps );
}

void PinballWarpingApp::mouseMove( MouseEvent event )
{
	// pass this mouse event to the warp editor first
	if( !Warp::handleMouseMove( mWarps, event ) ) {
		// let your application perform its mouseMove handling here
        mPinball.mouseMove(event);
	}
}

void PinballWarpingApp::mouseDown( MouseEvent event )
{
	// pass this mouse event to the warp editor first
	if( !Warp::handleMouseDown( mWarps, event ) ) {
		// let your application perform its mouseDown handling here
        mPinball.mouseDown(event);
	}
}

void PinballWarpingApp::mouseDrag( MouseEvent event )
{
	// pass this mouse event to the warp editor first
	if( !Warp::handleMouseDrag( mWarps, event ) ) {
		// let your application perform its mouseDrag handling here
        mPinball.mouseDrag(event);
	}
}

void PinballWarpingApp::mouseUp( MouseEvent event )
{
	// pass this mouse event to the warp editor first
	if( !Warp::handleMouseUp( mWarps, event ) ) {
		// let your application perform its mouseUp handling here
        mPinball.mouseUp(event);
	}
}

void PinballWarpingApp::keyDown( KeyEvent event )
{
	// pass this key event to the warp editor first
	if( !Warp::handleKeyDown( mWarps, event ) ) {
		// warp editor did not handle the key, so handle it here
		switch( event.getCode() ) {
            case KeyEvent::KEY_ESCAPE:
                // quit the application
                quit();
                break;
            
            case KeyEvent::KEY_f:
                // toggle full screen
                setFullScreen( !isFullScreen() );
                break;
            
            case KeyEvent::KEY_w:
                // toggle warp edit mode
                Warp::enableEditMode( !Warp::isEditModeEnabled() );
                break;
                
            case KeyEvent::KEY_p:
                if(mParams->isVisible()){
                    mParams->hide();
                }else{
                    mParams->show();
                }
    
            //nudging locations when not in warp mode
            case KeyEvent::KEY_UP:
                // nudge up     pixels
                mPinball.keyShiftLoc(debugLoc, 0);;
                break;
                
            case KeyEvent::KEY_DOWN:
                // nudge down pixels
                mPinball.keyShiftLoc(debugLoc, 1);
                break;
            
            case KeyEvent::KEY_LEFT:
                // nudge left pixels
                mPinball.keyShiftLoc(debugLoc, 2);
                break;

            case KeyEvent::KEY_RIGHT:
                // nudge right pixels
                mPinball.keyShiftLoc(debugLoc, 3);
                break;

            case KeyEvent::KEY_TAB:
                // select which point we are nudging
                debugLoc = mPinball.selectedLoc();
                break;
             
            //changing the threshold values
            case KeyEvent::KEY_9:
                // select which point we are nudging
                mThreshSend = mPinball.changeThreshold(debugLoc, 0);
                break;
            
            case KeyEvent::KEY_0:
                // select which point we are nudging
                mThreshSend = mPinball.changeThreshold(debugLoc, 1);
                break;
		}
	}
}

void PinballWarpingApp::keyUp( KeyEvent event )
{
	// pass this key event to the warp editor first
	if( !Warp::handleKeyUp( mWarps, event ) ) {
		// let your application perform its keyUp handling here
	}
}

void PinballWarpingApp::cleanup()
{
	// save warp settings
	fs::path settings = getAssetPath( "" ) / "warps.xml";
	Warp::writeSettings( mWarps, writeFile( settings ) );
}

CINDER_APP( PinballWarpingApp, RendererGl(RendererGl::Options().msaa(16)) )
