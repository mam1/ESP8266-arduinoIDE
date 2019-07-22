 // Configure the framework
#include "bconf/MCU_ESP8266.h"              // Load the code directly on the ESP8266
#include "conf/Gateway.h"                   // The main node is the Gateway, we have just one node
#include "conf/DynamicAddressing.h"         // Use dynamic addressing

void setup() {
  // put your setup code here, to run once:

  Serial.begin(57600);
  delay(1000);
  Serial.printf("\n\n**************************************************\n");
  Serial.println("");
  Serial.println("");
}

void loop() {
  // put your main code here, to run repeatedly:
  // Set Baud rate to 57600


  // Get current baud rate
  int br = Serial.baudRate();

  // Will print "Serial is 57600 bps"
  Serial.printf("Serial is %d bps\n", br);
  delay(2000);
}
