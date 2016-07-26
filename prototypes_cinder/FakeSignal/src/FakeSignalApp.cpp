#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Utilities.h"
#include "cinder/Json.h"

#include "Osc.h"
#include "SensorRect.h"

#include "CinderConfig.h"
#include "cinder/params/Params.h"
#include "PlayBack.hpp"

using namespace ci;
using namespace app;
using namespace std;


class FakeSignalApp : public App {
public:
    FakeSignalApp();
    void setup() override;
    void update() override;
    void draw() override;
    void mouseMove( MouseEvent event ) override;
    void mouseDown( MouseEvent event ) override;
    void fileDrop ( FileDropEvent event ) override;
    
    void sendMsg();
    void sendMsg( std::string str );
    void setupPhotoCell();
    void setupHallEffect();
    void reset();
    
    //light number is different from photocell number
    int numLights,numPhotocell, numHallEff, numSensor;
    Rectf mNewGame;
    
    //has 23 elements, stands for 13 lights, flipper, slingshots, bumpers and plunger.
    std::vector<SensorRect*> sensor_vector;
    std::unique_ptr<osc::SenderUdp> mSender;
    
    std::string mDestinationHost = "127.0.0.1";
    uint16_t mDestinationPort = 10001;
    
    params::InterfaceGlRef	mParams;
    config::ConfigRef       mConfig;

    PlayBack mPlayBack;
};

FakeSignalApp::FakeSignalApp(){}

void FakeSignalApp::setup()
{
    setFrameRate(15.0f);
    setWindowSize(900,850);
    setWindowPos(0, 0);
    
    numHallEff = 10;
    numPhotocell = 10;
    numLights = 13;
    numSensor = numHallEff + numPhotocell;
    
    setupPhotoCell();
    setupHallEffect();

    mNewGame = Rectf(200,740,300,780);

    
    fs::path p = getAppPath() / "settings.xml";
    mParams = params::InterfaceGl::create("General Settings", ivec2(250, 200) );
    mParams->setPosition(ivec2(20));
    
    mConfig = config::Config::create( mParams );
    mParams->addButton( "Save config", std::bind(&cinder::config::Config::save, mConfig, p) );
    mParams->addButton( "Load config", std::bind(&cinder::config::Config::load, mConfig, p) );
    
    //mParams->addSeparator();
    //mParams->addParam( "FPS", &mModel->mFps, "", true );
    
    mParams->addSeparator();
    mConfig->addParam( "Destination Host", &mDestinationHost );
    mConfig->addParam( "Destination Port", &mDestinationPort );
    if(fs::exists(p) )mConfig->load(p);
    
    mSender = make_unique<osc::SenderUdp>(10000, mDestinationHost, mDestinationPort);
    mSender->bind();
}

void FakeSignalApp::mouseMove(MouseEvent event)
{
    ivec2 mousePos = event.getPos();
    for (SensorRect *sensor : sensor_vector) {
        sensor->mouseOver = sensor->rect.contains(mousePos);
    }
}

void FakeSignalApp::mouseDown(MouseEvent event)
{
    for (SensorRect *sensor : sensor_vector) {
        if (sensor->mouseOver) {
            if (sensor->sensorType == SensorRect::HALL_EFFECT) {
                sensor->status = true;
            }
            else if (sensor->sensorType == SensorRect::PHOTOCELL) {
                
                switch (sensor->index){
                        //index 14 for light 2-K, both lights go off
                    case 14:
                        sensor_vector[1]->status = sensor_vector[12]->status = false;
                        break;
                        
                        //index 14 for light 7-8
                    case 10:
                        sensor_vector[6]->status = sensor_vector[7]->status = false;
                        break;
                        
                        //index 19 for light 6-9
                    case 19:
                        sensor_vector[5]->status = sensor_vector[8]->status = false;
                        break;
                        
                    default:
                        sensor->status = false;
                        break;
                }
            }
            sendMsg();
            return;
        }
    }

    if (mNewGame.contains(event.getPos())){
        ci::app::console() << "New Game: Everything Reset!" << endl;
        reset();
        return;
    }
    
    if (mPlayBack.mplayBackButton.contains(event.getPos())){
        mPlayBack.changeStatus();
    }
    
}

void FakeSignalApp::fileDrop(FileDropEvent event)
{
    float frameRate = mPlayBack.loadRecordedData(event.getFiles()[0]);
    if (frameRate > 0 ){
        app::setFrameRate(frameRate);
    }
    app::console() << "Set framerate to: " << app::getFrameRate()<< endl;
}


void FakeSignalApp::update()
{
    
    string playBackResult = mPlayBack.playback();
    if(playBackResult.compare("")){
        sendMsg(playBackResult);
        
        //parse the result and set state for every sensor
        
        //split string by comma into vector
        std::vector<int> sensorSignalVec;
        std::stringstream ss(playBackResult);
        int i;
        while (ss >> i){
            sensorSignalVec.push_back(i);
            if (ss.peek() == ',')
                ss.ignore();
        }
        
        //app::console() << "sensorSignalVec.size: " << sensorSignalVec.size() << endl;
        
        for (int counter =0; counter < sensorSignalVec.size(); counter++){
            for(SensorRect *sensor : sensor_vector){
                if (sensor->index == counter){
                    sensor->status = sensorSignalVec[counter];
                }
            }
        }
    }
}

void FakeSignalApp::draw()
{
    gl::clear( Color( 0, 0, 0 ) );
    
    for (SensorRect *sensor : sensor_vector) {
        if (sensor->mouseOver) {
            gl::ScopedColor gray(0.5, 0.5, 0.5);
            gl::drawSolidRect(sensor->rect.inflated(vec2(3)));
        }
        gl::ScopedColor col(sensor->status == true ? sensor->colorOn : sensor->colorOff);
        gl::drawSolidRect(sensor->rect);
        gl::drawStringCentered(sensor->sensorName, vec2((sensor->rect.x1+sensor->rect.x2)/2, sensor->rect.y1 - 10), Color::gray(0.5));
    }
    
    gl::color(0, 0, 1,0.5);
    gl::drawSolidRect(mNewGame);
    gl::drawStringCentered("New Game", vec2((mNewGame.x1+mNewGame.x2)/2,(mNewGame.y1+ mNewGame.y2)/2), Color(1,1,1));

    //draw playback function
    gl::color(1, 1, 1, 0.8);
    gl::drawSolidRect(Rectf (500,0,900,850));
    gl::drawStringCentered("Playback Function", vec2(700,80),Color(0,0,0));
    gl::drawStringCentered("Drag and drop JSON file here", vec2(700,120),Color(0,0,0));
    
    mPlayBack.draw();
    
    // Leave this at the end of draw(). Set hall effect sensor status back to false.
    // otherwise hall effect sensor won't change color.
    if (app::getElapsedFrames()%3 ==0){
        for (SensorRect* sensor:sensor_vector){
            if (sensor -> sensorType == SensorRect::HALL_EFFECT){
                sensor->status = false;
            }
        }
    }


    mParams->draw();
}

void FakeSignalApp::reset()
{
    for (SensorRect* sensor:sensor_vector){
        sensor->status = (sensor -> sensorType == SensorRect::PHOTOCELL);
    }
    // when everything is reset, send out a message to reset all graphics.
    sendMsg();
}


//OSC send message
void FakeSignalApp::sendMsg()
{
    //generate the signals based on the sensor_vector status
    //this vector should correspond with vector<PinballSensor*> mSensors in SerialGraph app
    
    std::vector<int> signals;
    signals.resize(numSensor, 0);

    for (SensorRect* sensor: sensor_vector){
        signals[sensor->index] = sensor->status ? 1:0 ;
    }
    
    string vector_concat="";
    
    for (int i=0; i < signals.size();i++){
        if (i>0) vector_concat += ",";
        vector_concat += std::to_string(signals[i]);
    }
    sendMsg(vector_concat);
}

void FakeSignalApp::sendMsg( std::string str )
{
    
    osc::Message msg("/pinball_status/");
    app::console() << "Send string: " << str << std::endl;
    msg.append(str);
    mSender->send(msg);
}

void FakeSignalApp::setupPhotoCell()
{
    int edge_length = 15;
    vec2 position (250, 50);
    Rectf rect;
    
    //light Ace
    sensor_vector.push_back(new SensorRect(15, SensorRect::PHOTOCELL, "Button A", Rectf(position.x -edge_length,position.y-edge_length, position.x+edge_length, position.y+edge_length)));
    
    //light 2
    rect = Rectf(100 - edge_length,650 - edge_length, 100 + edge_length, 650 + edge_length );
    sensor_vector.push_back(new SensorRect(14,SensorRect::PHOTOCELL, "Button 2",rect));
    
    //light 3,4,5
    for (int i=2; i<5; i++) {
        rect = Rectf(position.x -i*35 -edge_length, position.y + i*50 - edge_length, position.x - i*35 + edge_length, position.y +i*50 +edge_length );
        sensor_vector.push_back(new SensorRect(14+i, SensorRect::PHOTOCELL, "Button " + to_string(1+i), rect));
    }
    
    //light 6
    rect = Rectf(70 - edge_length,300 - edge_length, 70 + edge_length, 300 + edge_length );
    sensor_vector.push_back(new SensorRect(19,SensorRect::PHOTOCELL, "Button 6", rect));
    
    //light 7
    rect = Rectf(70 - edge_length,500 - edge_length, 70 + edge_length, 500 + edge_length );
    sensor_vector.push_back(new SensorRect(10,SensorRect::PHOTOCELL, "Button 7",rect));
    
    //light 8
    rect = Rectf(430 - edge_length,500 - edge_length, 430 + edge_length, 500 + edge_length );
    sensor_vector.push_back(new SensorRect(10,SensorRect::PHOTOCELL, "Button 8",rect));
    
    //light 9
    rect = Rectf(430 - edge_length,300 - edge_length, 430 + edge_length, 300 + edge_length );
    sensor_vector.push_back(new SensorRect(19,SensorRect::PHOTOCELL, "Button 9",rect));
    
    //light 10,11,12
    for(int i=2;i<5;i++){
        rect = Rectf(position.x +i*35 - edge_length,position.y+i*50 -edge_length, position.x + i*35 + edge_length, position.y +i*50 +edge_length );
        sensor_vector.push_back(new SensorRect(15-i,SensorRect::PHOTOCELL, "Button "+to_string(14-i),rect));
    }
    
    //light K
    rect = Rectf(400 - edge_length,650 - edge_length, 400 + edge_length, 650 + edge_length );
    sensor_vector.push_back(new SensorRect(14,SensorRect::PHOTOCELL, "Button K",rect));
}

void FakeSignalApp::setupHallEffect()
{
    int height = 5;
    int width = 30;
    
    Rectf rect;
    
    //left & right flipper
    rect = Rectf (150-width, 700-height, 150+width, 700+height);
    sensor_vector.push_back(new SensorRect(0, SensorRect::HALL_EFFECT, "Flipper Left",rect));
    
    rect = Rectf (350-width, 700-height, 350+width, 700+height);
    sensor_vector.push_back(new SensorRect(1, SensorRect::HALL_EFFECT, "Flipper Right", rect));
    
    //left & right slingshot
    rect = Rectf (80-height, 570-width, 80+height, 570+width);
    sensor_vector.push_back(new SensorRect(2, SensorRect::HALL_EFFECT,"Slingshot Left", rect));
    
    rect = Rectf (420-height, 570-width, 420+height, 570+width);
    sensor_vector.push_back(new SensorRect(3, SensorRect::HALL_EFFECT,"Slingshot Right", rect));
    
    //left & right flipper
    rect = Rectf (80-height, 400-width, 80+height,400+width);
    sensor_vector.push_back(new SensorRect(4, SensorRect::HALL_EFFECT, "Slingshot Upper Left", rect));
    
    rect = Rectf (420-height, 400-width, 420+height,400+width);
    sensor_vector.push_back(new SensorRect(5, SensorRect::HALL_EFFECT, "Slingshot Upper Right", rect));
    
    //left, right & center bumper
    int edge_length = 25;
    
    rect = Rectf (200-edge_length, 300-edge_length, 200+edge_length, 300+edge_length);
    sensor_vector.push_back(new SensorRect(6, SensorRect::HALL_EFFECT,"Bumper Left",rect));
    
    rect = Rectf (300-edge_length, 300-edge_length,300+edge_length, 300+edge_length);
    sensor_vector.push_back(new SensorRect(7, SensorRect::HALL_EFFECT,"Bumper Right", rect));
    
    rect = Rectf (250-edge_length, 400-edge_length, 250+edge_length, 400+edge_length);
    sensor_vector.push_back(new SensorRect(8, SensorRect::HALL_EFFECT, "Bumper Center", rect));
    
    //Plunger
    //Plunger uses IR reflective sensor but behave like hall effect sensor. Just put HALL_EFFECT for convenience
    rect = Rectf (450-height, 740-width, 450+height, 740+width);
    sensor_vector.push_back(new SensorRect(9, SensorRect::HALL_EFFECT, "Plunger", rect));
}


CINDER_APP( FakeSignalApp, RendererGl )
