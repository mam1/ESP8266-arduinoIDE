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

#define SUB_TOPIC             "258Thomas/shop/sensor/dryer/humidity"
#define PUB_TOPIC             "258Thomas/shope/controller/dryer/dehumidifier"

#define MQTT_MESSAGE_SIZE   100               // max size of mqtt message
#define LOOP_DELAY          60000             // time between readings
#define HUMIDITY_HIGH_LIMIT 59                // turn dihumidifier on
#define HUMIDITY_LOW_LIMIT  58                // tune dihumidifier off

#define LED_PIN              0                // Led in NodeMCU at pin GPIO16 (D0)

#define DHTPIN               13               // pin the DHT22 is connected to
#define DHTTYPE           DHT22               // DHT 22  (AM2302)
#define DHT22_OFFSET         -6               // correction added to DHT22 huniudity reading

#define EST_OFFSET   -4                       // convert GMT to EST
#define NTP_OFFSET   60 * 60 * EST_OFFSET     // In seconds
#define NTP_INTERVAL 60 * 1000                // In milliseconds
#define NTP_ADDRESS  "us.pool.ntp.org"        // time server

const char* ssid = "FrontierHSI";
const char* password = "";
const char* mqtt_server = "192.168.254.221";

// ESP8266 GPIO pins 
static const uint8_t D0   = 16;
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
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
DHT dht(DHTPIN, DHTTYPE, 15);
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

long        lastMsg = 0;
char        msg[MQTT_MESSAGE_SIZE];
int         value = 0;
int         dehumidifer_state = 0;

/* setu a wifi connection */
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

  char hstring[] = "humidity";
  String          convert;
  char char1[8];
  float humid;

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // search for key string
  char *ptr = strstr((char*)payload, hstring);
  if (ptr != NULL) /* Substring found */
  {
    // convert humidity text value from the mqtt message to float
    convert = "";
    ptr += strlen(hstring) + 2;
    for (int i = 0; i < 6; i++)
      convert += *ptr++;
    convert.toCharArray(char1, convert.length() + 1);
    humid = atof(char1);
    Serial.println(humid);

    // set dehumidifer_state based on received humidity value
    if (humid > HUMIDITY_HIGH_LIMIT)
      if (!dehumidifer_state)  dehumidifer_state = 1;
      else if (humid < HUMIDITY_LOW_LIMIT)
        if (dehumidifer_state) dehumidifer_state = 0;
  }
  return;
}

/* connect ti a MQTT briker */
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
      Serial.print("listening for messages on topic {");
      Serial.print(SUB_TOPIC);
      Serial.println("}");
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
  Serial.begin(115200);                 // start the serial interface
  setup_wifi();                         // connect to wifi
  timeClient.begin();                   // initialize time client
  dht.begin();                          // initialize the DHT22 sensor
  client.setServer(mqtt_server, 1883);  // initialize MQTT broker
  client.setCallback(callback);         // set function that executes when a message is received
  Serial.println();

}

void loop() {

  // String                tt;
  // String                th;
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
    } else {
      digitalWrite(D0, HIGH);  // Turn the LED off by making the voltage HIGH
    }

    controller_ready_message  = c_time_string;
    if (dehumidifer_state)
      controller_ready_message += " dehumidifier on ";
    else
      controller_ready_message += " dehumidifier off ";
    controller_ready_message += "controller ready";

    // publish ready message
    // con_topic = String(PARENT_TOPIC + String("/") + TYPE_TOPIC + String("/") + LOCATION_TOPIC + String("/") + CONTROLER_TOPIC);
    con_topic = String(PUB_TOPIC);
    client.publish(con_topic.c_str(), controller_ready_message.c_str());
  }
}

