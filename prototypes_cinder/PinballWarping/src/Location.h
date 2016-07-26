//
//  Location.h
//  PinballWarping
//
//  Created by Eric on 6/10/15.
//
//

#pragma once


class Location {
public:
    enum Id{
        FLIPPER_LEFT,
        FLIPPER_RIGHT,
        SLINGSHOT_LEFT,
        SLINGSHOT_RIGHT,
        SLINGSHOT_LEFT_UPPER,
        SLINGSHOT_RIGHT_UPPER,
        BUMPER_LEFT,
        BUMPER_RIGHT,
        BUMPER_CENTER,
        PLUNGER,
        BUTTON_SEVEN_EIGHT,
        BUTTON_TEN,
        BUTTON_JACK,
        BUTTON_QUEEN,
        BUTTON_KING_TWO,
        BUTTON_ACE,
        BUTTON_THREE,
        BUTTON_FOUR,
        BUTTON_FIVE,
        BUTTON_SIX_NINE,
        BLANK,
    };
    
    //for ease of knowing which dot is selected during debug
    std::string idToString()
    {
        switch (locId) {
            case FLIPPER_LEFT: return "FLIPPER_LEFT";
            case FLIPPER_RIGHT: return "FLIPPER_RIGHT";
            case SLINGSHOT_LEFT: return "SLINGSHOT_LEFT";
            case SLINGSHOT_RIGHT: return "SLINGSHOT_RIGHT";
            case SLINGSHOT_LEFT_UPPER: return "SLINGSHOT_LEFT_UPPER";
            case SLINGSHOT_RIGHT_UPPER: return "SLINGSHOT_RIGHT_UPPER";
            case BUMPER_LEFT: return "BUMPER_LEFT";
            case BUMPER_RIGHT: return "BUMPER_RIGHT";
            case BUMPER_CENTER: return "BUMPER_CENTER";
            case PLUNGER: return "PLUNGER";
            case BUTTON_ACE: return "BUTTON_ACE";
            case BUTTON_KING_TWO: return "BUTTON_KING_TWO";
            case BUTTON_THREE: return "BUTTON_THREE";
            case BUTTON_FOUR: return "BUTTON_FOUR";
            case BUTTON_FIVE: return "BUTTON_FIVE";
            case BUTTON_SIX_NINE: return "BUTTON_SIX_NINE";
            case BUTTON_SEVEN_EIGHT: return "BUTTON_SEVEN_EIGHT";
            case BUTTON_TEN: return "BUTTON_TEN";
            case BUTTON_JACK: return "BUTTON_JACK";
            case BUTTON_QUEEN: return "BUTTON_QUEEN";
            default:
                return "null";
        }
    }
    
    //for pulling from JSON and debug things
    static Id stringToId(std::string X)
    {
        if (X == "FLIPPER_LEFT"){ return FLIPPER_LEFT;}
        else if (X == "FLIPPER_RIGHT"){ return FLIPPER_RIGHT;}
        else if (X == "SLINGSHOT_LEFT"){ return SLINGSHOT_LEFT;}
        else if (X == "SLINGSHOT_RIGHT"){ return SLINGSHOT_RIGHT;}
        else if (X == "SLINGSHOT_LEFT_UPPER"){ return SLINGSHOT_LEFT_UPPER;}
        else if (X == "SLINGSHOT_RIGHT_UPPER"){ return SLINGSHOT_RIGHT_UPPER;}
        else if (X == "BUMPER_LEFT"){ return BUMPER_LEFT;}
        else if (X == "BUMPER_RIGHT"){ return BUMPER_RIGHT;}
        else if (X == "BUMPER_CENTER"){ return BUMPER_CENTER;}
        else if (X == "BUTTON_ACE"){ return BUTTON_ACE;}
        else if (X == "BUTTON_KING_TWO"){ return BUTTON_KING_TWO;}
        else if (X == "BUTTON_THREE"){ return BUTTON_THREE;}
        else if (X == "BUTTON_FOUR"){ return BUTTON_FOUR;}
        else if (X == "BUTTON_FIVE"){ return BUTTON_FIVE;}
        else if (X == "BUTTON_SIX_NINE"){ return BUTTON_SIX_NINE;}
        else if (X == "BUTTON_SEVEN_EIGHT"){ return BUTTON_SEVEN_EIGHT;}
        else if (X == "BUTTON_TEN"){ return BUTTON_TEN;}
        else if (X == "BUTTON_JACK"){ return BUTTON_JACK;}
        else if (X == "BUTTON_QUEEN"){ return BUTTON_QUEEN;}
        else if (X == "PLUNGER"){ return PLUNGER;}
        else{
            return BLANK;
        }
    }
    
    Location(){};
    Location(Id lId, ci::vec2 p, float r){
        bMouse = false;
        bArduino = false;
        bHasDot = false;
        locId = lId;
        pos = p;
        radius = r;
    };
    bool bArduino = false;
    bool bMouse = false;
    bool bHasDot = false;
    
    ci::vec2 pos;
    float radius;
    Id locId;
    
    int moveValue = 5;
    
    void nudge(int direction){
        switch (direction){
            case 0: pos.y = pos.y - moveValue; break; // moving up
            case 1: pos.y = pos.y + moveValue; break; // moving down
            case 2: pos.x = pos.x - moveValue; break;// moving left
            case 3: pos.x = pos.x + moveValue; break;// moving right
        }
        cinder::app::console() << pos << " : " << this->idToString() << std::endl;
    }

protected:
    
};
