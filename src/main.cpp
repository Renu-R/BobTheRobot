#include "Arduino.h"
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include "Timer.h"

const int TAPE_SENSOR_L = PA4;
const int TAPE_SENSOR_R = PB0;
const int begin = PB9;
const PinName R_F = PA_0;
const PinName R_B = PA_1;
const PinName L_F = PA_2;
const PinName L_B = PA_3;

#define SCREEN_WIDTH 128 // OLED display width, in pixelsS
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET 	-1 // This display does not have a reset pin accessible
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
int prev_err = 0;
int I = 0;
int Kp = 0;
int Ki = 0;
int Kd = 0;
int count = 0;

void setup() {
  analogReadResolution(12);

  pinMode(R_F, OUTPUT);
  pinMode(R_B, OUTPUT);
  pinMode(L_F, OUTPUT);
  pinMode(L_B, OUTPUT);

  pinMode(A6, INPUT_PULLDOWN);
  pinMode(A7, INPUT_PULLDOWN);
  //pinMode(A8, INPUT_PULLDOWN);

  pinMode(TAPE_SENSOR_L, INPUT_ANALOG);
  pinMode(TAPE_SENSOR_R, INPUT_ANALOG);

  pinMode(begin, OUTPUT);
  digitalWrite(begin, LOW);

  pwm_start(R_B, 100, 0, RESOLUTION_12B_COMPARE_FORMAT);
  pwm_start(L_B, 100, 0, RESOLUTION_12B_COMPARE_FORMAT);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.display();
}

int calculate_error (int l, int r){
  int error = 0;
  if(l<800 && r<800 && prev_err<0){
    error = -6;
  }
  else if(l<800 && r>800){
    error = -1;
  }
  else if (l>=800 && r>=800){
    error = 0;
  }
  else if(l>800 && r<800){
    error = 1;
  }
  else if(l<800 && r<800 && prev_err>0){
    error = 6;
  }
  return error;
}

int calculate_PID (int error){
  int P = error;
  //I += error;
  int D = error - prev_err;
  int PID =  Kp*P + Kd*D;
  prev_err = error;
  return PID;
}

void loop(){
  Kp =analogRead(PA6) / 11;
  Kd = analogRead(PA7) / 100;
  //Kd = analogRead(PA7) / 100;

  int l = analogRead(TAPE_SENSOR_L);
  int r = analogRead(TAPE_SENSOR_R);
  int error = calculate_error(l, r);

  if (error != 0 && count == 0 ){
    digitalWrite(begin, HIGH);
    count++;
  }
  int PID = calculate_PID(error);

  display.println(error);
  display.println(Kp);
  display.println(Kd);
  display.println(PID);
  display.display();
  //pwm_start(R_F, 100, 2000, RESOLUTION_12B_COMPARE_FORMAT);
  //pwm_start(L_F, 100, 2000, RESOLUTION_12B_COMPARE_FORMAT);


  if (PID>=0){
    pwm_start(R_F, 100, 1050+PID, RESOLUTION_12B_COMPARE_FORMAT);
    pwm_start(L_F, 100, 1050-PID, RESOLUTION_12B_COMPARE_FORMAT);
    
  }
  else if (PID<0){
    pwm_start(R_F, 100, 1050+PID, RESOLUTION_12B_COMPARE_FORMAT);
    pwm_start(L_F, 100, 1050-PID, RESOLUTION_12B_COMPARE_FORMAT);
  }
  
  display.clearDisplay();
  display.setCursor(0,0);
  delay(5);
}