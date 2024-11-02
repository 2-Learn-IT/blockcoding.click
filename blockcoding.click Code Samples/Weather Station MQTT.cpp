#include <Arduino.h>

#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <BME280I2C.h>
#include <SparkFun_SGP30_Arduino_Library.h>
#include <Servo.h>

char* ssid = "Blockcoding.click";
const char* key = "wlan4blockcoding.click";
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_client_name;
const char* mqtt_user = "";
const char* mqtt_password = "";
WiFiClient espClient;
PubSubClient client(espClient);
BME280I2C bme;
SGP30 SGP30_sensor;

uint32_t getAbsoluteHumidity(float temperature, float humidity) {
  const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3] formula source: Sensirion SGP30 Driver Integration chapter 3.15
  const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
  return absoluteHumidityScaled;
}

int servoAngle = 90;
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
  Wire.begin();
  while(!bme.begin())
  {
    Serial.println("Could not find BME280 or BMP280 sensor! Loop - Checking again in 2 seconds!");
    delay(2000);
  }

  switch(bme.chipModel())
  {
    case BME280::ChipModel_BME280:
      Serial.println("Found BME280 sensor!");
      break;
    case BME280::ChipModel_BMP280:
      Serial.println("Found BMP280 sensor! No Humidity available.");
      break;
    default:
      Serial.println("Found UNKNOWN sensor! Error!");
  }

  while(!SGP30_sensor.begin())
  {
    Serial.println("Could not find SGP30 sensor! Loop - Checking again in 2 seconds!");
    delay(2000);
  }

  SGP30_sensor.initAirQuality();
  SGP30_sensor.getBaseline();

  myServo23.attach(23);
  client.subscribe(String("esp32-servo").c_str());
  Serial.println("Subscribed to MQTT topic 'esp32-servo'");
  client.setCallback(mqtt_callback);

  myServo23.write(servoAngle);
  client.publish(String("esp32-servo").c_str(), String(servoAngle).c_str());

}

void loop() {
  float temperature = 0;
  float humidity = 0;
  float air_pressure = 0;
  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_hPa);
  bme.read(air_pressure, temperature, humidity, tempUnit, presUnit);
  float height = ((288.15 / 0.0065) * (1 - pow(air_pressure / 1013.25, 1 / 5.255)));
  int CO2_eq = 0;
  int TVOC = 0;
  int H2 = 0;
  int ethanol = 0;
  SGP30_sensor.setHumidity(getAbsoluteHumidity(temperature, humidity));
  SGP30_sensor.measureAirQuality();
  SGP30_sensor.measureRawSignals();
  CO2_eq = SGP30_sensor.CO2;
  TVOC = SGP30_sensor.TVOC;
  H2 = SGP30_sensor.H2;
  ethanol = SGP30_sensor.ethanol;
  client.publish(String("esp32-air-temp").c_str(), String(temperature).c_str());
  client.publish(String("esp32-air-humidity").c_str(), String(humidity).c_str());
  client.publish(String("esp32-air-pressure").c_str(), String(air_pressure).c_str());
  client.publish(String("esp32-air-height").c_str(), String(height).c_str());
  client.publish(String("esp32-air-co2").c_str(), String(CO2_eq).c_str());
  client.publish(String("esp32-air-tvoc").c_str(), String(TVOC).c_str());
  client.publish(String("esp32-air-h2").c_str(), String(H2).c_str());
  client.publish(String("esp32-air-ethanol").c_str(), String(ethanol).c_str());
  Serial.println((String("Temp: ") + String((String)(temperature)) + String(" °C")));
  Serial.println((String("Humidity: ") + String((String)(humidity)) + String(" %")));
  Serial.println((String("Air Pressure: ") + String((String)(air_pressure)) + String(" hPa")));
  Serial.println((String("Height: ") + String((String)(height)) + String(" m.ü.A.")));
  Serial.println((String("CO2-eq: ") + String((String)(CO2_eq)) + String(" ppm")));
  Serial.println((String("TVOC: ") + String((String)(TVOC)) + String(" ppb")));
  Serial.println((String("H2: ") + String((String)(H2)) + String(" RAW value")));
  Serial.println((String("Ethanol: ") + String((String)(ethanol)) + String(" RAW value")));
  Serial.println("");
  delay(5000);

}