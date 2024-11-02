#include <Arduino.h>

#include "soc/rtc_cntl_reg.h"
#include <WiFi.h>
#include <PubSubClient.h>

int servoAngle = 90;
float motorSpeed = 0.9;
int motorSpin = 127;
const char* ssid = "Blockcoding.click";
const char* key = "wlan4blockcoding.click";
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_client_name;
const char* mqtt_user = "";
const char* mqtt_password = "";
WiFiClient espClient;
PubSubClient client(espClient);
int Servo_23_angle = 0;
char* text_to_cut_global;
char* text_delimiter_global;

int PIN_MotorA_enA = 13;
int PIN_MotorA_in1 = 32;
int PIN_MotorA_in2 = 33;
int PIN_MotorB_in3 = 25;
int PIN_MotorB_in4 = 26;
int PIN_MotorB_enB = 27;
int PIN_Servo_23 = 23;

// Describe this function...
void reset_motor_spin() {
  motorSpin = 127;
  int speed_spin = (motorSpin * motorSpeed);
  Serial.println((String("Reset Speed * Spin: ") + String(speed_spin)));
  ledcWrite(2, speed_spin);
  ledcWrite(3, speed_spin);
  client.publish(String("esp32-motor-spin-set").c_str(), String(motorSpin).c_str());
}

void servo_set_angle(int servo_pin, int angle_to_set){
  // Pulses duration: 0deg - 600; 90deg - 1450; 180deg - 2300
  int servo_pin_high_time_microsec = round(600 + (angle_to_set * 9.4444444444));
  digitalWrite(servo_pin, HIGH);
  delayMicroseconds(servo_pin_high_time_microsec);
  digitalWrite(servo_pin, LOW);
  delayMicroseconds(20000 - servo_pin_high_time_microsec);
}

char* cut_text(String original_text, String delimiter) {
  text_to_cut_global = (char*) calloc(String(original_text).length(),sizeof(char));
  strcpy(text_to_cut_global, original_text.c_str());
  text_delimiter_global = (char*) calloc(String(delimiter).length(),sizeof(char));
  strcpy(text_delimiter_global, delimiter.c_str());
  return strtok(text_to_cut_global, text_delimiter_global);
}

void mqtt_callback(char* topic, byte* message_byte, unsigned int length) {
  char* tmp_message = (char*) calloc(length+1,sizeof(char));
  for (int z = 0; z < length; z++){
    tmp_message[z] = (char)message_byte[z];
  }
  tmp_message[length] = '\0'; //set null char as end of string
  String message = String(tmp_message);
  if (strcmp(topic, "esp32-servo") == 0){
    Serial.println((String("esp32-servo: ") + String(message)));
    servoAngle = message.toInt();
    servo_set_angle(PIN_Servo_23, servoAngle);
    Servo_23_angle = servoAngle;
  }
  if (strcmp(topic, "esp32-motor") == 0){
    Serial.println((String("esp32-motor: ") + String(message)));
    if (strcmp(message.c_str(), "forward:A+B") == 0) {
      digitalWrite(PIN_MotorA_in1, HIGH);
      digitalWrite(PIN_MotorA_in2, LOW);
      Serial.println("Motor A forward!");
      digitalWrite(PIN_MotorB_in3, HIGH);
      digitalWrite(PIN_MotorB_in4, LOW);
      Serial.println("Motor B forward!");
    } else if (strcmp(message.c_str(), "backward:A+B") == 0) {
      digitalWrite(PIN_MotorA_in1, LOW);
      digitalWrite(PIN_MotorA_in2, HIGH);
      Serial.println("Motor A backward!");
      digitalWrite(PIN_MotorB_in3, LOW);
      digitalWrite(PIN_MotorB_in4, HIGH);
      Serial.println("Motor B backward!");
    } else if (strcmp(message.c_str(), "stop:A+B") == 0) {
      digitalWrite(PIN_MotorA_in1, LOW);
      digitalWrite(PIN_MotorA_in2, LOW);
      Serial.println("Motor A stopped!");
      digitalWrite(PIN_MotorB_in3, LOW);
      digitalWrite(PIN_MotorB_in4, LOW);
      Serial.println("Motor B stopped!");
    } else if (strcmp(cut_text(message, ":"), "rotate") == 0) {
      reset_motor_spin();
      String direction = strtok(NULL, text_delimiter_global);
      if (strcmp(direction.c_str(), "left") == 0) {
        digitalWrite(PIN_MotorA_in1, LOW);
        digitalWrite(PIN_MotorA_in2, LOW);
        Serial.println("Motor A stopped!");
        digitalWrite(PIN_MotorB_in3, LOW);
        digitalWrite(PIN_MotorB_in4, LOW);
        Serial.println("Motor B stopped!");
        delay(200);
        digitalWrite(PIN_MotorB_in3, LOW);
        digitalWrite(PIN_MotorB_in4, HIGH);
        Serial.println("Motor B backward!");
        digitalWrite(PIN_MotorA_in1, HIGH);
        digitalWrite(PIN_MotorA_in2, LOW);
        Serial.println("Motor A forward!");
      } else {
        digitalWrite(PIN_MotorA_in1, LOW);
        digitalWrite(PIN_MotorA_in2, LOW);
        Serial.println("Motor A stopped!");
        digitalWrite(PIN_MotorB_in3, LOW);
        digitalWrite(PIN_MotorB_in4, LOW);
        Serial.println("Motor B stopped!");
        delay(200);
        digitalWrite(PIN_MotorB_in3, HIGH);
        digitalWrite(PIN_MotorB_in4, LOW);
        Serial.println("Motor B forward!");
        digitalWrite(PIN_MotorA_in1, LOW);
        digitalWrite(PIN_MotorA_in2, HIGH);
        Serial.println("Motor A backward!");
      }
    }
  }
  if (strcmp(topic, "esp32-motor-spin") == 0){
    Serial.println((String("esp32-motor-spin: ") + String(message)));
    client.publish(String("esp32-motor-spin-set").c_str(), String(message).c_str());
    motorSpin = message.toInt();
    if (motorSpin <= 127) {
      ledcWrite(3, (motorSpin * motorSpeed));
      ledcWrite(2, (127 * motorSpeed));
    } else if (motorSpin > 127) {
      ledcWrite(3, (127 * motorSpeed));
      ledcWrite(2, ((127 - (motorSpin - 127)) * motorSpeed));
    }
  }
  if (strcmp(topic, "esp32-motor-speed") == 0){
    Serial.println((String("esp32-motor-speed: ") + String(message)));
    client.publish(String("esp32-motor-speed-set").c_str(), String(message).c_str());
    motorSpeed = message.toFloat();
    if (motorSpin <= 127) {
      ledcWrite(3, (motorSpin * motorSpeed));
      ledcWrite(2, (127 * motorSpeed));
    } else if (motorSpin > 127) {
      ledcWrite(3, (127 * motorSpeed));
      ledcWrite(2, ((127 - (motorSpin - 127)) * motorSpeed));
    }
  }
}

void setup() {
  Serial.begin(115200);
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  pinMode(PIN_MotorA_enA, OUTPUT);
  ledcSetup(2, 1000, 7);
  ledcWrite(2, 127);
  ledcAttachPin(PIN_MotorA_enA, 2);
  pinMode(PIN_MotorA_in1, OUTPUT);
  pinMode(PIN_MotorA_in2, OUTPUT);
  pinMode(PIN_MotorB_in3, OUTPUT);
  pinMode(PIN_MotorB_in4, OUTPUT);
  pinMode(PIN_MotorB_enB, OUTPUT);
  ledcSetup(3, 1000, 7);
  ledcWrite(3, 127);
  ledcAttachPin(PIN_MotorB_enB, 3);
  Serial.print("Connecting to WLAN Network: Blockcoding.click");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, key);
  while (WiFi.status() != WL_CONNECTED) {
	  delay(500);
	  Serial.print(".");
  }
  Serial.println("\nWLAN connected!");
  Serial.print("Your IP address: ");
  Serial.println(WiFi.localIP());
  String mqtt_client_name_str = "";
  mqtt_client_name = mqtt_client_name_str.c_str();
  client.setServer(mqtt_server, mqtt_port);
  Serial.print("Connecting to MQTT Server");
  while (!client.connected()) {
    Serial.print(".");
    if(client.connect(mqtt_client_name))
      Serial.println("\nMQTT connected!");
    else {
      Serial.print("\nMQTT Connection failed with state: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
  pinMode(PIN_Servo_23, OUTPUT);
  client.subscribe(String("esp32-servo").c_str());
  Serial.println("Subscribed to MQTT topic 'esp32-servo'");
  client.subscribe(String("esp32-motor").c_str());
  Serial.println("Subscribed to MQTT topic 'esp32-motor'");
  client.subscribe(String("esp32-motor-spin").c_str());
  Serial.println("Subscribed to MQTT topic 'esp32-motor-spin'");
  client.subscribe(String("esp32-motor-speed").c_str());
  Serial.println("Subscribed to MQTT topic 'esp32-motor-speed'");
  client.setCallback(mqtt_callback);

  digitalWrite(PIN_MotorA_in1, LOW);
  digitalWrite(PIN_MotorA_in2, LOW);
  Serial.println("Motor A stopped!");
  digitalWrite(PIN_MotorB_in3, LOW);
  digitalWrite(PIN_MotorB_in4, LOW);
  Serial.println("Motor B stopped!");
  client.publish(String("esp32-motor").c_str(), String("stop:A+B").c_str());
  servo_set_angle(PIN_Servo_23, servoAngle);
  Servo_23_angle = servoAngle;
  client.publish(String("esp32-servo").c_str(), String(servoAngle).c_str());
  client.publish(String("esp32-log").c_str(), String("ESP32 booted and set!").c_str());
  ledcWrite(2, (127 * motorSpeed));
  ledcWrite(3, (127 * motorSpeed));
  client.publish(String("esp32-motor-speed-set").c_str(), String(motorSpeed).c_str());
  client.publish(String("esp32-motor-spin-set").c_str(), String(motorSpin).c_str());

}

void loop() {
  client.loop();

}