#include <Arduino.h>

#include <SparkFun_SGP30_Arduino_Library.h> // Click here to get the library: http://librarymanager/All#SparkFun_SGP30
#include <Wire.h>
 
SGP30 SGP30_sensor; //create an object of the SGP30 class

uint32_t getAbsoluteHumidity(float temperature, float humidity) {
    // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
    return absoluteHumidityScaled;
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  while(!SGP30_sensor.begin())
  {
    Serial.println("Could not find SGP30 sensor! Loop - Checking again in 2 seconds!");
    delay(2000);
  }

  SGP30_sensor.getBaseline();
  SGP30_sensor.initAirQuality();

}
 
void loop() {

    delay(1000);
    SGP30_sensor.setHumidity(getAbsoluteHumidity(35, 70));
    SGP30_sensor.measureAirQuality();
    Serial.print("CO2: ");
    Serial.print(SGP30_sensor.CO2);
    Serial.print(" ppm\tTVOC: ");
    Serial.print(SGP30_sensor.TVOC);
    Serial.println(" ppb");
    
    SGP30_sensor.measureRawSignals();
    Serial.print("Raw H2: ");
    Serial.print(SGP30_sensor.H2);
    Serial.print(" \tRaw Ethanol: ");
    Serial.println(SGP30_sensor.ethanol);
}