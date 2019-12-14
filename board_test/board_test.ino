/*


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

int     cnt = 0;

void setup() {

  Serial.begin(115200);                 // start the serial interface
  delay(500);
  printf("%s\n","\n\nflash leds and pulse relay pins\n" );

// setup GPIO pins
  printf("%s\n", "initialize gpios");
  pinMode(RED_LED_pin, OUTPUT);         // use to control red LED
  pinMode(BLUE_LED_pin, OUTPUT);        // use to control blue led
  pinMode(RELAY_1_pin, OUTPUT);         // use to control power relay #1
  pinMode(RELAY_2_pin, OUTPUT);         // use to control power relay #2
  printf("%s\n", "pin mode set" );
  return;
}

// heart beat loop
void loop() {
  cnt++;
  printf("%s %i\n", "looping", cnt);
  digitalWrite(RELAY_1_pin, HIGH);
  digitalWrite(RELAY_2_pin, HIGH);
  digitalWrite(RED_LED_pin, HIGH);
  digitalWrite(BLUE_LED_pin, LOW);
  delay(1000 );
  digitalWrite(RELAY_1_pin, LOW);
  digitalWrite(RELAY_2_pin, LOW);
  digitalWrite(RED_LED_pin, LOW);
  digitalWrite(BLUE_LED_pin, HIGH);
  delay(1000 );
}


