// Remote Control
// Joseph Hong
// Description: Uses the minimal channels code to make a robot that can accel/decelerate
// and steer.

// Using minimal channels code
#include <EnableInterrupt.h>

#define SERIAL_PORT_SPEED 9600
#define RC_NUM_CHANNELS  4

#define RC_CH1  0
#define RC_CH2  1
#define RC_CH3  2
#define RC_CH4  3

#define RC_CH1_PIN  8
#define RC_CH2_PIN  7
#define RC_CH3_PIN  4
#define RC_CH4_PIN  2

// match motor speeds
#define WEIGHT_LEFT 1
#define WEIGHT_RIGHT 1

uint16_t rc_values[RC_NUM_CHANNELS];
uint32_t rc_start[RC_NUM_CHANNELS];
volatile uint16_t rc_shared[RC_NUM_CHANNELS];

// Motor Pins
// left
int pin1 = 3;
int pin2 = 5;
// right
int pin3 = 6;
int pin4 = 9;

void rc_read_values() {
  noInterrupts();
  memcpy(rc_values, (const void *)rc_shared, sizeof(rc_shared));
  interrupts();
}

void calc_input(uint8_t channel, uint8_t input_pin) {
  if (digitalRead(input_pin) == HIGH) {
    rc_start[channel] = micros();
  } else {
    uint16_t rc_compare = (uint16_t)(micros() - rc_start[channel]);
    rc_shared[channel] = rc_compare;
  }
}

void calc_ch1() {
  calc_input(RC_CH1, RC_CH1_PIN);
}
void calc_ch2() {
  calc_input(RC_CH2, RC_CH2_PIN);
}
void calc_ch3() {
  calc_input(RC_CH3, RC_CH3_PIN);
}
void calc_ch4() {
  calc_input(RC_CH4, RC_CH4_PIN);
}

void setup() {
  Serial.begin(SERIAL_PORT_SPEED);

  pinMode(RC_CH1_PIN, INPUT);
  pinMode(RC_CH2_PIN, INPUT);
  pinMode(RC_CH3_PIN, INPUT);
  pinMode(RC_CH4_PIN, INPUT);

  // set up motor input to In1 and In2 of L298 motors
  // motor 1: OUT1&2
  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT); // analog pin
  // motor 2: OUT3&4
  pinMode(pin3, OUTPUT);
  pinMode(pin4, OUTPUT); // analog pin

  enableInterrupt(RC_CH1_PIN, calc_ch1, CHANGE);
  enableInterrupt(RC_CH2_PIN, calc_ch2, CHANGE);
  enableInterrupt(RC_CH3_PIN, calc_ch3, CHANGE);
  enableInterrupt(RC_CH4_PIN, calc_ch4, CHANGE);
}

void loop() {
  rc_read_values();

  // Acceleration
  int speed = int(rc_values[RC_CH2]);
  // int rawSpeed = 1500 - speed;                          // trigger pulled is 960, pushed is 2040
  int conSpeed = map(speed, 960, 2040, 255, -255);
  // Steering
  int turn = int(rc_values[RC_CH1]);
  int rawTurn = 1500 - turn;
  // steering: positive is left, negative is right
  float turnWeight = map(turn, 960, 2040, -50, 50);
  int turnRight = 0;
  int turnLeft = 0;
  if (turn > 1510) {
    turnRight = 1;
  }
  else if (turn < 1490){
    turnLeft = 1;
  }
  
  // move & steer
  if(1490 < speed && speed < 1510){
    analogWrite(pin1, 0); analogWrite(pin2, 0); analogWrite(pin3, 0); analogWrite(pin4, 0);  
  }
  else{
    // motor 1 - left side
    analogWrite(pin1, max((-1)*conSpeed,0) * (100 - abs(turnLeft*turnWeight)) * WEIGHT_LEFT / 100 );
    analogWrite(pin2, max(conSpeed, 0) * (100 - abs(turnLeft*turnWeight)) * WEIGHT_LEFT / 100);
    // motor 2 - right side
    analogWrite(pin3, max((-1)*conSpeed,0) * (100 - abs(turnRight*turnWeight)) * WEIGHT_RIGHT / 100 );
    analogWrite(pin4, max(conSpeed, 0) * (100 - abs(turnRight*turnWeight)) * WEIGHT_RIGHT / 100 ); 
    
    
    // debug
    // analogWrite(pin1, max((-1)*conSpeed,0) );
    // analogWrite(pin2, max(conSpeed, 0) );
    // // motor 2 - right side
    // analogWrite(pin3, max((-1)*conSpeed,0));
    // analogWrite(pin4, max(conSpeed, 0) ); 
    
  }

  // Serial.print("Left Speed: "); Serial.print((-1)*conSpeed * (1 - (turnRight*turnWeight)) * WEIGHT_LEFT); Serial.println();
  // Serial.print("Right Speed: "); Serial.print((-1)*conSpeed * (1 - (turnLeft*turnWeight)) * WEIGHT_RIGHT) ); Serial.print("\t");

  
  // Serial.print(\033[2J);
  
  // Serial.print((turnWeight) * WEIGHT_LEFT );
    Serial.print("      ");
    Serial.print( max(conSpeed, 0) * (100 - (turnLeft*turnWeight)) * WEIGHT_LEFT / 100);
    
    Serial.print("      ");
    // motor 2 - right side
    // Serial.print((turnWeight) * WEIGHT_RIGHT );
    Serial.print("      ");
    Serial.print( max(conSpeed, 0) * (100 - (turnRight*turnWeight)) * WEIGHT_RIGHT / 100); 
    Serial.print("      ");

  Serial.print(turn); Serial.print("   "); Serial.print("   "); Serial.print(turnWeight);
  Serial.println();
  // Serial.print("CH1:"); Serial.print(rc_values[RC_CH1]); Serial.print("\t");
  // Serial.print("CH2:"); Serial.print(rc_values[RC_CH2]); Serial.print("\t");
  // Serial.print("CH3:"); Serial.print(rc_values[RC_CH3]); Serial.print("\t");
  // Serial.print("CH4:"); Serial.println(rc_values[RC_CH4]); Serial.println();

}
