#include <Arduino.h>

int PIN_MotorA_enA = 13;
int PIN_MotorA_in1 = 32;
int PIN_MotorA_in2 = 33;
int PIN_MotorB_in3 = 25;
int PIN_MotorB_in4 = 26;
int PIN_MotorB_enB = 27;

float motorSpeed = 1;
int motorSpin = 127;

void reset_motor_spin() {
  motorSpin = 127;
  int speed_spin = (int)(motorSpin * motorSpeed);
  Serial.println((String("Reset Speed * Spin: ") + String(speed_spin)));
  ledcWrite(0, speed_spin);
  ledcWrite(1, speed_spin);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_MotorA_enA, OUTPUT);
  ledcSetup(0, 500, 7);
  ledcSetup(1, 500, 7);
  ledcWrite(0, 127);
  ledcWrite(1, 127);
  ledcAttachPin(PIN_MotorA_enA, 0);
  pinMode(PIN_MotorA_in1, OUTPUT);
  pinMode(PIN_MotorA_in2, OUTPUT);
  pinMode(PIN_MotorB_in3, OUTPUT);
  pinMode(PIN_MotorB_in4, OUTPUT);
  pinMode(PIN_MotorB_enB, OUTPUT);
  ledcAttachPin(PIN_MotorB_enB, 0);

  digitalWrite(PIN_MotorA_in1, LOW);
  digitalWrite(PIN_MotorA_in2, LOW);
  Serial.println("Motor A stopped!");
  digitalWrite(PIN_MotorB_in3, LOW);
  digitalWrite(PIN_MotorB_in4, LOW);
  Serial.println("Motor B stopped!");

}

void loop() {
  delay(2000);
  reset_motor_spin();
  digitalWrite(PIN_MotorA_in1, HIGH);
  digitalWrite(PIN_MotorA_in2, LOW);
  Serial.println("Motor A forward!");
  digitalWrite(PIN_MotorB_in3, HIGH);
  digitalWrite(PIN_MotorB_in4, LOW);
  Serial.println("Motor B forward!");
  delay(2000);

  Serial.println((String("esp32-motor-spin: 200")));
  
  motorSpin = 200;
  if (motorSpin <= 127) {
    ledcWrite(0, (motorSpin * motorSpeed));
    ledcWrite(1, (127 * motorSpeed));
  } else if (motorSpin > 127) {
    ledcWrite(0, (127 * motorSpeed));
    ledcWrite(1, ((127 - (motorSpin - 127)) * motorSpeed));
  }

  motorSpeed = motorSpeed - 0.2;

}