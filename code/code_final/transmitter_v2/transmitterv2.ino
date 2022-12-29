// Common code

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

#include <printf.h>  // for debugging

// See note in rf24Handshaking about address selection
//

// Channel and address allocation:
// Dhabia and Joseph: Channel 90, addr = 0x33
const byte addr = 0x33; // change as per the above assignment
const int RF24_CHANNEL_NUMBER = 90; // change as per the above assignment

// Do not make changes here
const byte xmtrAddress[] = { addr, addr, 0xC7, 0xE6, 0xCC };
const byte rcvrAddress[] = { addr, addr, 0xC7, 0xE6, 0x66 };

const int RF24_POWER_LEVEL = RF24_PA_LOW;

// global variables
uint8_t pipeNum;
unsigned int totalTransmitFailures = 0;

struct DataStruct {
  uint8_t selectorBits;
};
DataStruct data;

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

  Serial.print("Sending data = ");
  Serial.print(data.selectorBits);
  Serial.print(" ... ");
  if (retval) {
    Serial.println("success");

  } else {
    totalTransmitFailures++;
    Serial.print("failure, total failures = ");
    Serial.println(totalTransmitFailures);
  }
}


// Transmitter code

//Buttons + Button State
const int blueButtonPin = 2;  // the number of the blue pushbutton pin
const int redButtonPin = 4;  // the number of the pushbutton pin
const int greenButtonPin = 7;  // the number of the green pushbutton pin

// variables will change:
int redButtonState = 0;  // variable for reading the pushbutton status
int blueButtonState = 0;  // variable for reading the pushbutton status
int greenButtonState = 0;  // variable for reading the pushbutton status

// Additional pin usage for transmitter
// Example of selector switches line 1
// const int SELECTOR0PIN = 6;
// const int SELECTOR1PIN = 5;
// const int SELECTOR2PIN = 4;
// const int SELECTOR3PIN = 3;
// const int XMIT1PIN = 2;

// Hypothetical second row of selector switches
// const int SELECTOR4PIN = A0;
// const int SELECTOR5PIN = A1;
// const int SELECTOR6PIN = A2;
// const int SELECTOR7PIN = A3;
// const int XMIT2PIN = 7;

// Hypothetical audio control switches
// const int PLAYNEXTCLIPPIN = A4;
// const int PLAYPREVIOUSCLIPPIN = A5;
// const int SPAREPIN = 8;

void setup() {
  Serial.begin(9600);
  printf_begin();

  //Buttons
  pinMode(redButtonPin, INPUT);
  pinMode(blueButtonPin, INPUT);
  pinMode(greenButtonPin, INPUT);

  Serial.begin(9600);

  // All switches use internal pullup resistor
  // pinMode(SELECTOR0PIN, INPUT_PULLUP);
  // pinMode(SELECTOR1PIN, INPUT_PULLUP);
  // pinMode(SELECTOR2PIN, INPUT_PULLUP);
  // pinMode(SELECTOR3PIN, INPUT_PULLUP);
  // pinMode(SELECTOR4PIN, INPUT_PULLUP);
  // pinMode(SELECTOR5PIN, INPUT_PULLUP);
  // pinMode(SELECTOR6PIN, INPUT_PULLUP);
  // pinMode(SELECTOR7PIN, INPUT_PULLUP);

  // pinMode(XMIT1PIN, INPUT_PULLUP);
  // pinMode(XMIT2PIN, INPUT_PULLUP);

  // pinMode(PLAYNEXTCLIPPIN , INPUT_PULLUP);
  // pinMode(PLAYPREVIOUSCLIPPIN , INPUT_PULLUP);
  // pinMode(SPAREPIN , INPUT_PULLUP);

  setupRF24();
}

void setupRF24() {

  setupRF24Common();

  // Set us as a transmitter
  radio.openWritingPipe(xmtrAddress);
  radio.openReadingPipe(1, rcvrAddress);

  radio.printPrettyDetails();
  Serial.println("I am a transmitter");
}

void loop() {
    redButtonState = digitalRead(redButtonPin);
    blueButtonState = digitalRead(blueButtonPin);
    greenButtonState = digitalRead(greenButtonPin);
  // If the transmit button is pressed, read the switches
  // and send the bits

  // if (digitalRead(XMIT1PIN) == LOW) {  // remember switches are active LOW
  //   data.selectorBits = (digitalRead(SELECTOR0PIN) << 0 
  //   | digitalRead(SELECTOR1PIN) << 1 
  //   | digitalRead(SELECTOR2PIN) << 2
  //   | digitalRead(SELECTOR3PIN) << 3 );
        if (blueButtonState==HIGH){
          data.selectorBits = 1;
          }
        else if(redButtonState==HIGH){ 
          data.selectorBits = 2;
        }
        else if(greenButtonState==HIGH){ 
          data.selectorBits = 3;
        }
        else{ 
          data.selectorBits=0;
        }

    Serial.print("XMTR: sending data = ");
    Serial.println(data.selectorBits);

    
    radio.stopListening();
    rf24SendData();

    delay(100);  // if the button is still pressed don't do this too often
  }
  // end of loop()