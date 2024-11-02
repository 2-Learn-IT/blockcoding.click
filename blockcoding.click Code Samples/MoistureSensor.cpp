#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  pinMode(23, OUTPUT);
  pinMode(34, INPUT);
}

void loop() {
  int measuredHumidity = 0;
  digitalWrite(23, HIGH);
  measuredHumidity = analogRead(34);
  Serial.println((String("Measured Humidity: ") + String(measuredHumidity)));
  Serial.println("In 30 sec wird die Pumpe eingeschaltet");
  delay(30000);
  Serial.println("Pumpe an!");
  digitalWrite(23, LOW);
  delay(2000);
  digitalWrite(23, HIGH);
  Serial.println("Pumpe aus!");

}