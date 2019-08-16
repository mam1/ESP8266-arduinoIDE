/*
  control a 120 volt dehumidifier

*/

#include <stdio.h>
#include <string.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <NTPClient.h>
#include <time.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128                      // OLED display width, in pixels
#define SCREEN_HEIGHT 64                      // OLED display height, in pixels
#define SUB_TOPIC             "258Thomas/shop/dryer/#"
#define PUB_TOPIC             "258Thomas/shop/dryer/controller/dehumidifier"
#define MQTT_MESSAGE_SIZE   100               // max size of mqtt message
#define LOOP_DELAY          60000             // time between readings
#define HUMIDITY_HIGH_LIMIT 70                // turn dehumidifier on
#define HUMIDITY_LOW_LIMIT  55                // tune dehumidifier off
#define EST_OFFSET   -4                       // convert GMT to EST
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

long        lastMsg = 0;
char        msg[MQTT_MESSAGE_SIZE];
int         value = 0;
int         dehumidifer_state = 0;
float low_humid = HUMIDITY_LOW_LIMIT;
float high_humid = HUMIDITY_HIGH_LIMIT;

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

/* called when a message is received */
void callback(char* topic, byte* payload, unsigned int length) {

  char          t_humid[] = "humidity";
  char          t_high[] = "high";
  char          t_low[] = "low";
  String        convert;
  char          char1[8];
  float         humid;
  char          *ptr;

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

 // search for key strings
  ptr = strstr((char*)payload, t_humid);
  if (ptr != NULL) {
    // convert humidity text value from the mqtt message to float
    convert = "";
    ptr += strlen(t_humid) + 2;
    for (int i = 0; i < 6; i++)
      convert += *ptr++;
    convert.toCharArray(char1, convert.length() + 1);
    humid = atof(char1);
    Serial.println(humid);
    display.printf("humidity %2.1f\n");
  } 
  else ptr = strstr((char*)payload, t_high);
  if (ptr != NULL) {
    // convert high text value from the mqtt message to float
    convert = "";
    ptr += strlen(t_high) + 2;
    for (int i = 0; i < 6; i++)
      convert += *ptr++;
    convert.toCharArray(char1, convert.length() + 1);
    high_humid = atof(char1);
    Serial.println(high_humid);
  }

  else ptr = strstr((char*)payload, t_low);
  if (ptr != NULL) {
    // convert humidity low value from the mqtt message to float
    convert = "";
    ptr += strlen(t_low) + 2;
    for (int i = 0; i < 6; i++)
      convert += *ptr++;
    convert.toCharArray(char1, convert.length() + 1);
    low_humid = atof(char1);
  
    // set dehumidifer_state based on received humidity value
    if (humid > high_humid)
      if (!dehumidifer_state)  dehumidifer_state = 1;
      else if (humid < low_humid)
        if (dehumidifer_state) dehumidifer_state = 0;
  }
  return;
}

/* connect ti a MQTT broker */
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
  Serial.println();

 // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.display();                  // display buffer Adafruit logo
  delay(2000);                        // Pause for 2 seconds
  display.clearDisplay();             // Clear the buffer
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
}

void loop() {

  String                controller_ready_message;
  String                con_topic;

  // float                 temperature;
  float                 humidity;
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

    // Switch on the LED if dehumidifier is on
    if (dehumidifer_state) {
      digitalWrite(D0, LOW);   // Turn the LED on (Note that LOW is the voltage level)
      digitalWrite(D3, HIGH);  // Turn the dehumidifier on 
    } 
    else {
      digitalWrite(D0, HIGH);   // Turn the LED off by making the voltage HIGH
      digitalWrite(D3, LOW);    // Turn the dehumidifier off by making the voltage Low
    }

    //update oled display
    display.clearDisplay();
    display.setCursor(0,0);             // Start at top-left corner
    display.println(SUB_TOPIC);
    display.println(" ");
    controller_ready_message  = c_time_string;  // load time stamp

    if (dehumidifer_state){
      controller_ready_message += " dehumidifier on ";
      display.println("dehumidifier on");
    }
    else{
      controller_ready_message += " dehumidifier off ";
      display.println("dehumidifier off");

    }
    display.printf("\n%2.1f - %2.1f\n", high_humid, low_humid);

    controller_ready_message += high_humid;
    controller_ready_message += " - ";
    controller_ready_message += low_humid;
    controller_ready_message += " controller ready";

    display.display();    // update oled display

    // publish ready message
    con_topic = String(PUB_TOPIC);
    client.publish(con_topic.c_str(), controller_ready_message.c_str());
  }
}

