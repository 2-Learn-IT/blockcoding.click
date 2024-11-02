#include <Arduino.h>

#include <WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>
#include "soc/rtc_cntl_reg.h"

char* ssid = "Blockcoding.click";
const char* key = "wlan4blockcoding.click";
int servoAngle = 90;
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_client_name;
const char* mqtt_user = "";
const char* mqtt_password = "";
WiFiClient espClient;
PubSubClient client(espClient);

Servo myServo23;

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
    myServo23.write(servoAngle);
  }
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);
  Serial.print("Connecting to WLAN Network: Blockcoding.click");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, key);
  while (WiFi.status() != WL_CONNECTED) {
	delay(500);
	Serial.print(".");
  }
  Serial.println();
  Serial.println("WLAN connected!");
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
  myServo23.attach(23);
  client.subscribe(String("esp32-servo").c_str());
  Serial.println("Subscribed to MQTT topic 'esp32-servo'");
  client.setCallback(mqtt_callback);

  myServo23.write(servoAngle);
  client.publish(String("esp32-servo").c_str(), String(servoAngle).c_str());

}

void loop() {
  client.loop();
}