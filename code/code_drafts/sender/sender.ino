/*
   Using the nRF24L01 radio module to communicate
   between two Arduinos with much increased reliability following
   various tutorials, conversations, and studying the nRF24L01 datasheet
   and the library reference.
   This file contains code for both transmitter and receiver. 
   Transmitter at the top, receiver at the bottom. 
   One of them is commented out.
   These sketches require the RF24 library by TMRh20
   Documentation here: https://nrf24.github.io/RF24/index.html
   change log
   02 Dec 2022 - ms - initial entry based on rf24ControlPanelPerformingRobotsNoHandshaking
   10 Dec 2022 - ms - put all strings in flash memory and comment out printf.h to save space
*/

// Common code

// Common pin usage
// Note there are additional pins unique to transmitter or receiver
//

// nRF24L01 uses SPI which is fixed on pins 11, 12, and 13.
// It also requires two other signals
// (CE = Chip Enable, CSN = Chip Select Not)
// Which can be any pins:

const int CEPIN = 9;
const int CSNPIN = 10;

// In summary,
// nRF 24L01 pin    Arduino pin   name
//          1                     GND
//          2                     3.3V
//          3             9       CE
//          4             10      CSN
//          5             13      SCLK
//          6             11      MOSI/COPI
//          7             12      MISO/CIPO

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(CEPIN, CSNPIN);  // CE, CSN

//#include <printf.h>  // for debugging

// See note in rf24Handshaking about address selection
//

// Channel and address allocation:
// Eadin and Dania: Channel 30, addr = 0x76
// Fatima and Shamsa: Channel 40, addr = 0x73
// Oliver and Hessa:  Channel 50, addr = 0x7C
// Louis and Alpha: Channel 60, addr = 0xC6
// Yoki and Yupu:  Channel 70, addr = 0xC3
// Omar and Mudi: Channel 80, addr = 0xCC
// Dhabia and Joseph: Channel 90, addr = 0x33
const byte addr = 0x33;             // change as per the above assignment
const int RF24_CHANNEL_NUMBER = 90;  // change as per the above assignment

// Do not make changes here
const byte xmtrAddress[] = { addr, addr, 0xC7, 0xE6, 0xCC };
const byte rcvrAddress[] = { addr, addr, 0xC7, 0xE6, 0x66 };

const int RF24_POWER_LEVEL = RF24_PA_LOW;

// global variables
uint8_t pipeNum;
unsigned int totalTransmitFailures = 0;

struct DataStruct {
  uint8_t servoBits;
  uint8_t neoPixelBits;
  uint8_t armBits;
};
DataStruct data;
DataStruct currentData; 
DataStruct prevData; 


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

void rf24SendData() {

  // The write() function will block
  // until the message is successfully acknowledged by the receiver
  // or the timeout/retransmit maxima are reached.
  int retval = radio.write(&data, sizeof(data));

  Serial.print(F("Servo bits = "));
  Serial.print(data.servoBits, BIN);
  Serial.print(F(" NeoPixel bits = "));
  Serial.print(data.neoPixelBits, BIN);
  Serial.print(F(" Arm Bits = "));
  Serial.print(data.armBits, BIN);

  Serial.print(" ... ");
  if (retval) {
    Serial.println("success");
  } else {
    totalTransmitFailures++;
    Serial.print(F("failure, total failures = "));
    Serial.println(totalTransmitFailures);
  }
}


// Transmitter code
// Switches for selecting servo activity
const int SERVOSELPIN2 = 5;
const int SERVOSELPIN1 = 4;
const int SERVOSELPIN0 = 3;
const int SERVOXMITPIN = 2;
// Switches for selecting NeoPixel activity
const int NEOSELPIN2 = A2;
const int NEOSELPIN1 = A1;
const int NEOSELPIN0 = A0;
const int NEOXMITPIN = 6;
// Hypothetical audio control switches
const int PLAYNEXTCLIPPIN = A3;
const int PLAYPREVCLIPIN = A4;
// that leaves 6, 7, 8, A4, and A5 unused
void setup() {
  Serial.begin(9600);
  //printf_begin();
  // All switches use internal pullup resistor
  pinMode(SERVOSELPIN2, INPUT_PULLUP);
  pinMode(SERVOSELPIN1, INPUT_PULLUP);
  pinMode(SERVOSELPIN0, INPUT_PULLUP);
  pinMode(SERVOXMITPIN, INPUT_PULLUP);
  pinMode(NEOSELPIN2, INPUT_PULLUP);
  pinMode(NEOSELPIN1, INPUT_PULLUP);
  pinMode(NEOSELPIN0, INPUT_PULLUP);
  pinMode(NEOXMITPIN, INPUT_PULLUP);
  pinMode(PLAYNEXTCLIPPIN, INPUT_PULLUP);
  pinMode(PLAYPREVCLIPIN, INPUT_PULLUP);

  setupRF24();
}
void setupRF24() {
  setupRF24Common();
  // Set us as a transmitter
  radio.openWritingPipe(xmtrAddress);
  radio.openReadingPipe(1, rcvrAddress);
  // radio.printPrettyDetails();
  Serial.println(F("I am a transmitter"));
}

int fireUp = 0;

void loop() {

  // If the transmit button is pressed, read the switches
  // and send the bits
  
  if (digitalRead(SERVOXMITPIN) == LOW) {  // remember switches are active LOW
    // clearData();
    // shifting bits
    data.servoBits = (digitalRead(SERVOSELPIN0) << 0
                      | digitalRead(SERVOSELPIN1) << 1
                      | digitalRead(SERVOSELPIN2) << 2);
    if(digitalRead(SERVOSELPIN2)){
      fireUp = 4;
    }
    else{
      fireUp = 0;
    }
    radio.stopListening();
    // if(data.servoBits!=prevData.servoBits){ 
      prevData=data; 
      rf24SendData();
    // }
    delay(100);  // if the button is still pressed don't do this too often
  }
  if (digitalRead(NEOXMITPIN) == LOW) {  // remember switches are active LOW
    clearData();
    data.neoPixelBits = (digitalRead(NEOSELPIN0) << 0
                         | digitalRead(NEOSELPIN1) << 1
                         | digitalRead(NEOSELPIN2) << 2);
    radio.stopListening();
    // if(data.neoPixelBits!=prevData.neoPixelBits){ 
      prevData=data; 
    rf24SendData();
    // }
    delay(100);  // if the button is still pressed don't do this too often
  }

  // arms
  int rightUp = 0;
  int leftUp = 0;
  if (digitalRead(PLAYNEXTCLIPPIN) == LOW) {
    leftUp = 1;
  }
  if (digitalRead(PLAYPREVCLIPIN) == LOW){
    rightUp = 2;
  }
  if(leftUp > 0 || rightUp > 0){
    clearData();
    data.armBits = leftUp + rightUp;
    radio.stopListening();
    rf24SendData();
    delay(100);  // if the button is still pressed don't do this too often
  }

}  // end of loop()
void clearData() {
  // set all fields to 0
  data.servoBits = 0 + fireUp;
  data.neoPixelBits = 0;
  data.armBits = 0;
}
// End of transmitter code

