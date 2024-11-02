#include <Arduino.h>

#include <IRremote.hpp>
 
int IR_RECEIVE_PIN = 34;
 
void setup()
{
  Serial.begin(115200);
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK); // Start the receiver
}

void loop()
{
  if (IrReceiver.decode()) {
      Serial.println("Raw Data:");
      Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX); // Print "old" raw data
      Serial.println("Raw Protocol:");
      Serial.println(IrReceiver.decodedIRData.protocol, HEX); // Print "old" raw data
      Serial.println();
      
      // USE NEW 3.x FUNCTIONS
      Serial.println("Results short:");
      IrReceiver.printIRResultShort(&Serial); // Print complete received data in one line
      Serial.println("Statements to send:");
      IrReceiver.printIRSendUsage(&Serial);   // Print the statement required to send this data
      Serial.println();
      Serial.println();

      IrReceiver.resume(); // Enable receiving of the next value
  }
}