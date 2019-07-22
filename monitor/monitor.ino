
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <DHT.h>
#include <PubSubClient.h>


#define DHTPIN 13        // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)

const char* ssid     = "FrontierHSI";
const char* password = "";

const char* mqtt_server = "192.168.254.221";
const uint16_t port = 1883;

unsigned int localPort = 2390;  // local port to listen for UDP packets
WiFiUDP udp;                    // A UDP instance to let us send and receive packets over UDP

/* Don't hardwire the IP address or we won't get the benefits of the pool.
    Lookup the IP address for the host name instead */

//IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "time.nist.gov";

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
unsigned long           epoch;
const unsigned long     seventyYears = 2208988800UL;

DHT dht(DHTPIN, DHTTYPE, 15);

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
      client.publish("258Thomas/testing", "hello world");
      // ... and resubscribe
      client.subscribe("258Thomas/testing");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // connect to a WiFi network
  setup_wifi();

  // wait for a good time stamp
  while (get_timestamp()==0) delay(100);

  // initialize the DHT22 sensor
  dht.begin();

  // connect to MQTT broker
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);        
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress& address) {
  // Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

unsigned long get_timestamp() {

  //get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServerIP);

  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  
  delay(1000);  // wait to see if a reply is available

  int cb = udp.parsePacket();
  delay(100);
  if (!cb) {
    // Serial.println("no packet yet");
    delay(100);
  } 
  else {
    // Serial.println("got a packet");
    // We've received a packet, read the data from it
    // read the packet into the buffer
    udp.read(packetBuffer, NTP_PACKET_SIZE); 
    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    // now convert NTP time into everyday time:
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    // subtract seventy years:
    epoch = secsSince1900 - seventyYears;
  }

  return epoch;
}

void loop() {
  unsigned long         tstamp;
  char                  msg[100];
  float                 temperature;
  float                 humidity;


  Serial.print("connecting to ");
  Serial.print(mqtt_server);
  Serial.print(':');
  Serial.println(port);

  if (client.connected()) {
  // build MQTT message
    tstamp = get_timestamp();
    temperature = dht.readTemperature(true);
    humidity = dht.readHumidity();
    if (isnan(temperature) || isnan(humidity)){
      Serial.println("DTH22 read failed");
      return;
    }

    snprintf (msg, 100, "%lu %2.2f %2.2f", tstamp, temperature, humidity);
    Serial.println(msg);
    client.publish("258Thomas/testing", msg);
  }

  // Close the connection
  Serial.println();
  // Serial.println("closing connection");
  client.stop();

  delay(9000);
}
