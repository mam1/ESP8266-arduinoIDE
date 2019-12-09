/*

  listens for a messages from the dryer that contain a humidity value.  When a message is received relay states are adjusted

  off  - turn relay off and switch to manual mode
  on   - turn relay on and switch to manual mode
  auto - automatically turn a power relay off or on depending on the humidity value and the humidity limits
  low xx.xx   - set low humidity limit
  high xx.xx  - set high humidity limit

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <Wire.h>

static const uint8_t BLUE_LED_pin = 2;  // blue led
static const uint8_t RED_LED_pin = 0;   // red led
static const uint8_t RELAY_1_pin = 12;  // Power relay 1   
static const uint8_t RELAY_2_pin = 13;  // Power relay 2

void setup() {

  Serial.begin(115200);                 // start the serial interface
  delay(1000);
printf("%s\n", "\n\ninitialize gpios");

// setup GPIO pins
  pinMode(RED_LED_pin, OUTPUT);         // use to control red LED
  pinMode(BLUE_LED_pin, OUTPUT);        // use to control blue led
  pinMode(RELAY_1_pin, OUTPUT);         // use to control power relay #1       
  pinMode(RELAY_2_pin, OUTPUT);         // use to control power relay #1  
  return;         
}

// heart beat loop
void loop() {
  printf("%s\n","looping" );
  digitalWrite(RELAY_1_pin, HIGH);    // turn the humidifier on
  digitalWrite(RELAY_2_pin, HIGH);    // turn the humidifier on
  digitalWrite(RED_LED_pin, HIGH);    // turn the humidifier on
  digitalWrite(BLUE_LED_pin, LOW);    // turn the humidifier on
  delay(1000 );
  digitalWrite(RELAY_1_pin, LOW);    // turn the humidifier on
  digitalWrite(RELAY_2_pin, LOW);    // turn the humidifier on
  digitalWrite(RED_LED_pin, LOW);    // turn the humidifier on
  digitalWrite(BLUE_LED_pin, HIGH);    // turn the humidifier on
  delay(1000 );
}
  

