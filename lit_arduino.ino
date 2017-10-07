#include "FastLED.h"
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <WiFiUdp.h>

#define NUM_LEDS 60

const char* ssid     = "OMITTED";
const char* password = "OMITTED";
const unsigned int port = 9000;
WiFiUDP socket;
char packetBuffer[NUM_LEDS*3];
CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<NEOPIXEL, 6>(leds, NUM_LEDS);
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  socket.begin(port);
}


void loop() {
  int packetSize = socket.parsePacket();
  if(packetSize) {
    Serial.print("Recieved packet");
    int len = socket.read(packetBuffer, 255);
    if (len > 0) {
      packetBuffer[len] = 0;
    }
    Serial.println("Contents:");
    for (int i = 0; i < len; i = i+3) {
          leds[i/3] = CRGB(packetBuffer[i], packetBuffer[i+1], packetBuffer[i+2]);
    }
    FastLED.show();
  }
}
