//
// This Cinder app is made to work with the pinball_serialgraph Arduino app
//

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Serial.h"
#include "cinder/Utilities.h"
#include "cinder/Json.h"
#include "cinder/Log.h"
#include "cinder/Timeline.h"

#include "CinderConfig.h"
#include "Osc.h"
#include "PinballSensor.h"

#define NUM_SENSORS 20

using namespace ci;
using namespace ci::app;
using namespace std;

class SerialGraphApp : public App {
public:
    void setup() override;
    void update() override;
    void draw() override;
    void resize() override;
    void fileDrop( FileDropEvent event ) override;
    void keyDown( KeyEvent event ) override;
    void mouseWheel( MouseEvent event ) override;
    
    void setupSerial();
    void setupOsc();
    void setupParams();
    void updateGraphSizes();
    
    void readData();
    bool processData(const std::string &msg);
    void sendOsc();
    
    void saveJsonConf();
    void loadJsonConf(fs::path path);
    void loadJsonSequence(fs::path path);
    void saveRecordedData();
    
    void playSequence();
    void pauseSequence();
    
    string getTimestamp();
    
private:
    bool bPaused = false;
    bool bSerialConnected = false;
    bool bDrawGraphs = true;
    bool bRecording = false;
    bool bPlayingSequence = false;
    
    int mSelectedSensor = 0;
    int mNumGraphsInScreen = 10;
    float mFps;
    float mScroll = 0;
    float mMaxScroll = 10000;
    float mGraphPadding = 60;
    float mRecordingStartTime;
    
    int mSequenceFrame = 0;
    Anim<float> mSequencePlayback = 0;
    vector<string> mLoadedSequence;
    float mLoadedSequenceDuration = 0;
    
    Rectf mGraphRect;
    
    SerialRef mSerial;
    std::vector<PinballSensor> mSensors;
    
    JsonTree mOscJson;
    JsonTree mSerialJson;
    
    config::ConfigRef mConfig;
    params::InterfaceGlRef mParams;
    params::InterfaceGlRef mThreshParams;
  
    std::string mOscDestinationIp = "10.0.1.3";
    uint16_t mOscDestinationPort = 10001;
    std::unique_ptr<osc::SenderUdp> mOscSender;
};

void SerialGraphApp::setup()
{
    setWindowSize(1200, 900);
    setWindowPos(20, 20);
    
    // init sensors
    mSensors = {
        PinballSensor(0, "Flipper Left", PinballSensor::HALL),
        PinballSensor(1, "Flipper Right", PinballSensor::HALL),
        PinballSensor(2, "Slingshot Left", PinballSensor::HALL),
        PinballSensor(3, "Slingshot Right", PinballSensor::HALL),
        PinballSensor(4, "Slingshot Upper Left", PinballSensor::HALL),
        PinballSensor(5, "Slingshot Upper Right", PinballSensor::HALL),
        PinballSensor(6, "Bumper Left", PinballSensor::HALL),
        PinballSensor(7, "Bumper Right", PinballSensor::HALL),
        PinballSensor(8, "Bumper Center", PinballSensor::HALL),
        PinballSensor(9, "Plunger", PinballSensor::PLUNGER),
        PinballSensor(10, "Button 7-8", PinballSensor::PHOTO),
        PinballSensor(11, "Button 10", PinballSensor::PHOTO),
        PinballSensor(12, "Button Jack", PinballSensor::PHOTO),
        PinballSensor(13, "Button Queen", PinballSensor::PHOTO),
        PinballSensor(14, "Button King 2", PinballSensor::PHOTO),
        PinballSensor(15, "Button Ace", PinballSensor::PHOTO),
        PinballSensor(16, "Button 3", PinballSensor::PHOTO),
        PinballSensor(17, "Button 4", PinballSensor::PHOTO),
        PinballSensor(18, "Button 5", PinballSensor::PHOTO),
        PinballSensor(19, "Button 6-9", PinballSensor::PHOTO),
        //PinballSensor(20, "Blank")
    };
    
    setupParams();

    updateGraphSizes();
    loadJsonConf(getAppPath());
    setupSerial();
    setupOsc();
}

void SerialGraphApp::setupSerial()
{
    try {
        Serial::Device dev = Serial::findDeviceByNameContains("tty.usbmodem");
        mSerial = Serial::create( dev, 115200 );
        mSerial->flush();
        bSerialConnected = true;
    }
    catch(SerialExc &exc) {
        console() << "Exception caugh initializing the serial: " << exc.what() << std::endl;
    }
}

void SerialGraphApp::setupOsc()
{
    try {
        mOscSender = make_unique<osc::SenderUdp>(10000, mOscDestinationIp, mOscDestinationPort);
        mOscSender->bind();
        CI_LOG_V("OSC Initialized");
    }
    catch (Exception e) {
        CI_LOG_E("Unable to initialize OSC: " << e.what());
    }
}

void SerialGraphApp::setupParams()
{
    fs::path p = getAppPath() / "settings.xml";
    mParams = params::InterfaceGl::create("Serial Pinball Reader", vec2(250, 340) * app::getWindowContentScale());
    
    // basic ui
    mConfig = config::Config::create( mParams );
    mParams->addButton( "Save config", std::bind(&cinder::config::Config::save, mConfig, p) );
    mParams->addButton( "Load config", std::bind(&cinder::config::Config::load, mConfig, p) );
    mParams->addSeparator();
    
    mParams->addParam("FPS", &mFps, "", true);
    mParams->addSeparator();
    
    mParams->addParam("Serial Connected", &bSerialConnected, "", true);
    mParams->addButton("Reconnect Serial", std::bind(&SerialGraphApp::setupSerial, this));
    mParams->addSeparator();
    
    mConfig->addParam("OSC IP", &mOscDestinationIp);
    mConfig->addParam("OSC Port", &mOscDestinationPort);
    mParams->addButton("Reconnect OSC", std::bind(&SerialGraphApp::setupOsc, this));
    mParams->addSeparator();
    
    mParams->addParam("Paused", &bPaused);
    mConfig->addParam("Draw Graphs", &bDrawGraphs);
    mConfig->addParam("Graphs on screen", &mNumGraphsInScreen).min(1).max(20).updateFn(std::bind(&SerialGraphApp::updateGraphSizes, this));
    mConfig->addParam("Delta scale", &PinballSensor::sDeltaGraphScale).min(1).max(4.0f);
    mParams->addSeparator();
    
    mParams->addParam("Record Data", &bRecording).updateFn([this] {
        if (bRecording) {
            mSerialJson = JsonTree::makeArray("serial_data");
            mOscJson = JsonTree::makeArray("osc_data");
            mRecordingStartTime = getElapsedSeconds();
        }
        else saveRecordedData();
    });
    mParams->addButton("Play", std::bind(&SerialGraphApp::playSequence, this));
    mParams->addButton("Pause", std::bind(&SerialGraphApp::pauseSequence, this));
    
    if (fs::exists(p)) mConfig->load(p);
    
    // threshold ui
    mThreshParams = params::InterfaceGl::create("Thresholds", vec2(250, 450) * app::getWindowContentScale());
    mThreshParams->setPosition(vec2(10, 360) * app::getWindowContentScale());

    mThreshParams->addButton("Save", std::bind(&SerialGraphApp::saveJsonConf, this));
    mThreshParams->addButton("Load", std::bind(&SerialGraphApp::loadJsonConf, this, getAppPath()));
    mThreshParams->addButton("Reset", [this] {
        for (PinballSensor &sensor : mSensors)
            sensor.reset();
    });
    mThreshParams->addSeparator();
    
    mThreshParams->addParam("Start Calibration", &PinballSensor::sCalibrating);
    mThreshParams->addSeparator();
    
    mThreshParams->addText("Hall effect sensors");
    mThreshParams->addParam("Global sensitivity", &PinballSensor::sDeltaThreshPct).min(0).max(1).step(0.01);
    for (PinballSensor &sensor : mSensors) {
        if (sensor.getType() == PinballSensor::HALL) {
            mThreshParams->addParam(sensor.getName(), &sensor.mMaxAvgDelta);
        }
    }
    mThreshParams->addText("Photocells");
    for (PinballSensor &sensor : mSensors) {
        if (sensor.getType() != PinballSensor::HALL) {
            mThreshParams->addParam(sensor.getName(), &sensor.mThreshold);
        }
    }
}

void SerialGraphApp::update()
{
    readData();
    mFps = getAverageFps();
}

void SerialGraphApp::draw()
{
    gl::clear( Color( 0, 0, 0 ) );
    gl::ScopedBlendAlpha alpha;
    
    if (bDrawGraphs) {
        for (int i = 0; i < mSensors.size(); i++) {
            float yOffset = i * (mGraphPadding + mGraphRect.getHeight()) - mScroll; // + 50?
            Rectf rect = mGraphRect + ivec2(0, yOffset);
            bool inScreen = getWindowBounds().intersects(Area(rect));
            
            if (inScreen) {
                if (i == mSelectedSensor) {
                    gl::color(mSensors.at(i).getColor());
                    gl::drawSolidRect(rect.inflated(vec2(2)));
                }
                if (!bPaused) {
                    mSensors.at(i).updateGraph();
                }
                mSensors.at(i).draw(rect, mGraphPadding);
            }
        }
    }
    
    if (bRecording) {
        gl::ScopedColor red(1, 0, 0);
        gl::drawSolidCircle(vec2(20, 340), 10);
        gl::drawString("RECORDING", vec2(40, 340));
    }
    mParams->draw();
    mThreshParams->draw();
}

void SerialGraphApp::resize()
{
    updateGraphSizes();
}

void SerialGraphApp::readData()
{
    if (mSerial && !bPlayingSequence) {
        try{
            while (mSerial->getNumBytesAvailable() > 0) {
                std::string message = mSerial->readStringUntil('\n');
                //ci::app::console() << "serial msg:" << message << std::endl;
                
                if (bRecording) {
                    JsonTree oneSignal = JsonTree::makeObject();
                    oneSignal.pushBack( JsonTree( "signal", message.erase(message.size()-4) ) );
                    oneSignal.pushBack( JsonTree( "frameCnt", app::getElapsedFrames() ) );
                    oneSignal.pushBack( JsonTree( "time", app::getElapsedSeconds() - mRecordingStartTime) );
                    mSerialJson.pushBack(oneSignal);
                }
                if (processData(message)) {
                    sendOsc();
                }
            }
        }
        catch (std::exception e) {
            bSerialConnected = false;
            CI_LOG_E(e.what());
        }
    }
}

bool SerialGraphApp::processData(const std::string &message)
{
    bool triggered = false;
    std::vector<std::string> sensorValues = split(message, ',');
    
    if (sensorValues.size() >= 20) {
        try {
            for (int i = 0; i < mSensors.size(); i++) {
                int val = stoi(sensorValues.at(i));
                triggered = mSensors.at(i).addValue(val) || triggered;
            }
        }
        catch (std::exception &e) {
            CI_LOG_E(e.what());
        }
    }
    else {
        CI_LOG_E("Message too short. Ignoring it: " << message);
    }
    return triggered;
}

void SerialGraphApp::updateGraphSizes()
{
    if (mNumGraphsInScreen < 8)         mGraphPadding = 50;
    else if (mNumGraphsInScreen < 12)   mGraphPadding = 25;
    else                                mGraphPadding = 5;
    
    float graphHeight = (getWindowHeight() - mGraphPadding * mNumGraphsInScreen - 50) / mNumGraphsInScreen;
    mGraphRect = Rectf(325, 50, getWindowWidth() - 50, 50 + graphHeight);
    mMaxScroll = float(mSensors.size() - mNumGraphsInScreen) * (graphHeight + mGraphPadding);
}

//OSC send message
void SerialGraphApp::sendOsc()
{
    string str = "";
    
    for (PinballSensor &sensor : mSensors) {
        str += std::to_string(sensor.getState()) + ",";
    }
    
    osc::Message msg("/pinball_status/");
    msg.append(str);
    mOscSender->send(msg);
    CI_LOG_V(msg);
    
    //after sending the message, save the str to JSON file
    if (bRecording) {
        JsonTree oneSignal = JsonTree::makeObject();
        oneSignal.pushBack( JsonTree( "signal", str ) );
        oneSignal.pushBack( JsonTree( "frameCnt", app::getElapsedFrames() ) );
        oneSignal.pushBack( JsonTree( "time", app::getElapsedSeconds() - mRecordingStartTime) );
        mOscJson.pushBack(oneSignal);
    }
}

void SerialGraphApp::playSequence()
{
    if (mLoadedSequence.empty()) return;
    
    timeline().apply(&mSequencePlayback, 1.0f, mLoadedSequenceDuration * (1.0f - mSequencePlayback))
    .loop()
    .startFn([this] {
        bPlayingSequence = true;
        mSequenceFrame = 0;
    })
    .updateFn([this] {
        bool triggered = false;
        int newFrame = float(mLoadedSequence.size() - 1) * mSequencePlayback;
        for (; mSequenceFrame < newFrame; mSequenceFrame++) {
            triggered = processData(mLoadedSequence.at(mSequenceFrame));
        }
        if (triggered) {
            sendOsc();
        }
    })
    .finishFn([this] {
        bPlayingSequence = false;
    });
}

void SerialGraphApp::pauseSequence()
{
    if (mLoadedSequence.empty()) return;
    
    mSequencePlayback.stop();
    bPlayingSequence = false;
}

//-----------------------------------------------------------------
#pragma mark Callbacks
//-----------------------------------------------------------------

void SerialGraphApp::keyDown(KeyEvent event)
{
    switch (event.getCode()) {
        case KeyEvent::KEY_SPACE:
            bPaused = !bPaused;
            if (!mSerial || !bSerialConnected) {
                if (bPlayingSequence) pauseSequence();
                else playSequence();
            }
            break;
        case KeyEvent::KEY_r:
            if (event.isAltDown() || event.isMetaDown()) {
                for (int i = 0; i < NUM_SENSORS; i++) {
                    mSensors[i].reset();
                }
            }
            else mSensors.at(mSelectedSensor).reset();
            break;
        case KeyEvent::KEY_s:
            saveJsonConf();
            break;
        case KeyEvent::KEY_l:
            loadJsonConf(getAppPath());
            break;
        case KeyEvent::KEY_t:
            setupSerial();
            break;
        case KeyEvent::KEY_UP:
            mSelectedSensor--;
            mSelectedSensor = glm::clamp(mSelectedSensor, 0, (int)mSensors.size() - 1);
            break;
        case KeyEvent::KEY_DOWN:
            mSelectedSensor++;
            mSelectedSensor = glm::clamp(mSelectedSensor, 0, (int)mSensors.size() - 1);
            break;
        case KeyEvent::KEY_RIGHT:
            //            mDisplayedSensors[mSelectedSensor]++;
            //            mDisplayedSensors[mSelectedSensor] = glm::clamp(mDisplayedSensors[mSelectedSensor], 0, NUM_SENSORS - 1);
            break;
        case KeyEvent::KEY_LEFT:
            //            mDisplayedSensors[mSelectedSensor]--;
            //            mDisplayedSensors[mSelectedSensor] = glm::clamp(mDisplayedSensors[mSelectedSensor], 0, NUM_SENSORS - 1);
            break;
            
            //save real time data
        case KeyEvent::KEY_d:
            saveRecordedData();
            break;
        default:
            break;
    }
}

void SerialGraphApp::mouseWheel(MouseEvent event)
{
    mScroll = glm::clamp(mScroll + event.getWheelIncrement() * 10.0f , 0.0f, mMaxScroll);
}

void SerialGraphApp::fileDrop(FileDropEvent event)
{
    fs::path file = event.getFiles()[0];
    if (file.extension() == ".json") {
        std::string stem = file.stem().string();
        
        if (stem.find("pinballconfig_") != string::npos) {
            loadJsonConf(file);
        }
        else if (stem.find("pinballserial_") != string::npos) {
            loadJsonSequence(file);
        }
        else if (stem.find("pinballosc_") != string::npos) {
        }
    }
}

//-----------------------------------------------------------------
#pragma mark Saving and loading config
//-----------------------------------------------------------------

void SerialGraphApp::saveJsonConf()
{
    //    fs::path savePath = getHomeDirectory() / "Desktop" / "Pinball_Sensors" / ("pinball_" + getTimestamp() + ".json");
    fs::path savePath = getAppPath() / ("pinballconfig_" + getTimestamp() + ".json");
    
    JsonTree doc = JsonTree::makeObject();
    JsonTree sensors = JsonTree::makeArray("sensors");
    
    for (int i = 0; i < mSensors.size(); i++) {
        PinballSensor::Type type = mSensors[i].getType();
        
        JsonTree sensor = JsonTree::makeObject();
        sensor.pushBack( JsonTree( "id", i ) );
        sensor.pushBack( JsonTree( "name", mSensors[i].getName() ) );
        sensor.pushBack( JsonTree( "type", PinballSensor::typeToString(type)) );
        sensor.pushBack( JsonTree( "min_val", mSensors[i].mMinVal ) );
        sensor.pushBack( JsonTree( "max_val", mSensors[i].mMaxVal ) );
        
        if (type == PinballSensor::HALL) {
            sensor.pushBack( JsonTree( "max_frame_delta", mSensors[i].mMaxFrameDelta ) );
            sensor.pushBack( JsonTree( "max_avg_delta", mSensors[i].mMaxAvgDelta ) );
        }
        else {
            sensor.pushBack( JsonTree( "threshold", mSensors[i].mThreshold ) );
        }
        
        sensors.pushBack(sensor);
    }
    doc.pushBack(JsonTree("thresh_delta_pct", PinballSensor::sDeltaThreshPct));
    doc.pushBack(sensors);
    doc.write(savePath);
    app::console() << "Saved JSON file: " << savePath << std::endl;
}

void SerialGraphApp::loadJsonSequence(fs::path path)
{
    try {
        mLoadedSequence.clear();
        
        JsonTree doc(loadFile(path));
        auto nodes = doc["serial_data"].getChildren();
        if (nodes.size() > 2) {
            for (auto &node : nodes) {
                mLoadedSequence.push_back(node["signal"].getValue<string>());
            }
        
            mLoadedSequenceDuration = nodes.back()["time"].getValue<float>() - nodes.front()["time"].getValue<float>();
            float fps = float(mLoadedSequence.size()) / mLoadedSequenceDuration;
            
            CI_LOG_V("Loaded sequence of " << mLoadedSequence.size() << " frames, " << mLoadedSequenceDuration << " s and " << fps << " fps.");
        }
        
    }
    catch(Exception e)  {
        CI_LOG_E("Failed to parse json file: " << e.what());
        return;
    }
}

void SerialGraphApp::loadJsonConf(fs::path path)
{
    // find latest config file in app folder
    fs::path latestConfig;
    
    if (fs::is_directory(path)) {
        for (fs::directory_iterator it(getAppPath()); it != fs::directory_iterator(); ++it) {
            if (fs::is_regular_file(*it) && it->path().extension() == ".json") {
                fs::path jsonPath = it->path();
                std::string stem = jsonPath.stem().string();
                
                if (stem.find("pinballconfig_") != string::npos) {
                    if (latestConfig.empty()) {
                        latestConfig = jsonPath;
                    }
                    else if(std::difftime(fs::last_write_time(jsonPath), fs::last_write_time(latestConfig)) > 0) {
                        latestConfig = jsonPath;
                    }
                }
            }
        }
    }
    else if (fs::is_regular_file(path) && path.extension() == ".json") {
        latestConfig = path;
    }
    
    if(!fs::exists(latestConfig)) {
        CI_LOG_E(latestConfig << " doesn't exist!");
        return;
    }
    
    try {
        JsonTree doc(loadFile(latestConfig));
        PinballSensor::sDeltaThreshPct = doc["thresh_delta_pct"].getValue<float>();
        
        for (auto &sensor : doc["sensors"].getChildren()) {
            PinballSensor &s = mSensors[sensor["id"].getValue<int>()];
            s.mMinVal = sensor["min_val"].getValue<float>();
            s.mMaxVal = sensor["max_val"].getValue<float>();
            
            if (s.getType() == PinballSensor::HALL) {
                s.mMaxAvgDelta = sensor["max_avg_delta"].getValue<float>(); // this is the only that we really need
                s.mMaxFrameDelta = sensor["max_frame_delta"].getValue<int>();
            }
            else {
                s.mThreshold = sensor["threshold"].getValue<float>();
            }
        }
    }
    catch(Exception e)  {
        CI_LOG_E("Failed to parse json file: " << e.what());
        return;
    }
}

void SerialGraphApp::saveRecordedData()
{
    std::string ts = getTimestamp();
    
    fs::path oscPath = getAppPath() / ("pinballosc_" + ts + ".json");
    JsonTree oscJson = JsonTree::makeObject();
    oscJson.pushBack(mOscJson);
    oscJson.write(oscPath);

    fs::path serialPath = getAppPath() / ("pinballserial_" + ts + ".json");
    JsonTree serialJson = JsonTree::makeObject();
    serialJson.pushBack(mSerialJson);
    serialJson.write(serialPath);

    CI_LOG_V("Saved recorded data json in " << oscPath << " and " << serialPath);
}

string SerialGraphApp::getTimestamp()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S");
    return ss.str();
}

CINDER_APP( SerialGraphApp, RendererGl, [&]( App::Settings *settings ) {
    settings->setHighDensityDisplayEnabled(true);
})
