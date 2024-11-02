#include <Arduino.h>

TaskHandle_t myTaskHandle = NULL;
TaskHandle_t myTaskHandle2 = NULL;
QueueHandle_t queue;
SemaphoreHandle_t queue_print_messages = xSemaphoreCreateMutex();

void Demo_Task(void *arg)
{
    String hallo = "Heeee des is da String";
    char txQueue[50] = "Text ";
    //"teeest".toCharArray(txQueue, 50);
    
    //printf("Set txQueue to= %s\n", txQueue);

    for(int i=0; i<10; i++){
      sprintf(txQueue, "%s%d", txQueue, i);
      while(!xQueueSendToBack(queue, (void*)&txQueue, (TickType_t)0)){
        delay(5);
      }
      xSemaphoreTake(queue_print_messages, portMAX_DELAY);
      printf("Sent txQueue to queue= %s\n", txQueue);
      xSemaphoreGive(queue_print_messages);
    }

    /*while(1){
        vTaskDelay(1000/ portTICK_RATE_MS);
    }*/
    vTaskDelete(NULL);
}

void Demo_Task2(void *arg)
{
    String hallo = "";
    
    while(1){
      char rxQueue[50];  
     if( xQueueReceive(queue, &(rxQueue), (TickType_t)5))
     {
      hallo = rxQueue;
      xSemaphoreTake(queue_print_messages, portMAX_DELAY);
      Serial.print("Received als String: ");
      Serial.println(hallo);
      xSemaphoreGive(queue_print_messages);

      //vTaskDelay(1000/ portTICK_RATE_MS);
     }
     
    }
}

void setup()
{
  Serial.begin(115200);

  if (!(queue = xQueueCreate(10, sizeof(char)*50)))
    Serial.println("Failed to create queue !");

  xTaskCreate(Demo_Task, "Demo_Task", 4096, NULL, 10, &myTaskHandle);
  delay(5000);
  xTaskCreatePinnedToCore(Demo_Task2, "Demo_Task2", 4096, NULL,10, &myTaskHandle2, 0);
}

void loop() {

}