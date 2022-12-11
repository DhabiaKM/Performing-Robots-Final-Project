// Receiver v2.0
// Modified by Joseph Hong
// Description: modified from the original code to control Anger robot for IM Showcase.
//=======================================================================================
//=======================================================================================
// Libraries

// For Servos
#include <Servo.h>
// For Facial Expressions
#include <Adafruit_NeoPixel.h>

#include <string.h>

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
// #include <printf.h>  // for debugging

//=======================================================================================
//=======================================================================================
// Global Variables & Definitions

const int CEPIN = 9;
const int CSNPIN = 10;
RF24 radio(CEPIN, CSNPIN);  // CE, CSN

// Channel and address allocation:
const byte addr = 0x33; // change as per the above assignment
const int RF24_CHANNEL_NUMBER = 90; // change as per the above assignment
const byte xmtrAddress[] = { addr, addr, 0xC7, 0xE6, 0xCC };
const byte rcvrAddress[] = { addr, addr, 0xC7, 0xE6, 0x66 };
const int RF24_POWER_LEVEL = RF24_PA_LOW;

// Pipe Variables
uint8_t pipeNum;
unsigned int totalTransmitFailures = 0;

// Additional pin usage for receiver
// Adafruit music maker shield
#define SHIELD_RESET -1  // VS1053 reset pin (unused!)
#define SHIELD_CS 7      // VS1053 chip select pin (output)
#define SHIELD_DCS 6     // VS1053 Data/command select pin (output)
#define CARDCS 4         // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3  // VS1053 Data request, ideally an _RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);
// Servo motors
const int SERVO0PIN = A1;
const int SERVO1PIN = A2;

// Neopixels
#define FACTORYRESET_ENABLE     1
#define NEOLEFT A3
#define NEORIGHT A4
#define NEOPIXELPIN3 A5
#define BRIGHTNESS 30

// define eyes and mouth
Adafruit_NeoPixel eyeRight(64, NEORIGHT, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel eyeLeft(64, NEOLEFT, NEO_GRB + NEO_KHZ800);



// using individual pixels to save space
// normal eye expressions
uint8_t LeftEyeNormal[]={8,9,10,11,12,13,14,15,16,23,24,31,32,39,41,46,50,51,52,53};
uint8_t RightEyeNormal[]={8,9,10,11,12,13,14,15,16,23,24,31,32,39,41,46,50,51,52,53};
// angry eye expressions
uint8_t LeftEyeAngry[]={7,14,15,21,22,23,28,29,30,31,35,36,37,38,39,42,43,44,45,46,47,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63};
uint8_t RightEyeAngry[]={0,8,9,16,17,18,24,25,26,27,32,33,34,35,36,40,41,42,43,44,45,48,49,50,51,52,53,54,56,57,58,59,60,61,62,63};

// define expression var. 0 - default, 1 - angry
int expressVal = 0;

// Define Servos
Servo armLeft;
Servo armRight;

// change as per your robot
const int ARMNEUTRALLEFT = 150;
const int ARMNEUTRALRIGHT = 20;
const int ARMMAXLEFT = 0;
const int ARMMAXRIGHT = 180;

// structure for data
struct DataStruct {
  uint8_t servoBits;
  uint8_t neoPixelBits;
  uint8_t armBits;
};
DataStruct data;

//=======================================================================================
// Setup Functions

void setup() {
  Serial.begin(9600);
  setupServoMotors();
  setupLights();
  setupRF24();  
}

void setupRF24Common() {
  // RF24 setup
  if (!radio.begin()) {
    Serial.println("radio  initialization failed");
    while (1)
      ;
  } else {
    Serial.println("radio successfully initialized");
  }

  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(RF24_CHANNEL_NUMBER);
  radio.setPALevel(RF24_POWER_LEVEL);
}

// RF24
void setupRF24() {
  setupRF24Common();
  // Set us as a receiver
  radio.openWritingPipe(rcvrAddress);
  radio.openReadingPipe(1, xmtrAddress);
  // radio.printPrettyDetails();
  // Serial.println("I am a receiver");
}

// Servos
void setupServoMotors() {
  armLeft.attach(SERVO0PIN);
  armRight.attach(SERVO1PIN);
  // Serial.println("Arms attached.");
  armLeft.write(ARMNEUTRALLEFT);
  delay(10);
  armRight.write(ARMNEUTRALRIGHT);
  delay(10);
  // Serial.println("Arms set up.");
}

void setupNeoPixel(Adafruit_NeoPixel &pixel){
  // Serial.print("Setting up pixel"); Serial.println(String(pixel);  
  pixel.begin();
  pixel.setBrightness(BRIGHTNESS);
  pixel.clear();
  pixel.show();
}

void setupLights() {
  setupNeoPixel(eyeRight);
  // Serial.println("Setting up lights...");
  setupNeoPixel(eyeLeft);
  // setupNeoPixel(mouthLED);
  displayEyes(expressVal);
  // need code for mouth
  // Serial.println("Lights set up.");
}

//====================================================================
// Other Functions

// EXPRESSIONS
// Display expression. Default val = 0, Angry = 1.
void displayEyes(int expression){
  // if 0, display normal
  if(!expression){
    // Serial.println("Normal");
    eyeNormalLeft(eyeLeft);
    eyeNormalRight(eyeRight);
  }
  // else display angry
  else{
    // Serial.print("Angry");
    eyeAngryLeft(eyeLeft);
    eyeAngryRight(eyeRight);
  }
}
// Angry Expression
// Left Eye
void eyeAngryLeft(Adafruit_NeoPixel &pixel){
  pixel.clear();
  for(int i : LeftEyeAngry) { // For each pixel...
    // Serial.print("left: turning on ");Serial.println(i);
    pixel.setPixelColor(i, pixel.Color(255, 0, 0));
  }
  pixel.show(); 
}
// Right Eye
void eyeAngryRight(Adafruit_NeoPixel &pixel){
  pixel.clear();
  for(int i : RightEyeAngry) { // For each pixel...
    // Serial.print("right: turning on "); Serial.println(i);
    pixel.setPixelColor(i, pixel.Color(255, 0, 0));
  }
  pixel.show();
}
// Normal Expression
// Left Eye
void eyeNormalLeft(Adafruit_NeoPixel &pixel){ 
  pixel.clear();
  for(int i : LeftEyeNormal) { // For each pixel...
    // Serial.print("left: turning on ");Serial.println(i);
    pixel.setPixelColor(i, pixel.Color(255, 255, 255));
  }
  pixel.show();
}
// Right Eye
void eyeNormalRight(Adafruit_NeoPixel &pixel){
  pixel.clear();
  for(int i : RightEyeNormal) { // For each pixel...
      // Serial.print("right: turning on ");Serial.println(i);
      pixel.setPixelColor(i, pixel.Color(255, 255, 255));
    pixel.show();
  }
}

// Macros
void armsUp(int angleLeft, int angleRight, Servo armLeft, Servo armRight){
  armLeft.write(angleLeft);
  armRight.write(angleRight);
}
// reset angle of arms
void resetArms(int armAngleLeft, int armAngleRight){
  for(armAngleLeft; armAngleLeft <= ARMNEUTRALLEFT; armAngleLeft+=5){
    armLeft.write(armAngleLeft);
    delay(5);
  }
  for(armAngleRight; armAngleRight >= ARMNEUTRALRIGHT; armAngleLeft+=5){
    armLeft.write(armAngleLeft);
    delay(5);
  }
}



//=======================================================================================
// Main
// const int ARMNEUTRALLEFT = 150;
// const int ARMNEUTRALRIGHT = 60;
// ARMMAXLEFT = 0;
// ARMMAXRIGHT = 180;
int armAngleLeft = ARMNEUTRALLEFT;
int armAngleRight = ARMNEUTRALRIGHT;

bool armLeftMoving = false;
bool armRightMoving = false;

unsigned long armUpMillis = 0;
unsigned long armDownMillis = 0;

bool buttonPressed = false;
bool armsUnlocked = true;


void loop() {
  unsigned long currentMillis = millis();
  radio.startListening();
  // displayEyes(expressVal);
  if (radio.available(&pipeNum)) {
    radio.read(&data, sizeof(data));
    // Serial.print("message received Data = ");
    // Serial.println(data.selectorBits);
    switch (data.neoPixelBits) {
      case 0b00000001:
        displayEyes(0);
        break;
      case 0b00000011:
        displayEyes(1);
        break;
      default:
        break;
    }
    switch (data.servoBits) {
      case 0b00000000:
        // armsUnlocked = false;
        break;
      case 0b00000001:
        armsUnlocked = false;
        armLeft.write(ARMMAXLEFT);
        armAngleLeft = ARMMAXLEFT;
        break;
      case 0b00000010:
        armsUnlocked = false;
        armRight.write(ARMMAXRIGHT);
        armAngleRight = ARMMAXRIGHT;
        break;
      case 0b00000011:
        armsUnlocked = false;
        armLeft.write(ARMMAXLEFT);
        armRight.write(ARMMAXRIGHT);
        armAngleLeft = ARMMAXLEFT;
        armAngleRight = ARMMAXRIGHT;
        break;
      default:
        break;
    }

    switch (data.armBits) {
      case 0b00000000:
        break;
      case 0b00000001:
        armsUnlocked = true;
        if(!armLeftMoving){
          armLeftMoving = true;
          armAngleLeft -= 50;
          armLeft.write(armAngleLeft);     
          delay(5);
          buttonPressed = true;
        }
        break;
      case 0b00000010:
        armsUnlocked = true;
        if(!armRightMoving){
          armRightMoving = true;
          armAngleRight += 50;
          armRight.write(armAngleRight);
          delay(5);
          buttonPressed = true;
        }
        break;
      case 0b00000011:
        armsUnlocked = true;
        if(!armLeftMoving){
          armLeftMoving = true;
          armAngleLeft -= 50;
          armLeft.write(armAngleLeft);     
          delay(5);
          buttonPressed = true;
        }
        if(!armRightMoving){
          armRightMoving = true;
          armAngleRight += 50;
          armRight.write(armAngleRight);
          delay(5);
          buttonPressed = true;
        }
        // expressVal += 1;
        // if(expressVal%2){
        //   expressVal = 1;
        // }
        // else{
        //   expressVal = 0;
        // }
        // displayEyes(expressVal);
        break;
      case 0b00000100:
        break;
    }
  }
  else{
    // if arm not moving, bring it down
    // left arm 150
    // right arm 10

    if(armsUnlocked){
      if(currentMillis - armDownMillis >= 100){
        // move arms down if not actively moving up
        if(!armLeftMoving && (armAngleLeft < (ARMNEUTRALLEFT)) ){
          // armLeft.write(ARMNEUTRALLEFT);
          // armAngleLeft = ARMNEUTRALLEFT;
          armAngleLeft += 10;
          armLeft.write(armAngleLeft);
          // for(armAngleLeft; armAngleLeft < ARMNEUTRALLEFT; armAngleLeft+=5){
          //   armLeft.write(armAngleLeft);
          //   delay(5);
          // }
        }
        if(!armRightMoving && (armAngleRight > (ARMNEUTRALRIGHT)) ){
          // armRight.write(ARMNEUTRALRIGHT);
          // armAngleRight = ARMNEUTRALRIGHT;
          armAngleRight -= 10;
          armRight.write(armAngleRight);
        }
        buttonPressed = false;
        armDownMillis = currentMillis;
      }
    }

    // check if arms are moving
    if(armRight.read() == armAngleRight){
      armRightMoving = false;
    }
    if(armLeft.read() == armAngleLeft){
      armLeftMoving = false;
    }

    // equalizer - in case numbers go out of range
    if(armAngleRight < ARMNEUTRALRIGHT){
      armAngleRight = ARMNEUTRALRIGHT;
    }
    else if (armAngleRight > 180){
      armAngleRight = 180;
    }
    if(armAngleLeft < 0){
      armAngleLeft = 0;
    }
    else if (armAngleLeft > ARMNEUTRALLEFT){
      armAngleLeft = ARMNEUTRALLEFT;
    }
  
    // debugging
    // Serial.print("armAngleLeft: "); Serial.print(armAngleLeft);Serial.print("  armAngleRight: "); Serial.println(armAngleRight);
  }
  // displayEyes(expressVal);

  
}  // end of loop()
// end of receiver code
