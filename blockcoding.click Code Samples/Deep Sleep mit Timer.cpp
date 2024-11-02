#include <Arduino.h>

RTC_DATA_ATTR int test = 0;


void print_ESP32_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case 1  : Serial.println("ESP32 Deep Sleep Wakeup caused by external signal using RTC_IO"); break;
    case 2  : Serial.println("ESP32 Deep Sleep Wakeup caused by external signal using RTC_CNTL"); break;
    case 3  : Serial.println("ESP32 Deep Sleep Wakeup caused by touchpad"); break;
    case 4  : Serial.println("ESP32 Deep Sleep Wakeup caused by timer"); break;
    case 5  : Serial.println("ESP32 Deep Sleep Wakeup caused by ULP program"); break;
    default : Serial.println("Wakeup was NOT caused by ESP32 Deep Sleep"); break;
  }
}

void setup() {
  Serial.begin(115200);

  print_ESP32_wakeup_reason();

  esp_sleep_enable_timer_wakeup(10 * 1000 000);
  Serial.println("Set timer for ESP32 Deep Sleep to 10 seconds");

  //http://esp-idf.readthedocs.io/en/latest/api-reference/system/deep_sleep.html
  //esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  //Serial.println("Configured all RTC Peripherals to be powered down in sleep");
  
  Serial.println("ESP32 going to Deep Sleep now!");
  esp_deep_sleep_start();
}

void loop() {

}