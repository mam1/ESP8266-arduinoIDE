

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <NTPClient.h>
#include <time.h>

const char* ssid = "FrontierHSI";
const char* password = "";
const char* mqtt_server = "192.168.254.221";

#define LOOP_DELAY  60000

#define DHTPIN 13       // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)

#define NTP_OFFSET   60 * 60 * 20     // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "us.pool.ntp.org"

DHT dht(DHTPIN, DHTTYPE, 15);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[100];
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

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

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
      // Once connected, publish an announcement...
      // client.publish("258Thomas/testing", "hello world");
      // ... and resubscribe
      // client.subscribe("258Thomas/testing");
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(10000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  timeClient.begin();
  dht.begin();    // initialize the DHT22 sensor
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  float                 temperature;
  float                 humidity;
  String                str;
  unsigned long         epoch;
  char*                 c_time_string;
  time_t                unix_time;
 

  timeClient.update();
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
    if (isnan(temperature) || isnan(humidity)){
      Serial.println("DTH22 read failed");
      return;
    }
    epoch =  timeClient.getEpochTime();
    unix_time = static_cast<time_t>(epoch);
    c_time_string = ctime(&unix_time);

        // ========================
    // Relevant code in this section.
    char *src, *dst;
    for (src = dst = c_time_string; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != '\n') dst++;
    }
    *dst = '\0';
    // ========================
    // Serial.print("time string ");
    // Serial.println(c_time_string);

    // // print the hour, minute and second:
    // Serial.print("epoch ");
    // Serial.println(epoch);
    // Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    // Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
    // Serial.print(':');
    // if (((epoch % 3600) / 60) < 10) {
    //   // In the first 10 minutes of each hour, we'll want a leading '0'
    //   Serial.print('0');
    // }
    // Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    // Serial.print(':');
    // if ((epoch % 60) < 10) {
    //   // In the first 10 seconds of each minute, we'll want a leading '0'
    //   Serial.print('0');
    // }
    // Serial.print(epoch % 60); // print the second

    // Serial.printf("\n");


    snprintf (msg, 100, "%s temperature: %2.2f", c_time_string,temperature);
    client.publish("258Thomas/location/temperature", msg);
    // delay(2000);
    snprintf (msg, 100, "%s humidity: %2.2f", c_time_string,humidity);
    client.publish("258Thomas/location/humidity", msg);

    Serial.print(str);
    Serial.printf("%s published sensor readings\n", c_time_string);
  }
}

