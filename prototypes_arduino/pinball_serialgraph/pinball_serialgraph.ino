/* Pinball_SerialGraph is to be used with SerialGraph Cinder app
 * the locations of the pins/sensors is based on the soldered pcb
 * so it's a bit different from the breadboard/prototyping version

 * Sensor array
 *  0   : Left Flipper
 *  1   : Right Flipper
 *  2   : Left Slingshot
 *  3   : Right Slingshot
 *  4   : Left Upper Slingshot
 *  5   : Right Upper Slingshot
 *  6   : Left Bumper
 *  7   : Right Bumper
 *  8   : Center Bumper
 *  9   : Plunger
 *  10  : 7 & 8 Light 
 *  11  : 10 Light
 *  12  : J Light
 *  13  : Q Light
 *  14  : K & 2 Light
 *  15  : A Light
 *  16  : 3 Light
 *  17  : 4 Light
 *  18  : 5 Light
 *  19  : 6 & 9 Light
 */
 
// Wiring:
//Mux1 control pins
int s0 = A2;
int s1 = A3;
int s2 = 12;
int s3 = 11;

//Mux2 control pins
int p0 = A4;
int p1 = A5;
int p2 = 10;
int p3 = 9;

//Mux in "SIG" pin for both of them
const int SIG_pin = A0;
const int SIG_pin2 = A1;
const int numHall = 10;                                   //number of Hall effects sensors, plus the distance sensor for the plunger
const int numPhoto = 10;                                    //numer of photocell sensors
const int numSensors = numHall + numPhoto ;              //total number of sensors in the circuit

// mapping between the sensor order (top of the document)
// and thir channel in the multiplexer
const int sensorChannels[10] = {
  7,
  6,
  5,
  4,
  3,
  12,
  11,
  10,
  9,
  8
};

const int muxChannel[16][4] = {
  {0, 0, 0, 0}, //channel 0
  {1, 0, 0, 0}, //channel 1
  {0, 1, 0, 0}, //channel 2
  {1, 1, 0, 0}, //channel 3
  {0, 0, 1, 0}, //channel 4
  {1, 0, 1, 0}, //channel 5
  {0, 1, 1, 0}, //channel 6
  {1, 1, 1, 0}, //channel 7
  {0, 0, 0, 1}, //channel 8
  {1, 0, 0, 1}, //channel 9
  {0, 1, 0, 1}, //channel 10
  {1, 1, 0, 1}, //channel 11
  {0, 0, 1, 1}, //channel 12
  {1, 0, 1, 1}, //channel 13
  {0, 1, 1, 1}, //channel 14
  {1, 1, 1, 1} //channel 15
};


void setup()
{
  //first multiplexer
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);

  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);

  //second multiplexer
  pinMode(p0, OUTPUT);
  pinMode(p1, OUTPUT);
  pinMode(p2, OUTPUT);
  pinMode(p3, OUTPUT);

  digitalWrite(p0, LOW);
  digitalWrite(p1, LOW);
  digitalWrite(p2, LOW);
  digitalWrite(p3, LOW);

   Serial.begin(115200);
}

void loop() 
{
  evalPinAll();
  delay(1);
}

void evalPinAll() 
{
  if (Serial) {
    int debugArray[numSensors];
    String values = "";
    for (int i = 0; i < numSensors; i ++) {
      if (i < numHall) {
        debugArray[i] = readHall(sensorChannels[i]);
      }
      else {
        debugArray[i] = readPhoto(sensorChannels[i - numHall]);
      }
      values = values + debugArray[i] + ", ";
    }
    Serial.println(values);
  }
}

//________________READING THE MULTIPLXEXER____________________//
int readHall(int channel) 
{
  int controlPin[] = {s0, s1, s2, s3};

  //loop through the 4 sig
  for (int i = 0; i < 4; i ++) {
    digitalWrite(controlPin[i], muxChannel[channel][i]);
  }

  //read the value at the SIG pin
  int val = analogRead(SIG_pin);

  //return the value
  return val;
}

int readPhoto(int channel) 
{ 
  int controlPin[] = {p0, p1, p2, p3};

  //loop through the 4 sig
  for (int i = 0; i < 4; i ++) {
    digitalWrite(controlPin[i], muxChannel[channel][i]);
  }

  //read the value at the SIG pin
  int val = analogRead(SIG_pin2);

  //return the value
  return val;
}






