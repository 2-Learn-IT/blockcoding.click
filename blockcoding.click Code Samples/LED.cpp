#include <Arduino.h>

  int red = 2;
  int green = 4;
  int blue = 5;

void setup() {
  Serial.begin(115200);
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
}

void loop() {
  Serial.println("Rote LED ein");
  digitalWrite(red, HIGH);
  delay(10000);
  digitalWrite(red, LOW);
  Serial.println("Grün LED ein");
  digitalWrite(green, HIGH);
  delay(10000);
  digitalWrite(green, LOW);
  Serial.println("Blau LED ein");
  digitalWrite(blue, HIGH);
  delay(10000);
  digitalWrite(blue, LOW);
}