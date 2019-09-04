/*
  
  read DHT22 sensor and report temperature and humidity

*/

#include <stdio.h>
#include <string.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <time.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHTesp.h"

#define SCREEN_WIDTH          128   // OLED display width, in pixels
#define SCREEN_HEIGHT         64    // OLED display height, in pixels
#define SUB_TOPIC             "258Thomas/house/office/sensor/commands"
#define PUB_TOPIC_H           "258Thomas/house/office/sensor/humidity"
#define PUB_TOPIC_T           "258Thomas/house/office/sensor/temperature"
#define MQTT_MESSAGE_SIZE     100    // max size of mqtt message
#define LOOP_DELAY            60000  // time between readings
#define HUMIDITY_HIGH_LIMIT   25     // turn dehumidifier on
#define HUMIDITY_LOW_LIMIT    20     // tune dehumidifier off
#define EST_OFFSET            -4     // convert GMT to EST
#define NTP_OFFSET   60 * 60 * EST_OFFSET     // In seconds
#define NTP_INTERVAL 60 * 1000                // In milliseconds
#define NTP_ADDRESS  "us.pool.ntp.org"        // time server

const char* ssid = "FrontierHSI";
const char* password = "";
const char* mqtt_server = "192.168.254.221";

#define OLED_RESET     -1         // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ESP8266 GPIO pins 
static const uint8_t D0   = 16;   // blue led
static const uint8_t D1   = 5;    // SCL
static const uint8_t D2   = 4;    // SDA
static const uint8_t D3   = 0;    // Power relay
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;

WiFiUDP ntpUDP;
WiFiClient espClient;
PubSubClient client(espClient);
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);
DHTesp dht;

long        lastMsg = 0;
char        msg[MQTT_MESSAGE_SIZE];
int         value = 0;
int         dehumidifer_state = 0;

/* setup a wifi connection */
void setup_wifi() {
  delay(10);
  // Connect to WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

/* called when a message is recieved */
void callback(char* topic, byte* payload, unsigned int length) {

  char hstring[] = "offset";
  String          convert;
  char char1[8];
  float offset;

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < (int)length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // search for offset string
  char *ptr = strstr((char*)payload, hstring);
  if (ptr != NULL) /* Substring found */
  {
    // convert  text value from the mqtt message to float
    convert = "";
    ptr += strlen(hstring) + 2;
    for (int i = 0; i < 6; i++)
      convert += *ptr++;
    convert.toCharArray(char1, convert.length() + 1);
    offset = atof(char1);
    Serial.println(offset);
  }
  return;
}

/* connect to a MQTT broker */
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(SUB_TOPIC);
      Serial.print("listening for messages on topic ");
      Serial.println(SUB_TOPIC);
      Serial.printf("Publishing readings to: %s, %s\n",PUB_TOPIC_H, PUB_TOPIC_T);
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 10 seconds");
      // Wait 10 seconds before retrying
      delay(10000);
    }
  }
}

void setup() {

  pinMode(D0, OUTPUT);                  // initialize the BUILTIN_LED pin as an output
  pinMode(D3, OUTPUT);                  // use D3  to control power relay 
  Serial.begin(115200);                 // start the serial interface
  setup_wifi();                         // connect to wifi
  timeClient.begin();                   // initialize time client
  client.setServer(mqtt_server, 1883);  // initialize MQTT broker
  client.setCallback(callback);         // set function that executes when a message is received
  dht.setup(13, DHTesp::DHT22);         // Connect DHT sensor to GPIO 17
  Serial.println();

 // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.display();  //display Adafruit logo
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
}

void loop() {

  float       humidity;
  float       temperature;
  String                controller_ready_message;
  String                con_topic;
  unsigned long         epoch;
  char*                 c_time_string;
  time_t                unix_time;

  // update the time client
  timeClient.update();

  // reconnect if necessary
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  long now = millis();
  if (now - lastMsg > LOOP_DELAY) {
    lastMsg = now;
    ++value;

    // get the time
    epoch =  timeClient.getEpochTime();
    unix_time = static_cast<time_t>(epoch);
    c_time_string = ctime(&unix_time);

    // remove carriage return
    char *src, *dst;
    for (src = dst = c_time_string; *src != '\0'; src++) {
      *dst = *src;
      if (*dst != '\n') dst++;
    }
    *dst = '\0';

    humidity = dht.getHumidity();
    temperature = dht.getTemperature();
    temperature = dht.toFahrenheit(temperature);
    Serial.printf("%s humidity %2.2f, temperature %2.2f\n", c_time_string, humidity, temperature);

    //update oled display
    display.clearDisplay();
    display.setTextSize(2);             
    display.setTextColor(WHITE);        // Draw white text
    display.setCursor(5,15);             
    display.print(" T = ");
    display.println(temperature);
    display.setCursor(5,40); 
    display.print(" H = ");
    display.println(humidity);
    display.display(); 


    // publish the readings
    snprintf (msg, MQTT_MESSAGE_SIZE, "%s temperature: %2.2f", c_time_string,temperature);
    client.publish(PUB_TOPIC_T, msg);
    snprintf (msg, MQTT_MESSAGE_SIZE, "%s humidity: %2.2f", c_time_string,humidity);
    client.publish(PUB_TOPIC_H, msg);
    Serial.printf("    published sensor readings to %s and %s\n", PUB_TOPIC_T, PUB_TOPIC_H);

  }
}

