#include <Arduino.h>
#include <TelnetStream2.h>
#include <WiFi.h>

const char* ssid = "Patricks Network";
const char* key = "wlan@pati";

TaskHandle_t TaskHandler_TelnetStream = NULL;

void task_function_TelnetStream(void* parameter) {
  TelnetStream2.begin();
  TaskHandler_TelnetStream = NULL;
  vTaskDelete(NULL);
}

void log() {
  static int i = 0;
  TelnetStream2.print(i++);
  TelnetStream2.print(" ");
  TelnetStream2.print("Uptime: ");
  TelnetStream2.println(millis());
}

void setup() {
  Serial.begin(115200);

  Serial.print("Connecting to WLAN Network: Patricks Network");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, key);
  while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
  }
  Serial.println("\nWLAN connected!");
  Serial.print("Your IP address: ");
  Serial.println(WiFi.localIP());

  xTaskCreatePinnedToCore(task_function_TelnetStream, "Task_TelnetStream", 10000, NULL, 1, &TaskHandler_TelnetStream, 0);
  
}

void loop() {

  char TelnetRead = TelnetStream2.read();
  if (TelnetRead == 'H')
    TelnetStream2.println("Satz mit X");
  
  /*switch () {
    case 'R':
    TelnetStream2.stop();
    delay(100);
    ESP.restart();
      break;
    case 'C':
      TelnetStream2.println("bye bye");
      TelnetStream2.flush();
      TelnetStream2.stop();
      break;
    case 'H':
      TelnetStream2.println("Hallooo du!");
      break;
    case 'X':
      TelnetStream2.println("Satz mit X");
      break;
  }*/

  static unsigned long next = millis();
  if (millis() > next) {
    next += 5000;
    log();
  }
}