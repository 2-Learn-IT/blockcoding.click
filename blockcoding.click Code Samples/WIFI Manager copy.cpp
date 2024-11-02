#include <Arduino.h>

#include <WiFiManager.h>

WiFiManager WiFi_Manager;
bool WiFi_Manager_status;
String WiFi_Manager_SSID = (String("ESP32-") + String(WiFi.macAddress()));
const char* mqtt_server = "";
const char* mqtt_port = "1883";
const char* mqtt_client_name = "";
const char* mqtt_user = "";
const char* mqtt_password = "";

void setup() {
  WiFi.mode(WIFI_STA);
  WiFi_Manager.setDebugOutput(false);
  WiFi_Manager.setConfigPortalTimeout(600);

  WiFiManagerParameter MQTT_Server_IP_text_box("MQTT_Server_IP", "MQTT Server IP:", mqtt_server, 50);
  WiFi_Manager.addParameter(&MQTT_Server_IP_text_box);
  WiFiManagerParameter MQTT_Server_Port_text_box("MQTT_Server_Port", "MQTT Server Port:", mqtt_port, 5);
  WiFi_Manager.addParameter(&MQTT_Server_Port_text_box);
  WiFiManagerParameter MQTT_Client_Name_text_box("MQTT_Client_Name", "MQTT Client Name:", mqtt_client_name, 50);
  WiFi_Manager.addParameter(&MQTT_Client_Name_text_box);
  WiFiManagerParameter MQTT_User_text_box("MQTT_User", "Optional: MQTT User:", mqtt_user, 50);
  WiFi_Manager.addParameter(&MQTT_User_text_box);
  WiFiManagerParameter MQTT_Password_text_box("MQTT_Password", "Optional: MQTT Pasword:", mqtt_password, 50);
  WiFi_Manager.addParameter(&MQTT_Password_text_box);

  Serial.print("WiFi Manager: Waiting for client to connect to WLAN Network SSID: " + (String)WiFi_Manager_SSID);
  Serial.println("As soon as your client is connected visit http://192.168.4.1/ and configure your WLAN");
  WiFi_Manager_status = WiFi_Manager.autoConnect(WiFi_Manager_SSID.c_str());

  if(!WiFi_Manager_status) {
    	Serial.println("WiFi Manager failed to connect to WLAN -> TIMOUT -> Restart!");
    	ESP.restart();
    	delay(5000);
  }

 mqtt_server = MQTT_Server_IP_text_box.getValue();
 mqtt_port = (int)MQTT_Server_Port_text_box.getValue();
 mqtt_client_name = MQTT_Client_Name_text_box.getValue();
 mqtt_user = MQTT_User_text_box.getValue();
 mqtt_password = MQTT_Password_text_box.getValue();

}

void loop() {

}