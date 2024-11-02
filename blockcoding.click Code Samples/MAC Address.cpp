#include <Arduino.h>

void setup() {

    Serial.begin(115200);
    

    Serial.print("ESP32 Serial: ");
    char MAC_address[12];
    snprintf(MAC_address, 12, "%llX", ESP.getEfuseMac());
    Serial.println(MAC_address); 
}
 
void loop() {
    // put your main code here, to run repeatedly:   
}