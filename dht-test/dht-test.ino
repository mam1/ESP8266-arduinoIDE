

#include <DHT.h>
#include "DHT.h"

#define DHTPIN 7        // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)

// Initialize DHT sensor for normal 16mhz Arduino
DHT dht(DHTPIN, DHTTYPE, 15);

void setup() {
  //pinMode(DHTPIN, INPUT);
  Serial.begin(9600); 
  delay(100);
  Serial.printf("\n\n***********************************************************\n\n");
  Serial.printf("DHT22 test!\n");
  Serial.printf("calling dht.begin()");
  dht.begin();
}

void loop() {
  // Wait a few seconds between measurements.
  delay(200);
  Serial.printf(".");

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  // float h = dht.readHumidity();
  // // Read temperature as Celsius
  // float t = dht.readTemperature();
  // // Read temperature as Fahrenheit
  // float f = dht.readTemperature(true);
  
  // // Check if any reads failed and exit early (to try again).
  // if (isnan(h) || isnan(t) || isnan(f)) {
  //   Serial.println("Failed to read from DHT sensor!");
  //   return;
  // }

  // // Compute heat index
  // // Must send in temp in Fahrenheit!
  // float hi = dht.computeHeatIndex(f, h);

  // Serial.print("Humidity: "); 
  // Serial.print(h);
  // Serial.print(" %\t");
  // Serial.print("Temperature: "); 
  // Serial.print(t);
  // Serial.print(" *C ");
  // Serial.print(f);
  // Serial.print(" *F\t");
  // Serial.print("Heat index: ");
  // Serial.print(hi);
  // Serial.println(" *F");
}
