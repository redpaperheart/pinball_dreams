///Users/Eric/work/projects/pinballdreams/pinballdreams/prototypes/SerialTest1_eh/xcode/SerialTest.xcodeproj
//  Pinball.cpp
//  SerialTest
//
//  Created by Eric on 2/11/15.
//
//

#include "Pinball.h"
#include "Object.h"
#include "ObjectContainer.h"
#include "cinder/Json.h"
#include "cinder/app/App.h"
#include "cinder/Utilities.h"

void Pinball::setup()
{
    //JSON load of all pinball locations
    fs::path configPath = app::getAssetPath("location.json");
    createFromJson(configPath);
    
    mPinballImage = gl::Texture::create( loadImage( loadAsset( "pinball.jpg" ) ) );
    mPinballStates.resize(mDebugImgLocations.size());
    
    mDebugLocId = 0;
    totalLoc = mDebugImgLocations.size();
}

//______________________JSON SECTION__________________________________//

void Pinball::createFromJson(fs::path jsonFile)
{
    // load json file
    if(!fs::exists(jsonFile)) {
        console() << "No JSON" << endl;
        return;
    }
    
    // parse and add pinabll locations
    try {
        JsonTree doc( loadFile( jsonFile ) );
        
        for( auto &item : doc.getChildren() ) {
            std::string id = item["id"].getValue<std::string>();
            Location::Id locationId = Location::stringToId(id);
            vec2 pos = vec2(item["pos"][0].getValue<float>(), item["pos"][1].getValue<float>());
            float radius = item["rad"].getValue<float>();
            
            mDebugImgLocations[locationId] = Location(locationId, pos, radius);
        }
    }
    catch( const JsonTree::ExcJsonParserError& e )  {
        app::console() << "Failed to parse json file: " << e.what() << std::endl;
    }
}

//called when debug mode is turned off
void Pinball::saveJson()
{
    //save out JSON file of fixed pinball locations
    JsonTree doc = JsonTree::makeArray();
    for (auto location: mDebugImgLocations){
        JsonTree jsonObj = JsonTree::makeObject();
        std::string locName = location.second.idToString();
        jsonObj.pushBack( JsonTree( "id", locName ));
        
        JsonTree jsonPos = JsonTree::makeArray("pos");
        jsonPos.pushBack( JsonTree("", toString(location.second.pos.x)) );
        jsonPos.pushBack( JsonTree("", toString(location.second.pos.y)) );
        jsonObj.pushBack( jsonPos );
        
        jsonObj.pushBack( JsonTree( "rad", location.second.radius) );
        
        console() << jsonObj << endl;
        doc.pushBack(jsonObj);
    }
    doc.write( writeFile( app::getAssetPath("location.json")), JsonTree::WriteOptions() );
    console() << "saved Json" << endl;
}

//______________________MOUSE SECTION_______________________________//

void Pinball::mouseMove( MouseEvent event )
{
    if( !mIsInEditingMode ) return;
    mMouseLoc = event.getPos();
}

void Pinball::mouseDown( MouseEvent event)
{
    if( !mIsInEditingMode ) return;
    
    //if the mouse is inside a circle and the mouse is down,
    //move the center position of the circle to the mouse pointer
    //and save the new position of the circle
    
    //mMouseLoc = event.getPos();
    //if the distance from the circle is less than the radius, redraw the circle with a fill
    for (int n=0; n<mDebugImgLocations.size(); n++){
        for (int n=0; n<mDebugImgLocations.size(); n++){
            if(mDebugImgLocations[Location::Id(n)].radius > distance(mMouseLoc, mDebugImgLocations[Location::Id(n)].pos)){
                mDebugImgLocations[Location::Id(n)].bMouse = true;
                console() << mDebugImgLocations[Location::Id(n)].idToString() << endl;
                break;
            }
        }
        
    }
}

void Pinball::mouseDrag( MouseEvent event )
{
    if( !mIsInEditingMode ) return;

    //if the mouse is inside a circle and the mouse is down,
    //move the center position of the circle to the mouse pointer
    //and save the new position of the circle
    
    mMouseLoc = event.getPos();
    //if the distance from the circle is less than the radius, redraw the circle with a fill
    for (int n=0; n<mDebugImgLocations.size(); n++){
            if(mDebugImgLocations[Location::Id(n)].bMouse){
                //console() << n << "mouse True" << endl;
                mDebugImgLocations[Location::Id(n)].pos = mMouseLoc;
                mDebugImgLocations[Location::Id(n)].bMouse = true;
                break;
            }
    }
}

void Pinball::mouseUp( MouseEvent event )
{
    if( !mIsInEditingMode ) return;
    for (int n=0; n<mDebugImgLocations.size(); n++){
        if (mDebugImgLocations[Location::Id(n)].bMouse == true){
            console() << mDebugImgLocations[Location::Id(n)].pos << endl;
        }
        mDebugImgLocations[Location::Id(n)].bMouse = false;
    }
}

//_____________________SERIAL & UPDATE SECTION______________________________//

void Pinball::serialSplit(string serialString)
{
    //remove the brackets
    mPinBallEvents = split(serialString, ',');
    
    for (int n=0; n<mPinBallEvents.size(); n++){
        //ci::app::console() << n << ":" << mPinBallEvents[n] << std::endl;
        if ( mPinBallEvents[n] == "1"){
            if(!mDebugImgLocations[Location::Id(n)].bArduino){
                mDebugImgLocations[Location::Id(n)].bArduino = true;
            }
        }
        else{
            mDebugImgLocations[Location::Id(n)].bArduino = false;
            mDebugImgLocations[Location::Id(n)].bHasDot = false; // not sure ab this

        }
    }
}

void Pinball::update(string triggerArray)
{
    //console() << triggerArray << endl;
    serialSplit(triggerArray);
    
    for(std::map<Location::Id, Location>::iterator iterP=mDebugImgLocations.begin(); iterP!=mDebugImgLocations.end(); ++iterP){
        
        if (iterP->second.bArduino == true && iterP->second.bHasDot == false) {
            iterP->second.bHasDot = true;
            Dot *newDot = new Dot( iterP->second.pos, iterP->second.radius, ci::ColorA(0,1,0,1) );
            newDot->animate();
            mDots.addChild(newDot);
            //ci::app::console() << iterP->second.locId << ":1 ";
        }
        else {
            //ci::app::console() << iterP->second.locId << ":0 ";
        }
    }
    mDots.update();
    //ci::app::console() << "number of dots: " << mDots.getNumChildren() << std::endl;
}

//_____________________________DRAW SECTION___________________________//

void Pinball::drawImage()
{
    gl::draw(mPinballImage);
}

void Pinball::draw()
{
    for (int n=0; n<mDebugImgLocations.size(); n++) {

        if(mDebug) {
            if (mDebugImgLocations[Location::Id(n)].bMouse) {
                ci::gl::color(0.0, 0.0, 1.0);
                cinder::gl::drawSolidCircle(mDebugImgLocations[Location::Id(n)].pos, mDebugImgLocations[Location::Id(n)].radius);
            }
            else {
                ci::gl::color(0.0, 1.0, 0.0);
                cinder::gl::drawSolidCircle(mDebugImgLocations[Location::Id(n)].pos, mDebugImgLocations[Location::Id(n)].radius);
            }
        }
        cinder::gl::color(1.0f, 1.0f, 1.0f);
        mDots.draw();
    }
}

//______________________________DEBUG & MAPPING SECTION_______________________________________//

Location Pinball::selectedLoc()
{
    //switch active location being used
    //return that location
    if( !mIsInEditingMode ) {return mDebugImgLocations[Location::Id(0)];}
    if (mDebugLocId == totalLoc){
        mDebugLocId = 0;
    }
    else{
        mDebugLocId += 1;
    }
    Location location;
    location = mDebugImgLocations[Location::Id(mDebugLocId)];

    console() << mDebugImgLocations.size() << " : " << mDebugLocId << " : " << location.locId <<location.idToString() << endl;
    return location;
}

void Pinball::keyShiftLoc(Location location, int direction)
{
    if( !mIsInEditingMode ) return;
    //if (location.locId == Location::BLANK) return;
    mDebugImgLocations[location.locId].nudge(direction);
}

//NEED THIS STILL, hasn't been added yet
string Pinball::changeThreshold(Location location, int delta)
{
    if( !mIsInEditingMode ) return "";
    //if (location.locId == Location::BLANK) return "";
    //increase or decrease the total delta change which triggers an event, and send it to the arduino
    return "";
    
}


