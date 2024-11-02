#include <Arduino.h>
#include <WiFi.h>
#include <OTA.h>

const char* ssid = "Patricks Network";
const char* key = "wlan@pati";


void setup() {
  Serial.begin(115200);

  Serial.print("Connecting to WLAN Network: Patricks Network");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, key);
  while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
  }
  Serial.println("\nWLAN connected!");
  Serial.print("Your IP address: ");
  Serial.println(WiFi.localIP());

  setup_OTA("ESP32");
  
}

void loop() {
  Serial.println("Das ist das OTA Programm!");
  delay(5000);
}