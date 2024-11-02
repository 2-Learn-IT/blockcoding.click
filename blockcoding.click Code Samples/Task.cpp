#include <Arduino.h>

int globalIntVar = 5;

void globalIntTask(void* parameter){
 
    Serial.print("globalIntTask: ");
    Serial.println(*((int*)parameter));            
 
    vTaskDelete( NULL );
 
}
 
void localIntTask(void* parameter){
 
    int localIntVar = *((int*)parameter);
    Serial.print("localIntTask: ");
    Serial.println(*((int*)parameter));  
    Serial.print("localIntTask Variable: ");
    Serial.println(localIntVar);
 
    vTaskDelete( NULL );
 
}

void setup() {
 
  Serial.begin(115200);
  delay(1000);
 
  xTaskCreate(globalIntTask,             /* Task function. */
                    "globalIntTask",           /* String with name of task. */
                    10000,                     /* Stack size in words. */
                    (void*)&globalIntVar,      /* Parameter passed as input of the task */
                    1,                         /* Priority of the task. */
                    NULL);                     /* Task handle. */
 
  int localIntVar = 9;
 
  xTaskCreate(localIntTask,              /* Task function. */
                    "localIntTask",            /* String with name of task. */
                    10000,                     /* Stack size in words. */
                    (void*)&localIntVar,       /* Parameter passed as input of the task */
                    1,                         /* Priority of the task. */
                    NULL);                     /* Task handle. */
 
  delay(1000)
}
 
void loop() {
  delay(1000);
}
 
