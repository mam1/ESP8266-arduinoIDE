/*

  listens for a messages from the dryer that contain a humidity value
  accepts command to automatically turn a power relay off or on depending on the humidity value and the humidity limits
  accepts commands to raise or lower humidity limits
  accepts command to manually turn the relay off or on

  off  - turn relay off and swithch to manual mode
  on   - turn relay on and swithch to manual mode
  auto - automatically turn a power relay off or on depending on the humidity value and the humidity limits
  h_low xx.xx   - set low humidity limit
  h_high xx.xx  - set high humidity limit

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

#define ON 			1
#define OFF 		0
#define AUTO 		2
#define MANUAL		1
#define TRUE 		1
#define FALSE 		-1

#define MAX_COMMAND_SIZE 	100
#define CMD_TYPES			9

#define SCREEN_WIDTH 128                      // OLED display width, in pixels
#define SCREEN_HEIGHT 64                      // OLED display height, in pixels

#define SUB_TOPIC             "258Thomas/shop/dryer/sensor/#"
#define PUB_TOPIC             "258Thomas/shop/dryer/controller"
#define MQTT_MESSAGE_SIZE   100               // max size of mqtt message

#define LOOP_DELAY          60000             // time between readings
#define HUMIDITY_HIGH_LIMIT 64                // turn dehumidifier on
#define HUMIDITY_LOW_LIMIT  61                // turn dehumidifier off
#define EST_OFFSET   -4                       // convert GMT to EST
#define NTP_OFFSET   60 * 60 * EST_OFFSET     // In seconds
#define NTP_INTERVAL 60 * 1000                // In milliseconds
#define NTP_ADDRESS  "us.pool.ntp.org"        // time server
#define CONTROLBLOCK_SIZE 

char 				*message;
typedef struct {
	int 			initialized;
	int 			mode;
	int 			state;
	float			high;
	float			low;
} CONTROLBLOCK;

CONTROLBLOCK 		persist;
CONTROLBLOCK 		*cbptr;
int 				control_block_size;

void save_controller_state(CONTROLBLOCK * ptr) {

	const byte* p = (const byte*)(const void*)ptr;
	int i; 
	int ee = 0;
	for (i = 0; i < control_block_size; i++)
		EEPROM.write(ee++, *p++);
	EEPROM.commit();
	return;
}

void get_contoller_state(CONTROLBLOCK * ptr) {	

	byte* p = (byte*)(void*)ptr;
	int i;
	int ee = 0;
	for (i = 0; i < control_block_size; i++)
		*p++ = EEPROM.read(ee++);
	return;
}
/* convert text to float */
float t_to_f(char *ptr) {
	int 			i;
	String 			convert = "";
	char          	char1[8];
	float 			converted;

	for (int i = 0; i < 6; i++)
		convert += *ptr++;
	convert.toCharArray(char1, convert.length() + 1);
	converted = atof(char1);
	return converted;
}

int get_command_type(char *sptr) {
	char  			*index;
	int 			i;

	/* command list */
	char 	command[MAX_COMMAND_SIZE + 1], *cmd_ptr;
	char    *keyword[CMD_TYPES] = {
		/*  0 */    "temperature",
		/*  1 */    "humidity",
		/*  2 */    "on",
		/*  3 */    "off",
		/*  4 */    "h_low",
		/*  5 */    "h_high",
		/*  6 */    "t_low",
		/*  7 */    "t_high",
		/*  8 */	"auto"
	};

	for (i = 0; i < CMD_TYPES; i++) {
		index = strstr(sptr, keyword[i]);
		if (index != NULL)
			return i;
	}
	return -1;
}

void setup() {

	Serial.begin(115200);                	// start the serial interface
	control_block_size = sizeof(persist);
	EEPROM.begin(control_block_size); 		// grab some flash memory
		Serial.printf("initialized %i, mode %i, state %i, range %2.2f-%2.2f\n", persist.initialized, persist.mode, persist.state, persist.high, persist.low);
	get_contoller_state(&persist);
	if (persist.initialized != TRUE) {
		persist.initialized = TRUE;
		persist.mode = MANUAL;
		persist.state = OFF;
		persist.high = HUMIDITY_HIGH_LIMIT;
		persist.low = HUMIDITY_LOW_LIMIT;
		save_controller_state(&persist);
	}
	get_contoller_state(&persist);

	Serial.printf("initialized %i, mode %i, state %i, range %2.2f-%2.2f\n", persist.initialized, persist.mode, persist.state, persist.high, persist.low);

	delay(1000);
	Serial.println(" ");
	Serial.println(" ");
	Serial.println("starting");
}

void loop() {
	char    command[MAX_COMMAND_SIZE];
	int 	command_type;

	Serial.printf("initialized %i, mode %i, state %i, range %2.2f-%2.2f\n", persist.initialized, persist.mode, persist.state, persist.high, persist.low);
	strcpy(command, "h_low 45.23");
	Serial.printf("input string <%s>\n", command);
	command_type = get_command_type(command);
	Serial.printf("command type %i found\n", command_type);
	if (command_type != NULL)
		switch (get_command_type(command)) {
			// case 0:	// temperature
			// 	Serial.println("invalid command");
			break;
		case 1:	// humidity
			Serial.println("humidity reading");
			break;
		case 2:	// on
			Serial.println("mode is manual force on");
			persist.mode = MANUAL;
			persist.state = ON;
			break;
		case 3:	// off
			Serial.println("mode is manual force off");
			persist.mode = MANUAL;
			persist.state = OFF;
			break;
		case 4:	// h_low
			Serial.println("set low limit");
			persist.low = 45;
			Serial.printf("initialized %i, mode %i, state %i, range %2.2f-%2.2f\n", persist.initialized, persist.mode, persist.state, persist.high, persist.low);
			save_controller_state(&persist);
			persist.low = 99;
			Serial.printf("initialized %i, mode %i, state %i, range %2.2f-%2.2f\n", persist.initialized, persist.mode, persist.state, persist.high, persist.low);

			get_contoller_state(&persist);
			Serial.printf("initialized %i, mode %i, state %i, range %2.2f-%2.2f\n", persist.initialized, persist.mode, persist.state, persist.high, persist.low);

			break;
		case 5:	// h_high
			Serial.println("set high limit");
			break;
		// case 6:	// t_low
		//   Serial.println("invalid command");
		//   break;
		// case 7:	// t_ high
		//   Serial.println("invalid command");
		// break;
		case 8:	// auto
			Serial.println("mode is auto");
			persist.mode = AUTO;
			break;
		default:
			Serial.println("command type table is screwed up");
			break;
		}
	Serial.printf("state = %i, mode = %i,  - looping", persist.state, persist.mode);

	save_controller_state(&persist);
	delay(40000);



}


