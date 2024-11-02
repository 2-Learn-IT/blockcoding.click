#include <Arduino.h>

#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    //snprintf(MAC_address, 12, "%llX", ESP.getEfuseMac());
    
    //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager WiFi_Manager;
    bool WiFi_Manager_status;
    String WiFi_Manager_SSID = "ESP32-" + WiFi.macAddress();

    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    // it is a good practice to make sure your code sets wifi mode how you want it.

    WiFi_Manager.setDebugOutput(true);
    WiFi_Manager.setConfigPortalTimeout(600);

    // reset settings - wipe stored credentials for testing
    // these are stored by the esp library
    WiFi_Manager.resetSettings();
 
    // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
    // if empty will auto generate SSID, if password is blank it will be anonymous AP (WiFi_Manager.autoConnect())
    // then goes into a blocking loop awaiting configuration and will return success result
 
    
    // res = WiFi_Manager.autoConnect(); // auto generated AP name from chipid
    // res = WiFi_Manager.autoConnect("AutoConnectAP"); // anonymous ap
    
    const char* mqtt_server = "broker.hivemq.com";
    const char* mqtt_client_name = ("ESP32-" + WiFi.macAddress()).c_str();
    const char* mqtt_user = "";
    const char* mqtt_password = "";

    
    WiFiManagerParameter MQTT_Server_IP_text_box("MQTT_Server_IP", "MQTT Server IP:", mqtt_server, 50);
    WiFi_Manager.addParameter(&MQTT_Server_IP_text_box);
    WiFiManagerParameter MQTT_Server_Port_text_box("MQTT_Server_Port", "MQTT Server Port:", "1883", 5);
    WiFi_Manager.addParameter(&MQTT_Server_Port_text_box);
    WiFiManagerParameter MQTT_Client_Name_text_box("MQTT_Client_Name", "MQTT Client Name:", ("ESP32-" + WiFi.macAddress()).c_str(), 50);
    WiFi_Manager.addParameter(&MQTT_Client_Name_text_box);
    WiFiManagerParameter MQTT_User_text_box("MQTT_User", "Optional: MQTT User:", "", 50);
    WiFi_Manager.addParameter(&MQTT_User_text_box);
    WiFiManagerParameter MQTT_Password_text_box("MQTT_Password", "Optional: MQTT Pasword:", "", 50);
    WiFi_Manager.addParameter(&MQTT_Password_text_box);
    
    Serial.print("WiFi Manager: Waiting for client to connect to WLAN Network SSID: " + (String)WiFi_Manager_SSID);
    Serial.println("\nAs soon as your client is connected visit http://192.168.4.1/ and configure your WLAN");

    WiFi_Manager_status = WiFi_Manager.autoConnect(WiFi_Manager_SSID.c_str(), "12345678"); // password protected ap
    if(!WiFi_Manager_status) {
      Serial.println("WiFi Manager failed to connect to WLAN -> TIMOUT -> Restart!");
      delay(3000);
      ESP.restart();
      delay(5000);
    }

    mqtt_server = MQTT_Server_IP_text_box.getValue();
    const int mqtt_port = (int)MQTT_Server_Port_text_box.getValue();
    mqtt_client_name = MQTT_Client_Name_text_box.getValue();

    Serial.print("MQTT_Server_IP: ");
    Serial.println(MQTT_Server_IP_text_box.getValue());
    Serial.print("MQTT_Server_Port: ");
    Serial.println(MQTT_Server_Port_text_box.getValue());
    Serial.print("MQTT_Client_Name: ");
    Serial.println(MQTT_Client_Name_text_box.getValue());
    Serial.print("MQTT_User: ");
    Serial.println(MQTT_User_text_box.getValue());
    Serial.print("MQTT_Password: ");
    Serial.println(MQTT_Password_text_box.getValue());

}
 
void loop() {
    // put your main code here, to run repeatedly:   
}