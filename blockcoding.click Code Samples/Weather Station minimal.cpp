#include <Arduino.h>

#include <Wire.h>
#include <BME280I2C.h>

BME280I2C bme;

void setup() {
  Serial.begin(115200);
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
}

void loop() {
  float temperature = 0;
  float humidity = 0;
  float air_pressure = 0;
  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_hPa);
  bme.read(air_pressure, temperature, humidity, tempUnit, presUnit);
  float height = ((288.15 / 0.0065) * (1 - pow(air_pressure / 1013.25, 1 / 5.255)));
  Serial.println((String("Temp: ") + String((String)(temperature)) + String(" °C")));
  Serial.println((String("Humidity: ") + String((String)(humidity)) + String(" %")));
  Serial.println((String("Air Pressure: ") + String((String)(air_pressure)) + String(" hPa")));
  Serial.println((String("Height: ") + String((String)(height)) + String(" m.ü.A.")));
  Serial.println("");
  delay(3000);

}