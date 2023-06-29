#include <ESPmDNS.h>
#include <ArduinoOTA.h>

TaskHandle_t TaskHandler_OTA = NULL;

void task_function_OTA(void* parameter) {
  while(true) {
    ArduinoOTA.handle();
    delay(3500);
  }
  TaskHandler_OTA = NULL;
  vTaskDelete(NULL);
}

void setup_OTA(const char* hostname) {
  // Configure the hostname
  ArduinoOTA.setHostname(hostname);

  // Port defaults to 3232
  // ArduinoOTA.setPort(3232); // Use 8266 port if you are working in Sloeber IDE, it is fixed there and not adjustable

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
	String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start uploading " + type);
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("\nFinished upload!");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("\nAuth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("\nBegin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("\nConnect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("\nReceive Failed");
    else if (error == OTA_END_ERROR) Serial.println("\nEnd Failed");
  });

  ArduinoOTA.begin();
  xTaskCreatePinnedToCore(task_function_OTA, "Task_OTA", 10000, NULL, 1, &TaskHandler_OTA, 0);
  Serial.println("OverTheAir Upload Process Initialized!");
}