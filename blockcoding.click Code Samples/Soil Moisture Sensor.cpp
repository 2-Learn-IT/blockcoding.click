#include <Arduino.h>

struct SoilMoistureSensor {
  String name = "";
  int PIN = 0;
  int dry_value = 0;
  int water_value = 0;
};

struct SoilMoistureSensor SoilMoistureSensor1;

void Soil_Moisture_calibrate(SoilMoistureSensor *sensor, int calibration_cycles) {
  long sum_moisture_dry = 0, sum_moisture_water = 0;
  
  printf("Starting %s calibration in 10 seconds...\nDry value calibration: Now the sensor needs to be totally dry in the air or in very dry soil or sand!", sensor->name);
  delay(10000);
  printf("Calibrating %s dry value now...", sensor->name);
  for (int calibration = 0; calibration < calibration_cycles; calibration++){
    sum_moisture_dry += analogRead(sensor->PIN);
    delay(5);
  }
  sensor->dry_value = sum_moisture_dry / (long)calibration_cycles;
  printf("Finished %s dry value calibration!\nStarting Soil Moisture Sensor water value calibration in 10 seconds...\nWater value calibration: Now the sensor needs to be in water or in very wet soil!", sensor->name);
  delay(10000);
  printf("Calibrating %s water value now...", sensor->name);
  for (int calibration = 0; calibration < calibration_cycles; calibration++){
    sum_moisture_water += analogRead(sensor->PIN);
    delay(5);
  }
  sensor->water_value = sum_moisture_water / calibration_cycles;
  printf("Finished %s calibration!", sensor->name);
}

void setup() {
  Serial.begin(115200);
  pinMode(SoilMoistureSensor1.PIN, INPUT);
  SoilMoistureSensor1.name = "Soil Moisture Sensor 1";
  SoilMoistureSensor1.PIN = 25;
  Soil_Moisture_calibrate(&SoilMoistureSensor1, 1000);

  Serial.print("Dry value: ");
  Serial.println(SoilMoistureSensor1.dry_value);
  Serial.print("Water value: ");
  Serial.println(SoilMoistureSensor1.water_value);
  delay(5000);
}

void loop() {
  Serial.print("Value: ");
  Serial.print(String(analogRead(SoilMoistureSensor1.PIN)));
  Serial.print(" Percent: ");
  Serial.println(String(map(analogRead(SoilMoistureSensor1.PIN), SoilMoistureSensor1.dry_value, SoilMoistureSensor1.water_value, 0, 100)));
  delay(100);
}