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

void mqtt_callback(char* topic, byte* message_byte, unsigned int length) {
  char* tmp_message = (char*) calloc(length+1,sizeof(char));
  for (int z = 0; z < length; z++){
    tmp_message[z] = (char)message_byte[z];
  }
  tmp_message[length] = '\0'; //set null char as end of string
  String message = String(tmp_message);
  if (strcmp(topic, "esp32-test") == 0){
    Serial.println((String("esp32-test: ") + String(message)));
  }
  if (strcmp(topic, "esp32-led") == 0){
    Serial.println((String("esp32-led: ") + String(message)));
    if (strcmp(message.c_str(), "red:on") == 0) {
      digitalWrite(2, HIGH);
      client.publish(String("esp32-led").c_str(), String((String("status-red:") + String(digitalRead(2)))).c_str());
    } else if (strcmp(message.c_str(), "red:off") == 0) {
      digitalWrite(2, LOW);
      client.publish(String("esp32-led").c_str(), String((String("status-red:") + String(digitalRead(2)))).c_str());
    }
    if (strcmp(message.c_str(), "green:on") == 0) {
      digitalWrite(4, HIGH);
      client.publish(String("esp32-led").c_str(), String((String("status-green:") + String(digitalRead(4)))).c_str());
    } else if (strcmp(message.c_str(), "green:off") == 0) {
      digitalWrite(4, LOW);
      client.publish(String("esp32-led").c_str(), String((String("status-green:") + String(digitalRead(4)))).c_str());
    }
    if (strcmp(message.c_str(), "blue:on") == 0) {
      digitalWrite(5, HIGH);
      client.publish(String("esp32-led").c_str(), String((String("status-blue:") + String(digitalRead(5)))).c_str());
    } else if (strcmp(message.c_str(), "blue:off") == 0) {
      digitalWrite(5, LOW);
      client.publish(String("esp32-led").c_str(), String((String("status-blue:") + String(digitalRead(5)))).c_str());
    }
  }
}

void setup() {
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
  client.subscribe(String("esp32-test").c_str());
  Serial.println("Subscribed to MQTT topic 'esp32-test'");
  pinMode(2, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  client.subscribe(String("esp32-led").c_str());
  Serial.println("Subscribed to MQTT topic 'esp32-led'");
  client.setCallback(mqtt_callback);

  client.publish(String("esp32-led").c_str(), String("status-red:0").c_str());
  client.publish(String("esp32-led").c_str(), String("status-green:0").c_str());
  client.publish(String("esp32-led").c_str(), String("status-blue:0").c_str());

}

void loop() {
  client.loop();

}