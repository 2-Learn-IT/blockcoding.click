#include <Arduino.h>

#include <WiFi.h>
#include <PubSubClient.h>

char* ssid = "Blockcoding.click";
const char* key = "wlan4blockcoding.click";
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_client_name;
const char* mqtt_user = "";
const char* mqtt_password = "";
WiFiClient espClient;
PubSubClient client(espClient);



void setup() {
  Serial.begin(115200);
  pinMode(34, INPUT);

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
}

void loop() {
  float analog = analogRead(34);
  float messung_roh = ((analog / 4095) * 3.3);
  float messung_v = messung_roh * 7000 / 2000;
  Serial.println((String("Analog: ") + String(analog)));
  Serial.println((String("Messung ROH: ") + String(messung_roh) + String("V")));
  Serial.println((String("Batteriezustand: ") + String(messung_v) + String("V\n")));
  client.publish(String("esp32-bat").c_str(), String(messung_v).c_str());

  delay(1000);

}

