#include <Arduino.h>

#include <WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>

char* ssid = "Blockcoding.click";
const char* key = "wlan4blockcoding.click";
const char* mqtt_server = "broker.hivemq.com";
int mqtt_port = 1883;
const char* mqtt_client_name;
const char* mqtt_user = "";
const char* mqtt_password = "";
WiFiClient espClient;
PubSubClient client(espClient);
String value_red = "";
String value_green = "";
String value_blue = "";
char* text_to_cut_global;
char* text_delimiter_global;

int servoAngle = 90;
Servo myServo23;

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
  if (strcmp(topic, "esp32-led-red") == 0){
    Serial.println((String("esp32-led-red: ") + String(message)));
    ledcWrite(0, message.toInt());
    value_red = (String)(message);
    client.publish(String("esp32-led-rgb-set").c_str(), String((String("rgb( ") + String(value_red) + String(", ") + String(value_green) + String(", ") + String(value_blue) + String(")"))).c_str());
  }
  if (strcmp(topic, "esp32-led-green") == 0){
    Serial.println((String("esp32-led-green: ") + String(message)));
    ledcWrite(1, message.toInt());
    value_green = (String)(message);
    client.publish(String("esp32-led-rgb-set").c_str(), String((String("rgb( ") + String(value_red) + String(", ") + String(value_green) + String(", ") + String(value_blue) + String(")"))).c_str());
  }
  if (strcmp(topic, "esp32-led-blue") == 0){
    Serial.println((String("esp32-led-blue: ") + String(message)));
    ledcWrite(2, message.toInt());
    value_blue = (String)(message);
    client.publish(String("esp32-led-rgb-set").c_str(), String((String("rgb( ") + String(value_red) + String(", ") + String(value_green) + String(", ") + String(value_blue) + String(")"))).c_str());
  }
  if (strcmp(topic, "esp32-led-rgb") == 0){
    Serial.println((String("esp32-led-rgb: ") + String(message) + String("--")));
    String tmp_color_string = cut_text(message, "(");
    tmp_color_string = strtok(NULL, text_delimiter_global);
    value_red = cut_text(tmp_color_string, ", ");
    Serial.println((String("value red: ") + String(value_red)));
    ledcWrite(0, value_red.toInt());
    client.publish(String("esp32-led-red-set").c_str(), String(value_red.toInt()).c_str());
    value_green = strtok(NULL, text_delimiter_global);
    Serial.println((String("value green: ") + String(value_green)));
    ledcWrite(1, value_green.toInt());
    client.publish(String("esp32-led-green-set").c_str(), String(value_green.toInt()).c_str());
    value_blue = strtok(NULL, text_delimiter_global);
    value_blue = cut_text(value_blue, ")");
    Serial.println((String("value blue: ") + String(value_blue)));
    ledcWrite(2, value_blue.toInt());
    client.publish(String("esp32-led-blue-set").c_str(), String(value_blue.toInt()).c_str());
    client.publish(String("esp32-led-rgb-set").c_str(), String((String("rgb( ") + String(value_red) + String(", ") + String(value_green) + String(", ") + String(value_blue) + String(")"))).c_str());
  }
  if (strcmp(topic, "esp32-servo") == 0){
    Serial.println((String("esp32-servo: ") + String(message)));
    servoAngle = message.toInt();
    myServo23.write(servoAngle);
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
  ledcSetup(0, 200, 8);
  ledcAttachPin(2, 0);
  ledcSetup(1, 200, 8);
  ledcAttachPin(4, 1);
  ledcSetup(2, 200, 8);
  ledcAttachPin(5, 2);
  client.subscribe(String("esp32-led-red").c_str());
  Serial.println("Subscribed to MQTT topic 'esp32-led-red'");
  client.subscribe(String("esp32-led-green").c_str());
  Serial.println("Subscribed to MQTT topic 'esp32-led-green'");
  client.subscribe(String("esp32-led-blue").c_str());
  Serial.println("Subscribed to MQTT topic 'esp32-led-blue'");
  client.subscribe(String("esp32-led-rgb").c_str());
  Serial.println("Subscribed to MQTT topic 'esp32-led-rgb'");
  client.setCallback(mqtt_callback);

  ledcWrite(0, 0);
  ledcWrite(1, 0);
  ledcWrite(2, 0);
  client.publish(String("esp32-led-red").c_str(), String("0").c_str());
  client.publish(String("esp32-led-green").c_str(), String("0").c_str());
  client.publish(String("esp32-led-blue").c_str(), String("0").c_str());
  client.publish(String("esp32-led-rgb").c_str(), String("rgb(0,0,0)").c_str());

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