#include <Arduino.h>

struct WaterPump {
  String name = "";
  int PIN = 0;
  bool highTrigger = true;
  int ms_pumping_1_ml = 0;
};
struct WaterPump WaterPump25;

void Water_Pump_calibrate(WaterPump *pump, int calibration_ms) {
  Serial.println("Starting " + pump->name + " calibration in 10 seconds by PUMPING WATER!...\nMEASURING CUP is needed for calibration!\nFirst calibration cycle will PUMP FOR " + String((float)calibration_ms/1000) + " SECONDS");
  delay(10000);
  Serial.println("STARTING " + pump->name + " for " + String((float)calibration_ms/1000) + " SECONDS now...");
  if(pump->highTrigger == true){
    digitalWrite(pump->PIN, HIGH);
    delay(calibration_ms);
    digitalWrite(pump->PIN, LOW);
  }
  else {
    digitalWrite(pump->PIN, LOW);
    delay(calibration_ms);
    digitalWrite(pump->PIN, HIGH);
  }
  Serial.println("NOW MEASURE the MILLILITERS which have been pumped and write it down!\nCalibration DONE!");
}

void Water_Pump_start (WaterPump *pump, int pump_for_ms, int extra_ms_for_start) {
  Serial.println("Starting pump for " + String((float)(pump_for_ms + extra_ms_for_start)/1000) + " seconds!");
  if(WaterPump25.highTrigger == true) {
    digitalWrite(25, HIGH);
    delay (pump_for_ms + extra_ms_for_start);
    digitalWrite(25, LOW);
  }
  else {
    digitalWrite(25, LOW);
    delay (pump_for_ms + extra_ms_for_start);
    digitalWrite(25, HIGH);
  } 
}

void setup() {
  Serial.begin(115200);
  pinMode(WaterPump25.PIN, OUTPUT);
  WaterPump25.PIN = 25;
  WaterPump25.highTrigger = true;
  WaterPump25.name = "Chili";
  if(WaterPump25.highTrigger == true)
    digitalWrite(25, LOW);
  else
    digitalWrite(25, HIGH);
  Water_Pump_calibrate(&WaterPump25, 5000, 20000);
}

void loop() {
  int measured_ms = 5000, measured_ml = 100;
  
  WaterPump25.ms_pumping_1_ml = measured_ms / measured_ml;
  Water_Pump_start(&WaterPump25, 100 * WaterPump25.ms_pumping_1_ml, 1000);

  delay(20000);

}