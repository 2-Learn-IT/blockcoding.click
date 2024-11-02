#include <Arduino.h>

struct SoilMoistureSensor {
  String name = "";
  int PIN = 0;
  int dry_value = 0;
  int water_value = 0;
};
struct SoilMoistureSensor SoilMoistureSensor25;
struct SoilMoistureSensor SoilMoistureSensor34;

void Soil_Moisture_calibrate(SoilMoistureSensor *sensor, int calibration_cycles) {
  long sum_moisture_dry = 0, sum_moisture_water = 0;
  Serial.println("Starting " + sensor->name + " calibration in 10 seconds...\nDry value calibration: Now the sensor needs to be totally dry in the air or in very dry soil or sand!");
  delay(10000);
  Serial.println("Calibrating " + sensor->name + " dry value now...");
  for (int calibration = 0; calibration < calibration_cycles; calibration++){
    sum_moisture_dry += analogRead(sensor->PIN);
    delay(5);
  }
  sensor->dry_value = sum_moisture_dry / (long)calibration_cycles;
  Serial.println(sensor->name + " Dry Value: " + (String)sensor->dry_value + "\nStarting Soil Moisture Sensor water value calibration in 10 seconds...\nWater value calibration: Now the sensor needs to be in water or in very wet soil!");
  delay(10000);
  Serial.println("Calibrating " + sensor->name + " water value now...");
  for (int calibration = 0; calibration < calibration_cycles; calibration++){
    sum_moisture_water += analogRead(sensor->PIN);
    delay(5);
  }
  sensor->water_value = sum_moisture_water / calibration_cycles;
  Serial.println(sensor->name + "Water Value: " + (String)sensor->water_value + "\nFinished " + sensor->name + " calibration!");
}

void setup() {
  Serial.begin(115200);
  pinMode(SoilMoistureSensor25.PIN, INPUT);
  SoilMoistureSensor25.PIN = 25;
  pinMode(SoilMoistureSensor34.PIN, INPUT);
  SoilMoistureSensor34.PIN = 34;
  SoilMoistureSensor34.name = "Soil Moisture Sensor 34";
  Soil_Moisture_calibrate(&SoilMoistureSensor34, 1000);
}

void loop() {
  Serial.println((String("Soil Sensor GPIO 25: ") + String(analogRead(SoilMoistureSensor25.PIN))));
  Serial.println((String("Soil Sensor GPIO 34: ") + String(analogRead(SoilMoistureSensor34.PIN)) + String(" - ") + String(map(analogRead(SoilMoistureSensor34.PIN), SoilMoistureSensor34.dry_value, SoilMoistureSensor34.water_value, 0, 100)) + String("%")));
  Serial.println("");
  delay(1000);

}