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
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>

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

// In summary,
// nRF 24L01 pin    Arduino pin   name
//          1                     GND
//          2                     3.3V
//          3             9       CE
//          4             10      CSN
//          5             13      SCLK
//          6             11      MOSI/COPI
//          7             12      MISO/CIPO

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
#define NEOLEFT A4
#define NEORIGHT A3
#define NEOPIXELPIN3 A5
#define BRIGHTNESS 30


// define eyes and mouth
Adafruit_NeoMatrix eyeLeft = Adafruit_NeoMatrix(8, 8, NEOLEFT,  NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
  NEO_MATRIX_COLUMNS,
  NEO_GRB            + NEO_KHZ800);
Adafruit_NeoMatrix eyeRight = Adafruit_NeoMatrix(8, 8, NEORIGHT,  NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
  NEO_MATRIX_COLUMNS,
  NEO_GRB            + NEO_KHZ800);
Adafruit_NeoMatrix mouthLED = Adafruit_NeoMatrix(8, 8, NEOPIXELPIN3,  NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
  NEO_MATRIX_COLUMNS,
  NEO_GRB            + NEO_KHZ800);
// define expression var. 0 - default, 1 - angry
int expressVal = 0;

// Define Servos
Servo armLeft;
Servo armRight;

// change as per your robot
const int ARMLEFTNEUTRAL = 150;
const int ARMRIGHTNEUTRAL = 20;

//=======================================================================================
// Setup Functions

void setup() {
  Serial.begin(9600);
  Serial.print("got all the way here");
  printf_begin();
  Serial.print("got all the way here");
  // setupMusicMakerShield();
  setupServoMotors();
  setupLights();
  setupRF24();
  // Set up all the attached hardware
  
  
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
  armLeft.write(ARMLEFTNEUTRAL);
  delay(10);
  armRight.write(ARMRIGHTNEUTRAL);
  delay(10);
  Serial.println("Arms set up.");
}

void setupNeoMatrix(Adafruit_NeoMatrix &matrix){
  // Serial.print("Setting up matrix"); Serial.println(String(matrix);  
  matrix.begin();
  matrix.setBrightness(BRIGHTNESS);
  matrix.fillScreen(0);
  matrix.show(); 
}

void setupLights() {
  Serial.println("Setting up lights...");
  setupNeoMatrix(eyeLeft);
  setupNeoMatrix(eyeRight);
  setupNeoMatrix(mouthLED);
  displayEyes(expressVal);
  // need code for mouth
  Serial.println("Lights set up.");
}

//====================================================================
// Other Functions

// Display expression. Default val = 0, Angry = 1. (val + 1)%2 on button.
void displayEyes(int expression){
  // if 0, display normal
  if(expression == 0){
    Serial.println("Normal");
    leftEyeNormal(eyeLeft, eyeLeft.Color(255,255,255));
    rightEyeNormal(eyeRight, eyeRight.Color(255,255,255));
  }
  // else display angry
  else{
    Serial.print("Angry");
    leftEyeAngry(eyeLeft, eyeLeft.Color(255,0,0) );
    rightEyeAngry(eyeRight, eyeRight.Color(255,0,0) );
  }
}

// Left Eye
void leftEyeAngry(Adafruit_NeoMatrix &matrix, uint32_t c){
  matrix.fillScreen(0);
  matrix.drawLine(1,0,4,0,c); 
  matrix.drawLine(5,1,6,2,c); 
  matrix.drawLine(6,3,6,5,c); 
  matrix.drawLine(0,0,6,6,c);
  matrix.show();
}
void leftEyeNormal(Adafruit_NeoMatrix &matrix, uint32_t c){ 
  matrix.fillScreen(0);
  matrix.drawLine(1,0,1,7,c); 
  matrix.drawLine(2,0,4,0,c); 
  matrix.drawLine(5,1,6,2,c); 
  matrix.drawLine(6,3,6,5,c); 
  matrix.drawLine(2,7,4,7,c);
  matrix.drawPixel(5, 6, c);
  matrix.show();
}

// Right Eye
void rightEyeAngry(Adafruit_NeoMatrix &matrix, uint32_t c){
  matrix.fillScreen(0);
  matrix.drawLine(6,1,0,7,c); 
  matrix.drawLine(6,2,6,5,c); 
  matrix.drawLine(1,7,4,7,c);
  matrix.drawPixel(5, 6, c);
  matrix.show();
}
void rightEyeNormal(Adafruit_NeoMatrix &matrix, uint32_t c){
  matrix.fillScreen(0);
  matrix.drawLine(1,0,1,7,c); 
  matrix.drawLine(2,0,4,0,c); 
  matrix.drawLine(5,1,6,2,c); 
  matrix.drawLine(6,3,6,5,c); 
  matrix.drawLine(2,7,4,7,c);
  matrix.drawPixel(5, 6, c);
  matrix.show();
}




//=======================================================================================
// Main
// const int ARMLEFTNEUTRAL = 150;
// const int ARMRIGHTNEUTRAL = 60;
int armLeftAngle = ARMLEFTNEUTRAL;
int armRightAngle = ARMRIGHTNEUTRAL;

bool armLeftMoving = false;
bool armRightMoving = false;

unsigned long armUpMillis = 0;
unsigned long armDownMillis = 0;

bool buttonPressed = false;


void loop() {
  unsigned long currentMillis = millis();
  // If there is data, read it,
  // and do the needfull
  // Become a receiver
  radio.startListening();
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
          armLeftAngle -= 50;
          armLeft.write(armLeftAngle);     
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
          armRightAngle += 50;
          armRight.write(armRightAngle);
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
        if(armRight.read() == armRightAngle){
            armRightMoving = false;
        }
        if(armLeft.read() == armLeftAngle){
              armLeftMoving = false;
        }
      // }
      // move arms down if not actively moving up
      if(!armLeftMoving && (armLeftAngle < (ARMLEFTNEUTRAL)) ){
        // armLeft.write(ARMLEFTNEUTRAL);
        // armLeftAngle = ARMLEFTNEUTRAL;
        armLeftAngle += 5;
        armLeft.write(armLeftAngle);
        // for(armLeftAngle; armLeftAngle < ARMLEFTNEUTRAL; armLeftAngle+=5){
        //   armLeft.write(armLeftAngle);
        //   delay(5);
        // }
      }
      if(!armRightMoving && (armRightAngle > (ARMRIGHTNEUTRAL)) ){
        // armRight.write(ARMRIGHTNEUTRAL);
        // armRightAngle = ARMRIGHTNEUTRAL;
        armRightAngle -= 5;
        armRight.write(armRightAngle);
      }
      buttonPressed = false;

      armDownMillis = currentMillis;
    }
    // check if arms are moving
      

    
    // equalizer - in case numbers go out of range
    if(armRightAngle < ARMRIGHTNEUTRAL){
      armRightAngle = ARMRIGHTNEUTRAL;
    }
    else if (armRightAngle > 180){
      armRightAngle = 180;
    }
    if(armLeftAngle < 0){
      armLeftAngle = 0;
    }
    else if (armLeftAngle > ARMLEFTNEUTRAL){
      armLeftAngle = ARMLEFTNEUTRAL;
    }

    
  
    // debugging
    Serial.print("armLeftAngle: "); Serial.print(armLeftAngle);Serial.print("  armRightAngle: "); Serial.println(armRightAngle);
  }
  // displayEyes(expressVal);

  
}  // end of loop()
// end of receiver code
