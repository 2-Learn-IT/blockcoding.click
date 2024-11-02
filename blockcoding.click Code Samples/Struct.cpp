#include <Arduino.h>
struct sensor {
  int deviceId = 0;
  int measurementType;
  float value;
};


void setup()
{
    struct sensor mySensor;
 
    mySensor.deviceId = 944;
    mySensor.measurementType = 1;
    mySensor.value = 20.4;
}

void loop() {

}