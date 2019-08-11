/*
  periodically read a DHT22 sensor and publish the readings

*/

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <NTPClient.h>
#include <time.h>

const char* ssid = "FrontierHSI";
const char* password = "";
const char* mqtt_server = "192.168.254.221";

#define PARENT_TOPIC  "258Thomas"
#define TYPE_TOPIC "sensor"
#define LOCATION_TOPIC "test"
#define TEMPERATURE_TOPIC "temperature"
#define HUMIDITY_TOPIC "humidity"
#define MQTT_MESSAGE_SIZE  100
#define LOOP_DELAY  6000                     // time between readings
#define DHT22_OFFSET -30

#define DHTPIN 13                             // what pin we're connected to
#define DHTTYPE DHT22                         // DHT 22  (AM2302)

#define EST_OFFSET   -4                       // convert GMT to EST
#define NTP_OFFSET   60 * 60 * EST_OFFSET     // In seconds
#define NTP_INTERVAL 60 * 1000                // In milliseconds
#define NTP_ADDRESS  "us.pool.ntp.org"

DHT dht(DHTPIN, DHTTYPE, 15);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[MQTT_MESSAGE_SIZE];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

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


  pinMode(BUILTIN_LED, OUTPUT);         // initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);                 // start the serial interface
  setup_wifi();                         // connect to wifi
  timeClient.begin();                   // initialize time client
  dht.begin();                          // initialize the DHT22 sensor
  client.setServer(mqtt_server, 1883);  // initialize MQTT broker
  client.setCallback(callback);

}

void loop() {

  String                tt;
  String                th;
  float                 temperature;
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

    temperature = dht.readTemperature(true);
    humidity = dht.readHumidity();
    humidity += float(DHT22_OFFSET);
    if (isnan(temperature) || isnan(humidity)){
      Serial.println("DTH22 read failed");
      return;
    }

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

    // publish the readings
    tt = PARENT_TOPIC;
    tt += "/";
    tt += TYPE_TOPIC;
    tt += "/";
    tt += LOCATION_TOPIC;
    tt += "/";
    tt += TEMPERATURE_TOPIC;

    th = PARENT_TOPIC;
    th += "/";
    th += TYPE_TOPIC;
    th += "/";
    th += LOCATION_TOPIC;
    th += "/";
    th += HUMIDITY_TOPIC;

    snprintf (msg, MQTT_MESSAGE_SIZE, "%s temperature: %2.2f", c_time_string,temperature);
    client.publish(tt.c_str(), msg);
    snprintf (msg, MQTT_MESSAGE_SIZE, "%s humidity: %2.2f", c_time_string,humidity);
    client.publish(th.c_str(), msg);


    // snprintf (msg, MQTT_MESSAGE_SIZE, "%2.2f",humidity);
    // client.publish(th.c_str(), msg);

    int sensorValue = 50;

    sensorValue = (int)humidity;

      Serial.println(sensorValue);
      Serial.print(" ");

    // Serial.printf("%2.2f/n ", humidity);

    // Serial.printf("%s published sensor readings\n", c_time_string);
  }
}

