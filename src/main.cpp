#include <WiFi.h>
#include "Secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_LEDBackpack.h>
#include <Adafruit_GFX.h>

#define LED_MATRIX_ADDR 0x70
#define BUZZER_PIN 16
#define MQ135_PIN 34
#define MAX_PIN 32

#define AWS_IOT_PUBLISH_TOPIC "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

Adafruit_8x8matrix matrix = Adafruit_8x8matrix();
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);
int air_quality;
int sound_level;

int readAndDisplayAirQuality()
{

  int air_quality = analogRead(MQ135_PIN);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(1000);
  digitalWrite(BUZZER_PIN, LOW);

  matrix.clear();
  matrix.setCursor(0, 0);
  matrix.print(air_quality);
  matrix.writeDisplay();

  delay(1000);
  return air_quality;
}

int readAndDisplaySoundLevel()
{

  int sound_level = analogRead(MAX_PIN);

  matrix.clear();
  matrix.setCursor(0, 0);
  matrix.print(sound_level);
  matrix.writeDisplay();
  delay(1000);
  return sound_level;
}

String get_wifi_status(int status)
{
  switch (status)
  {
  case WL_IDLE_STATUS:
    return "WL_IDLE_STATUS";
  case WL_SCAN_COMPLETED:
    return "WL_SCAN_COMPLETED";
  case WL_NO_SSID_AVAIL:
    return "WL_NO_SSID_AVAIL";
  case WL_CONNECT_FAILED:
    return "WL_CONNECT_FAILED";
  case WL_CONNECTION_LOST:
    return "WL_CONNECTION_LOST";
  case WL_CONNECTED:
    return "WL_CONNECTED";
  case WL_DISCONNECTED:
    return "WL_DISCONNECTED";
  }
}

void messageHandler(char *topic, byte *payload, unsigned int length)
{
  Serial.print("incoming: ");
  Serial.println(topic);

  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char *message = doc["message"];
  Serial.println(message);
}

void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);

  // Create a message handler
  client.setCallback(messageHandler);

  Serial.println("Connecting to AWS IOT");

  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }

  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }

  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("AWS IoT Connected!");
}

void publishMessage()
{
  StaticJsonDocument<200> doc;
  doc["air_quality"] = air_quality;
  doc["sound_level"] = sound_level;
  char jsonBuffer[200];
  serializeJson(doc, jsonBuffer); // print to client

  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

void setup()
{
  // initialize local devices
  // Initialize I2C communication
  Wire.begin();

  // Initialize HT16K33 LED matrix
  matrix.begin(LED_MATRIX_ADDR);
  matrix.setBrightness(10);

  // Initialize MAX9814 microphone
  pinMode(MAX_PIN, INPUT);

  // Initialize MQ135 air quality sensor and passive buzzer
  pinMode(MQ135_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // initialize wifi connection and cloud connection
  Serial.begin(115200);
  delay(1000);

  int status = WL_IDLE_STATUS;
  connectAWS();
  Serial.println("\nConnecting");
  Serial.println(get_wifi_status(status));
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    status = WiFi.status();
    Serial.println(get_wifi_status(status));
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());
}

void loop()
{
  air_quality = readAndDisplayAirQuality();
  sound_level = readAndDisplaySoundLevel();
  if (isnan(air_quality) || isnan(sound_level)) // Check if any reads failed and exit early (to try again).
  {
    Serial.println(F("Failed to read from sensors!"));
    return;
  }
  Serial.print(F("Air_quality: "));
  Serial.print(air_quality);
  Serial.print(F("  Sound_level: "));
  Serial.print(sound_level);
  publishMessage();
  client.loop();
  delay(1000);
}
