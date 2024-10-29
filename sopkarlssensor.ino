#include "secrets.h"
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#define TRIG_PIN D1
#define ECHO_PIN D2


const char* host = "api.thingspeak.com";
const int httpsPort = 443;

unsigned long myChannelNumber = SECRET_CH_ID;
const char* writeAPIKey = SECRET_WRITE_APIKEY;

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

WiFiClientSecure client;


long readDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(20);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  long distance = duration * 0.034 / 2;

  Serial.print("Mätt avstånd: ");
  Serial.println(distance);

  return distance;
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.println("Försöker ansluta till WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nAnsluten till WiFi!");

  client.setInsecure();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    long distance = readDistance();
    sendToThingSpeak(distance);
  }

  delay(20000);
}

void sendToThingSpeak(long value) {
  if (!client.connect(host, httpsPort)) {
    Serial.println("Anslutning till ThingSpeak misslyckades");
    return;
  }

  String url = "/update?api_key=" + String(writeAPIKey) + "&field1=" + String(value);
  Serial.print("Försöker skicka data till: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }
  String response = client.readString();
  Serial.println(response);

  client.stop();
}
