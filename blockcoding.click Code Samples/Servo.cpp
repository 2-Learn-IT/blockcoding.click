#include <Arduino.h>

int PIN_Servo_23 = 23;
int Servo_23_angle = 0;
int angle = 0;

void servo_set_angle(int servo_pin, int angle_to_set){
  
  // Pulses duration: 600 - 0deg; 1450 - 90deg; 2300 - 180deg
  int servo_pin_high_time_microsec = round(600 + (angle_to_set * 9.4444444444));
  digitalWrite(PIN_Servo_23, HIGH);
  delayMicroseconds(servo_pin_high_time_microsec);
  digitalWrite(PIN_Servo_23, LOW);
  delayMicroseconds(20000 - servo_pin_high_time_microsec);
}

void setup() {
  Serial.begin(115200);
  
  pinMode(PIN_Servo_23, OUTPUT);
  servo_set_angle(90);
  Serial.println("Servo 90 Grad - Ausgangsposition! ... 10 sec warten");
  delay(10000);
}

void loop() {
  
  // scan from 0 to 180 degrees
  for(angle = 0; angle < 180; angle++)  
  {                                  
    servo_set_angle(angle);               
    delay(15);
    Serial.println("Servo: " + (String)(angle));
  }

  delay(2000);

    // now scan back from 180 to 0 degrees
  for(angle = 180; angle > 0; angle--)    
  {                                
    servo_set_angle(angle);           
    delay(15);
    Serial.println("Servo: " + (String)(angle));
  }
 
  Serial.println("Servo 0");
  delay(2000);
}