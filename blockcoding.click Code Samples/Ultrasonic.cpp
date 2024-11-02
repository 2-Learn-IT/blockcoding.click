#include <Arduino.h>

const int PIN_Ultrasonic_trigger = 19;
const int PIN_Ultrasonic_echo = 18;
#define SPEED_OF_SOUND 0.0343 //define speed of sound as 343m/s ≙ 0.0343 cm/µS



float HCSR04_measure() {
  long soundwave_round_trip_time = 0;
  float distance_in_cm = 0;

  digitalWrite(PIN_Ultrasonic_trigger, LOW);
  delayMicroseconds(2);
  
  digitalWrite(PIN_Ultrasonic_trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_Ultrasonic_trigger, LOW);
  soundwave_round_trip_time = pulseIn(PIN_Ultrasonic_echo, HIGH);
  
  distance_in_cm = soundwave_round_trip_time * SPEED_OF_SOUND/2;
  if(distance_in_cm > 400)
    return -1;
  else
    return distance_in_cm;
}

void setup() {
  Serial.begin(115200); // Starts the serial communication
  pinMode(PIN_Ultrasonic_trigger, OUTPUT);
  pinMode(PIN_Ultrasonic_echo, INPUT);
}

void loop() {
  Serial.print("Distanz: ");
  Serial.println(HCSR04_measure());
  delay(500);
}