#include <Arduino.h>

#include <Wire.h>
#include <BME280I2C.h>
#include <SparkFun_SGP30_Arduino_Library.h>

BME280I2C bme;
SGP30 SGP30_sensor;

uint32_t getAbsoluteHumidity(float temperature, float humidity) {
  const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3] formula source: Sensirion SGP30 Driver Integration chapter 3.15
  const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
  return absoluteHumidityScaled;
}


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
  while(!SGP30_sensor.begin())
  {
    Serial.println("Could not find SGP30 sensor! Loop - Checking again in 2 seconds!");
    delay(2000);
  }

  SGP30_sensor.initAirQuality();
  SGP30_sensor.getBaseline();

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
  Serial.println((String("Temp: ") + String((String)(temperature)) + String(" °C")));
  Serial.println((String("Humidity: ") + String((String)(humidity)) + String(" %")));
  Serial.println((String("Air Pressure: ") + String((String)(air_pressure)) + String(" hPa")));
  Serial.println((String("Height: ") + String((String)(height)) + String(" m.ü.A.")));
  Serial.println((String("CO2-eq: ") + String((String)(CO2_eq)) + String(" ppm")));
  Serial.println((String("TVOC: ") + String((String)(TVOC)) + String(" ppb")));
  Serial.println((String("H2: ") + SGP30_sensor.H2 + String(" RAW value")));
  Serial.println((String("Ethanol: ") + SGP30_sensor.ethanol + String(" RAW value")));
  Serial.println("");
  delay(1000);

}