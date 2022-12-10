// Receiver v2.0
// Modified by Joseph Hong
// Description: modified from the original code to control Anger robot for IM Showcase.
//=======================================================================================
//=======================================================================================
// Libraries

// For MusicMaker
#include <Adafruit_VS1053.h>
#include <SD.h>
// For Servos
#include <Servo.h>
// For Facial Expressions
#include <Adafruit_NeoPixel.h>

#include <string.h>

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <printf.h>  // for debugging

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
struct DataStruct {
  uint8_t selectorBits;
};
DataStruct data;

// Additional pin usage for receiver
// Adafruit music maker shield
#define SHIELD_RESET -1  // VS1053 reset pin (unused!)
#define SHIELD_CS 7      // VS1053 chip select pin (output)
#define SHIELD_DCS 6     // VS1053 Data/command select pin (output)
#define CARDCS 4         // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3  // VS1053 Data request, ideally an Interrupt pin
Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);
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
Adafruit_NeoPixel eyeLeft(64, NEOLEFT, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel eyeRight(64, NEORIGHT, NEO_GRB + NEO_KHZ800);


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

//=======================================================================================
// Setup Functions

void setup() {
  Serial.begin(9600);
  printf_begin();
  // setupMusicMakerShield();
  setupServoMotors();
  setupLights();
  setupRF24();
  // Set up all the attached hardware
  // eyeRight.clear();
  
  
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
  radio.printPrettyDetails();
  Serial.println("I am a receiver");
}

// Music Maker
void setupMusicMakerShield() {
  if (!musicPlayer.begin()) {  // initialise the music player
    Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
    while (1)
      ;
  }
  Serial.println(F("VS1053 found"));
  if (!SD.begin(CARDCS)) {
    Serial.println(F("SD card failed or not present"));
    while (1)
      ;  // don't do anything more
  }

  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(20, 20);

  // Timer interrupts are not suggested, better to use DREQ interrupt!
  //musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT); // timer int

  // If DREQ is on an interrupt pin (on uno, #2 or #3) we can do background
  // audio playing
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int
}

void setupServoMotors() {
  armLeft.attach(SERVO0PIN);
  armRight.attach(SERVO1PIN);
  // Serial.println("Arms attached.");
  armLeft.write(ARMNEUTRALLEFT);
  delay(10);
  armRight.write(ARMNEUTRALRIGHT);
  delay(10);
  Serial.println("Arms set up.");
}

void setupNeoPixel(Adafruit_NeoPixel &pixel){
  // Serial.print("Setting up pixel"); Serial.println(String(pixel);  
  pixel.begin();
  pixel.setBrightness(BRIGHTNESS);
}

void setupLights() {
  Serial.println("Setting up lights...");
  setupNeoPixel(eyeLeft);
  setupNeoPixel(eyeRight);
  // setupNeoPixel(mouthLED);
  displayEyes(expressVal);
  // need code for mouth
  Serial.println("Lights set up.");
}

//====================================================================
// Other Functions

// Display expression. Default val = 0, Angry = 1.
void displayEyes(int expression){
  // if 0, display normal
  if(expression == 0){
    Serial.println("Normal");
    eyeNormalLeft(eyeLeft);
    eyeNormalRight(eyeRight);
  }
  // else display angry
  else{
    Serial.print("Angry");
    eyeAngryLeft(eyeLeft);
    eyeAngryRight(eyeRight);
  }
}
// Angry Expression
// Left Eye
void eyeAngryLeft(Adafruit_NeoPixel pixel){
  for(int i : LeftEyeAngry) { // For each pixel...
    // Serial.print("left: turning on ");Serial.println(i);
    pixel.setPixelColor(i, pixel.Color(255, 0, 0));
  }
  pixel.show(); 
}
// Right Eye
void eyeAngryRight(Adafruit_NeoPixel pixel){
  for(int i : RightEyeAngry) { // For each pixel...
    // Serial.print("right: turning on "); Serial.println(i);
    pixel.setPixelColor(i, pixel.Color(255, 0, 0));
  }
  pixel.show();
}
// Normal Expression
// Left Eye
void eyeNormalLeft(Adafruit_NeoPixel pixel){ 
  for(int i : LeftEyeNormal) { // For each pixel...
    // Serial.print("left: turning on ");Serial.println(i);
    pixel.setPixelColor(i, pixel.Color(255, 255, 255));
  }
  pixel.show();
}
// Right Eye
void eyeNormalRight(Adafruit_NeoPixel pixel){
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
int armAngleLeft = ARMNEUTRALLEFT;
int armAngleRight = ARMNEUTRALRIGHT;

bool armLeftMoving = false;
bool armRightMoving = false;

unsigned long armUpMillis = 0;
unsigned long armDownMillis = 0;

bool buttonPressed = false;


void loop() {
  unsigned long currentMillis = millis();
  radio.startListening();
  // displayEyes(expressVal);
  if (radio.available(&pipeNum)) {
    radio.read(&data, sizeof(data));
    // Serial.print("message received Data = ");
    // Serial.println(data.selectorBits);

    switch (data.selectorBits) {
      case 0b00000000:
        
        break;
      case 0b00000001:
        Serial.println("Moving Left Arm");
        // Don't play if it's already playing
        // if (musicPlayer.stopped()) {
        //   // Non-blocking
        //   Serial.println(F("Playing track 001"));
        //   musicPlayer.startPlayingFile("/track001.mp3");
        // } else {
        //   Serial.println(F("Playing in progress, ignoring"));
        // }
        if(!armLeftMoving){
          armLeftMoving = true;
          armAngleLeft -= 50;
          armLeft.write(armAngleLeft);     
          delay(5);
          buttonPressed = true;
        }
        
        
        break;
      case 0b00000010:
        Serial.println("Moving Right Arm");
        // Don't play if it's already playing
        // if (musicPlayer.stopped()) {
        //   // Non-blocking
        //   Serial.println(F("Playing track 002"));
        //   musicPlayer.startPlayingFile("/track002.mp3");
        // } else {
        //   Serial.println(F("Playing in progress, ignoring"));
        // }

        if(!armRightMoving){
          armRightMoving = true;
          armAngleRight += 50;
          armRight.write(armAngleRight);
          delay(5);
          buttonPressed = true;
        }
        
        
        break;
      case 0b00000011:
        expressVal = (expressVal + 1)%2;
        Serial.println("Changing Expression" + String(expressVal));
        displayEyes(expressVal);
        break;
      case 0b00000100:
        break;
      case 0b00000101:
        break;
      case 0b00000110:
        break;
      case 0b00000111:
        break;
      case 0b00001000:
        break;
      case 0b00001001:
        break;
      case 0b00001010:
        break;
      case 0b00001011:
        break;
      case 0b00001100:
        break;
      case 0b00001101:
        break;
      case 0b00001110:
        break;
      case 0b00001111:
        break;
      default:
        break;
    }
  }
  else{
    // if arm not moving, bring it down
    // left arm 150
    // right arm 10

    if(currentMillis - armDownMillis >= 100){
      // if(!buttonPressed){
        if(armRight.read() == armAngleRight){
            armRightMoving = false;
        }
        if(armLeft.read() == armAngleLeft){
              armLeftMoving = false;
        }
      // }
      // move arms down if not actively moving up
      if(!armLeftMoving && (armAngleLeft < (ARMNEUTRALLEFT)) ){
        // armLeft.write(ARMNEUTRALLEFT);
        // armAngleLeft = ARMNEUTRALLEFT;
        armAngleLeft += 5;
        armLeft.write(armAngleLeft);
        // for(armAngleLeft; armAngleLeft < ARMNEUTRALLEFT; armAngleLeft+=5){
        //   armLeft.write(armAngleLeft);
        //   delay(5);
        // }
      }
      if(!armRightMoving && (armAngleRight > (ARMNEUTRALRIGHT)) ){
        // armRight.write(ARMNEUTRALRIGHT);
        // armAngleRight = ARMNEUTRALRIGHT;
        armAngleRight -= 5;
        armRight.write(armAngleRight);
      }
      buttonPressed = false;

      armDownMillis = currentMillis;
    }
    // check if arms are moving
      

    
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
