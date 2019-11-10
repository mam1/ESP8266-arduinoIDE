/*

  listens for a messages from the dryer that contain a humidity value
  accepts command to automatically turn a power relay off or on depending on the humidity value and the humidity limits
  accepts commands to raise or lower humidity limits
  accepts command to manually turn the relay off or on

  off  - turn relay off and swithch to manual mode
  on   - turn relay on and swithch to manual mode
  auto - automatically turn a power relay off or on depending on the humidity value and the humidity limits
  low xx.xx   - set low humidity limit
  high xx.xx  - set high humidity limit

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <NTPClient.h>
#include <time.h>
#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define ON      1
#define OFF     0
#define AUTO    2
#define MANUAL    1
#define TRUE    1
#define FALSE     -1

#define MAX_COMMAND_SIZE  100
#define CMD_TYPES     9

#define SCREEN_WIDTH 128                      // OLED display width, in pixels
#define SCREEN_HEIGHT 64                      // OLED display height, in pixels

#define SUB_TOPIC_1   "258Thomas/shop/dryer/sensor/humidity"
#define SUB_TOPIC_2   "258Thomas/shop/dryer/controller/commands"
#define PUB_TOPIC     "258Thomas/shop/dryer/controller"

#define MQTT_MESSAGE_SIZE   200               // max size of mqtt message

#define LOOP_DELAY          60000             // time between readings
#define HUMIDITY_HIGH_LIMIT 64                // turn dehumidifier on
#define HUMIDITY_LOW_LIMIT  61                // turn dehumidifier off
#define EST_OFFSET   -4                       // convert GMT to EST
#define NTP_OFFSET   60 * 60 * EST_OFFSET     // In seconds
#define NTP_INTERVAL 60 * 1000                // In milliseconds
#define NTP_ADDRESS  "us.pool.ntp.org"        // time server

#define OLED_RESET     -1         // Reset pin # (or -1 if sharing Arduino reset pin)

const char* ssid = "FrontierHSI";
const char* password = "";
const char* mqtt_server = "192.168.254.221";

const char* keyword[CMD_TYPES] = {
  /*  0 */    "temperature",
  /*  1 */    "humidity",
  /*  2 */    "on",
  /*  3 */    "off",
  /*  4 */    "low",
  /*  5 */    "high",
  /*  6 */    "status",
  /*  7 */    "????",
  /*  8 */    "auto"
};
char  command[MAX_COMMAND_SIZE + 1], *cmd_ptr;

// char *_low = "low";

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ESP8266 GPIO pins
static const uint8_t D0   = 16;   // blue led
static const uint8_t D1   = 5;    // SCL
static const uint8_t D2   = 4;    // SDA
static const uint8_t D3   = 0;    // Power relay 1
static const uint8_t D4   = 2;    // Power relay 2
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
char            msg[MQTT_MESSAGE_SIZE];
// char            msg2[MQTT_MESSAGE_SIZE];
// const char*     mptr;

long        lastMsg = 0;
int         value = 0;
// char         *message;
typedef struct {
  int       initialized;
  int       mode;         // 0-manual off, 1-manual on, 2-auto
  int       r1state;
  int       r2state;
  float     high;
  float     low;
} CONTROLBLOCK;

CONTROLBLOCK    persist;
CONTROLBLOCK    *cbptr;
int         control_block_size;

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
  timeClient.begin();                   // initialize time client
  client.setServer(mqtt_server, 1883);  // initialize MQTT broker
  client.setCallback(callback);         // set function that executes when a message is received
}

/* update OLED display */

void oled_update(void) {
  display.clearDisplay();
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0, 15);
  if (persist.r1state == OFF) display.println("off");
  else if (persist.r1state == ON) display.println("on");
  display.setCursor(0, 40);
  if (persist.mode == 2) display.println("auto");
  else display.println("manual");
  display.display();
}

/* Save structure to flash */
void save_controller_state(CONTROLBLOCK * ptr) {
  const byte* p = (const byte*)(const void*)ptr;
  int i;
  int ee = 0;
  for (i = 0; i < control_block_size; i++)
    EEPROM.write(ee++, *p++);
  EEPROM.commit();
  return;
}

/* Load structure from flash */
void load_contoller_state(CONTROLBLOCK * ptr) {
  byte* p = (byte*)(void*)ptr;
  int i;
  int ee = 0;
  for (i = 0; i < control_block_size; i++)
    *p++ = EEPROM.read(ee++);
  return;
}

/* turn off relay 1 */
void r1_off(void) {
  if (persist.r1state != OFF) {
    digitalWrite(D3, LOW);    // Turn the dehumidifier off
    digitalWrite(D0, HIGH);   // turn off the led
    persist.r1state = OFF;
    save_controller_state(&persist);
    Serial.println("dehumidifier turned off");
    oled_update();
  }
  return;
}

/* turn on relay 1 */
void r1_on(void) {
  if (persist.r1state != ON) {
    digitalWrite(D3, HIGH);    // turn the dehumidifier on
    digitalWrite(D0, LOW);     // turn on the led
    persist.r1state = ON;
    save_controller_state(&persist);
    Serial.println("dehumidifier turned on");
    oled_update();
  }
  return;
}

/* turn off relay 2 */
void r2_off(void) {
  if (persist.r2state != OFF) {
    digitalWrite(D4, LOW);    // Turn the humidifier off
    persist.r2state = OFF;
    save_controller_state(&persist);
    Serial.println("humidifier turned off");
    oled_update();
  }
  return;
}

/* turn on relay 2 */
void r2_on(void) {
  if (persist.r2state != ON) {
    digitalWrite(D4, HIGH);    // turn the humidifier on
    persist.r2state = ON;
    save_controller_state(&persist);
    Serial.println("humidifier turned on");
    oled_update();
  }
  return;
}
/* convert text to float */
float t_to_f(char *ptr) {
  int       i;
  String      convert = "";
  char            char1[8];
  float       converted;

  for (i = 0; i < 6; i++)
    convert += *ptr++;
  convert.toCharArray(char1, convert.length() + 1);
  converted = atof(char1);
  return converted;
}

int get_command_type(char *sptr) {
  char        *index;
  int         i;

  for (i = 0; i < CMD_TYPES; i++) {
    index = strstr(sptr, keyword[i]);
    if (index != 0) {
      return i;
    }
  }
  return -1;
}

float get_command_value(char* sptr, const char* testc) {
  char        *ptr;

  ptr = strstr(sptr, testc);
  ptr +=  strlen(testc);
  while ((*ptr == ' ') || (*ptr == ':')) ptr++;
printf("%s%2.2f\n", "value = ",t_to_f(ptr));
  return t_to_f(ptr);
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

      client.subscribe(SUB_TOPIC_1);
      Serial.print("listening for messages on topic ");
      Serial.println(SUB_TOPIC_1);
      client.subscribe(SUB_TOPIC_2);
      Serial.print("listening for messages on topic ");
      Serial.println(SUB_TOPIC_2);
      Serial.print("publishing to ");
      Serial.println(PUB_TOPIC);
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

void pub_ready() {
  unsigned long         epoch;
  char*                 c_time_string;
  time_t                unix_time;
  // char                  floatString[10];

  Serial.println("pub_ready called");

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
  memset(&msg[0], '\0', sizeof(msg));
  strcpy(msg, c_time_string);

  switch (persist.mode) {
  case 0:
    strcat(msg, ", manual control, ");
    break;
  case 1:
    strcat(msg, ", manual control, ");
    break;
  case 2:
    strcat(msg, ", automatic control, ");
    break;
  default:
    strcat(msg, "*** error - bad mode code ***, ");
    break;
  }
  if (persist.r1state) {
    strcat(msg, "dehumidifier on, ");
  }
  else {
    strcat(msg, "dehumidifier off, ");
  }

  // dtostrf(persist.high, 2, 2, floatString);
  // strcat(msg, floatString);
  // strcat(msg, " - ");
  // dtostrf(persist.low, 2, 2, floatString);
  // strcat(msg, floatString);

  strcat(msg, ", controller ready");

  Serial.println("publishing message");
  Serial.printf("** %s **\n", msg);
  client.publish(PUB_TOPIC, msg);
  oled_update();

  return;
}

void process_command(byte* payload) {
  char*       ptr;
  int         command_type;

  ptr = (char*)payload;
  command_type = get_command_type(ptr);
  Serial.printf("processing a %s command\n", keyword[command_type]);
  if (command_type == -1) {
    Serial.println("Unknown command type");
    return;
  }
  // Serial.printf("command type %i\n", command_type);
  switch (command_type) {
  case 0:  // temperature
    break;
  case 1: // humidity
    break;
  case 2: // on
    Serial.println("mode is manual force on");
    persist.mode = MANUAL;
    save_controller_state(&persist);
    r1_on();
    pub_ready();
    break;
  case 3: // off
    Serial.println("mode is manual force off");
    persist.mode = MANUAL;
    save_controller_state(&persist);
    r1_off();
    pub_ready();
    break;
  case 4: // low
    Serial.println("set low humidity limit");
    char    *tptr;
    tptr = (char*)payload;
    Serial.print("payload <");
    while (*tptr != '\0') Serial.print(*tptr++);
    Serial.println(">");
    Serial.printf("command value %2.2f\n", get_command_value((char *)payload, "low"));
    persist.low = get_command_value((char *)payload, "low");
    save_controller_state(&persist);
    pub_ready();
    break;
  case 5: // high
    Serial.println("set high humidity limit");
    persist.high = get_command_value((char *)payload, "high");
    save_controller_state(&persist);
    pub_ready();
    break;
  case 6:  // status
    Serial.print("system status - ");
    pub_ready();
    break;
  case 7:  // t_ high
    break;
  case 8: // auto
    Serial.println("mode is auto");
    persist.mode = AUTO;
    save_controller_state(&persist);

    pub_ready();
    break;
  default:
    Serial.println("command type table is screwed up");
    break;
  }
  return;
}

void process_humidity_reading(byte* payload) {
  char*       ptr;
  int         command_type;
  float       humid;

  ptr = (char*)payload;
  command_type = get_command_type(ptr);
  if (command_type == -1) {
    Serial.println("Unknown command type");
    return;
  }
printf(" ##processing humidity reading command type is %i\n",command_type);
  if (command_type == 1) 
  {
    humid = get_command_value(ptr, "humidity");
printf(" processing humidity value is %2.2f\n", humid);
    Serial.printf(">>>>>> processing a humidity reading of %2.2f, %2.2f - %2.2f, mode = %i\n",
                  humid, persist.high, persist.low, persist.mode);
printf("  &&& before r1 state = %i, r2 state = %i\n", persist.r1state, persist.r2state);
    if (persist.mode == 2) {
      // set persist.state based on humidity value

      // r1_on();
      // r2_on();

      if (humid >= persist.high){
        r1_on();
        r2_off();
      } 
      else if (humid <= persist.low){
        r1_off();
        r2_on();
      } 
      else{
        r1_off();
        r2_off();
      }
printf("  &&& after r1 state = %i, r2 state = %i\n", persist.r1state, persist.r2state);
      printf("  &&& r1 state = %i, r2 state = %i\n", persist.r1state, persist.r2state);
      pub_ready();
      return;
    }
  }
  return;
}

/* called when a message is received */
void callback(char* topic, byte* payload, unsigned int length) {
  int       i;

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (i = 0; i < (int)length; i++) {
    Serial.print((char)payload[i]);
  }
  payload[i] = '\0';
  Serial.println();
  if (strcmp(topic, SUB_TOPIC_1) != 0) {
    process_command(payload);
  }
  else if (strcmp(topic, SUB_TOPIC_2) != 0) {
    process_humidity_reading(payload);
  }
  return;
}

void setup() {

  Serial.begin(115200);                 // start the serial interface
  control_block_size = sizeof(persist);
  EEPROM.begin(control_block_size);     // define some flash memory
  load_contoller_state(&persist);       // load control block from flash
  persist.initialized = FALSE;
  if (persist.initialized != TRUE) {    // initialize control block if necessary
    persist.initialized = TRUE;
    persist.mode = AUTO;
    persist.r1state = OFF;
    persist.r2state = OFF;
    persist.high = HUMIDITY_HIGH_LIMIT;
    persist.low = HUMIDITY_LOW_LIMIT;
    save_controller_state(&persist);    // save control block
  }
  load_contoller_state(&persist);

  pinMode(D0, OUTPUT);                  // use D0 to control builtin LED
  pinMode(D3, OUTPUT);                  // use D3  to control power relay

  if (persist.r1state == ON) r1_on();
  else if(persist.r1state == OFF) r1_off();
  else Serial.println("***** bad r1 state code\n");

  if (persist.r2state == ON) r1_on();
  else if(persist.r2state == OFF) r1_off();
  else Serial.println("***** bad r2 state code\n");

  setup_wifi();                         // connect to wifi
  timeClient.begin();                   // initialize time client
  client.setServer(mqtt_server, 1883);  // initialize MQTT broker
  client.setCallback(callback);         // set function that executes when a message is received

// SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  oled_update();
}

void loop() {

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
    pub_ready();
  }
}

