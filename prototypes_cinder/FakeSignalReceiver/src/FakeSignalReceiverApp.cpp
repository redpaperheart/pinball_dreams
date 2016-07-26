#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "Osc.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class FakeSignalReceiverApp : public App {
  public:
    FakeSignalReceiverApp();
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
    
    string signal_str="";
    osc::ReceiverUdp mReceiver;
};

FakeSignalReceiverApp::FakeSignalReceiverApp():mReceiver(10001)
{
}

void FakeSignalReceiverApp::setup()
{
    mReceiver.setListener("/pinball_status/", [&]( const osc::Message &msg ){
        signal_str = msg[0].string();
        ci::app::console()<< "signal_str: "+signal_str << "\n";
    });
    
    mReceiver.bind();
    mReceiver.listen();
}

void FakeSignalReceiverApp::mouseDown( MouseEvent event )
{
}

void FakeSignalReceiverApp::update()
{
}

void FakeSignalReceiverApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
}

CINDER_APP( FakeSignalReceiverApp, RendererGl )
