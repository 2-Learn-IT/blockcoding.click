#include <Arduino.h>

//#include <Arduino_FreeRTOS.h>
//#include <semphr.h>
//create handle for the mutex. It will be used to reference mutex
SemaphoreHandle_t xMutex = xSemaphoreCreateMutex();
// create mutex and assign it a already create handler 

String printstr = "Print Task: Diesen String ausgeben!";
int i = 0;

// this printer task send data to arduino serial monitor
//aslo it is shared resource between both instances of the tasks
void printer()
{
  // take mutex
  Serial.println (i);
  i += 1;
  
  xSemaphoreTake(xMutex, portMAX_DELAY);
  Serial.print("Das"); // send string to serial monitor
  Serial.print("ist");
  Serial.print("der");
  Serial.print("Text");
  Serial.print("zum");
  Serial.print("Ausgeben");
  Serial.println("!");
  xSemaphoreGive(xMutex); // release mutex
}

// this is a definition of tasks 
void OutputTask(void* parameter)
{
  while(1)
  {
    printer();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void setup()
{
  // Enable serial module of Arduino with 9600 baud rate
  Serial.begin(115200);

// create two instances of task "OutputTask" which are used to display string on 
// arduino serial monitor. We passed strings as a paramter to these tasks such as ""Task 1 //#####################Task1" and "Task 2 ---------------------Task2". Priority of one //instance is higher than the other
  
  xTaskCreatePinnedToCore(OutputTask,"Printer Task 1", 10000,NULL,1,NULL, 0);
  xTaskCreatePinnedToCore(OutputTask,"Printer Task 2", 10000,NULL,2,NULL, 1);    
}


void loop(){}