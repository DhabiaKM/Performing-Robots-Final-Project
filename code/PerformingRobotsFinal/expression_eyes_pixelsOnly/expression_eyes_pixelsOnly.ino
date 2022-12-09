#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#define FACTORYRESET_ENABLE     1

#define PIN                     A3
#define PIN2                    A4

Adafruit_NeoPixel rightEye(64, PIN2, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel leftEye(64, PIN, NEO_GRB + NEO_KHZ800);

uint8_t LeftEyeNormal[]={8,9,10,11,12,13,14,15,16,23,24,31,32,39,41,46,50,51,52,53};

uint8_t RightEyeNormal[]={8,9,10,11,12,13,14,15,16,23,24,31,32,39,41,46,50,51,52,53};

uint8_t LeftEyeAngry[]={7,14,15,21,22,23,28,29,30,31,35,36,37,38,39,42,43,44,45,46,47,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63};

uint8_t RightEyeAngry[]={0,8,9,16,17,18,24,25,26,27,32,33,34,35,36,40,41,42,43,44,45,48,49,50,51,52,53,54,56,57,58,59,60,61,62,63};

void setup(void)
{ 
  leftEye.clear();
  rightEye.clear(); 

  Serial.begin(9600);
  leftEye.begin(); 
  leftEye.setBrightness(40);
  rightEye.begin(); 
  rightEye.setBrightness(40);
  Serial.println("test");

}

void loop()
{
  angryRightEye(); 
  angryLeftEye();
    


}

void normalLeftEye(){
  for(int i : LeftEyeNormal) { // For each pixel...
    Serial.print("left: turning on ");Serial.println(i);
    // if(i == LeftEyeNormal[i]){ 
    leftEye.setPixelColor(i, leftEye.Color(255, 255, 255));
    // }
  }
  leftEye.show(); 
}

void normalRightEye(){ 
  for(int i : RightEyeNormal) { // For each pixel...
      Serial.print("right: turning on ");Serial.println(i);
      rightEye.setPixelColor(i, rightEye.Color(255, 255, 255));
      
  }
  rightEye.show();
  // test();
}

void angryRightEye(){ 
    for(int i : RightEyeAngry) { // For each pixel...
    Serial.print("right: turning on ");Serial.println(i);
    rightEye.setPixelColor(i, rightEye.Color(255, 0, 0));
  }
  rightEye.show();   
}

void angryLeftEye(){ 
  for(int i : LeftEyeAngry) { // For each pixel...
    Serial.print("left: turning on ");Serial.println(i);
    leftEye.setPixelColor(i, leftEye.Color(255, 0, 0));
  }
  leftEye.show(); 
}