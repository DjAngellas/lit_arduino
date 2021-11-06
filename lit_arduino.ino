#include <NeoPixelBus.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <WiFiUdp.h>

#define TIMESTAMP_SIZE 8
#define NUM_LEDS 90
#define PACKET_SIZE (NUM_LEDS * 3 + TIMESTAMP_SIZE)

const char* ssid     = "OMITTED";
const char* password = "OMITTED";
const unsigned int port = 9000;
WiFiUDP socket;
char packetBuffer[PACKET_SIZE];
uint64_t latestTimestamp = 0;
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> neopixels(NUM_LEDS);

void setup() {
  Serial.begin(115200);
  neopixels.Begin();

  delay(10);

  Serial.println();
  Serial.println();


  while (WiFi.status() != WL_CONNECTED)
  {
    connectToWifi();
  }

  socket.begin(port);
}

void connectToWifi()
{
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.disconnect();
  WiFi.begin(ssid, password);

  while (WiFi.status() == WL_DISCONNECTED) {
    delay(500);
    Serial.print(".");
    //Serial.printf("%d", WiFi.status());
  }
  Serial.println("");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

uint64_t extractTimestamp(char* packetBuffer, int len) {
  char* timeArr = packetBuffer + len - 8;
  // Serial.printf("%d %d %d %d %d %d %d\n", timeArr[0], timeArr[1], timeArr[2], timeArr[3], timeArr[4], timeArr[5], timeArr[6],timeArr[7]); 
  return timeArr[0] + (timeArr[1]<<8) + (timeArr[2]<<16) + (timeArr[3]<<24) + (timeArr[4]<<32) + (timeArr[5]<<40) + (timeArr[6]<<48) + (timeArr[7]<<56);
}


void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectToWifi();
    return;
  }
  int packetSize = socket.parsePacket();
  if (!packetSize) {
    return;
  }
  if (packetSize == PACKET_SIZE) {
    int len = socket.read(packetBuffer, PACKET_SIZE);
    uint64_t timestamp = extractTimestamp(packetBuffer, len);
    if(timestamp < latestTimestamp) {
      Serial.println("Out of order packet received");
      return;
    } else {
      latestTimestamp = timestamp;
    }
    for (int i = 0; i < NUM_LEDS * 3; i = i + 3) {
      neopixels.SetPixelColor(i / 3, RgbColor(packetBuffer[i], packetBuffer[i + 1], packetBuffer[i + 2]));
    }
    neopixels.Show();
  }
  else {
    Serial.printf("ERROR: Incorrect packet size. Expected %d elements, but received %d.\n", NUM_LEDS * 3, packetSize);
  }
}
