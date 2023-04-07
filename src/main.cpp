#include <WiFi.h>
#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <MQ135.h>
#include "Secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_LEDBackpack.h>
#include <Adafruit_GFX.h>
#include <BluetoothSerial.h>
#include <NimBLEDevice.h>

#define LED_MATRIX_ADDR 0x70
#define BUZZER_PIN 16
#define MQ135_PIN 34
#define MAX_PIN 32

#define AWS_IOT_PUBLISH_TOPIC "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"
MQ135 mq135(MQ135_PIN);
Adafruit_8x8matrix matrix = Adafruit_8x8matrix();
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);
float air_quality;
float sound_level;

static const char *SERVICE_UUID = "3a4e2ff2-c9fb-11ed-afa1-0242ac120002";
static const char *CHARACTERISTIC_UUID = "7214fb32-c9fb-11ed-afa1-0242ac120002";
AsyncWebServer server(80);
BluetoothSerial SerialBT;

class CharacteristicCallbacks : public NimBLECharacteristicCallbacks
{
  void onWrite(NimBLECharacteristic *pCharacteristic)
  {
    std::string value = pCharacteristic->getValue();
    Serial.print("Received value: ");
    Serial.println(value.c_str());
  }
};

float readAndDisplayAirQuality()
{

  float air_quality = mq135.getPPM();
  /* digitalWrite(BUZZER_PIN, HIGH);
  delay(1000);
  digitalWrite(BUZZER_PIN, LOW); */
  /*   matrix.clear();
    matrix.setCursor(0, 0);
    matrix.print(air_quality);
    matrix.writeDisplay();
   */
  delay(100);
  return air_quality;
}

float readAndDisplaySoundLevel()
{

  float max9814_gain = 60.0;   // Set this to the gain value you have selected for the MAX9814 (40dB, 50dB, or 60dB)
  float adc_ref_voltage = 3.3; // Set this to your ADC reference voltage (e.g., 3.3V or 5V)
  int mic_value = analogRead(MAX_PIN);
  Serial.println(mic_value);
  float output_voltage = (float)mic_value / 4095.0 * adc_ref_voltage;
  float sound_level1 = 20 * log10(output_voltage / adc_ref_voltage);

  /*   matrix.clear();
    matrix.setCursor(0, 0);
    matrix.print(sound_level1);
    matrix.writeDisplay(); */
  delay(100);
  return sound_level1;
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
  float air_quality = doc["air_quality"];
  float sound_level = doc["sound_level"];
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]); // Print payload content
  }
  char buzzer = (char)payload[62]; // Extracting the controlling command from the Payload to Controlling Buzzer from AWS
  digitalWrite(BUZZER_PIN, LOW);
  if (air_quality > 100 || sound_level > -5)
  {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(1000);
    digitalWrite(BUZZER_PIN, LOW);
    delay(1000);

    // Draw "X" on the LED matrix
    matrix.clear();
    matrix.drawLine(0, 0, 7, 7, LED_ON);
    matrix.drawLine(0, 7, 7, 0, LED_ON);
    matrix.writeDisplay();
  }
  else
  {
    digitalWrite(BUZZER_PIN, HIGH);
    // Draw "O" on the LED matrix
    matrix.clear();
    matrix.drawCircle(3, 3, 3, LED_ON);
    matrix.writeDisplay();
  }
}

void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }

  //
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

/* void processBluetoothData()
{
  if (SerialBT.available())
  {
    String data = SerialBT.readStringUntil('\n');
    if (data == "1")
    { // Turn on the LED matrix display
      matrix.fillScreen(1);
      matrix.writeDisplay();
    }
    else if (data == "0")
    { // Turn off the LED matrix display
      matrix.fillScreen(0);
      matrix.writeDisplay();
    }
  }
} */

void handleResult(AsyncWebServerRequest *request)
{
  if (request->hasParam("value"))
  {
    int result = request->getParam("value")->value().toInt();

    Serial.print(F(" ESP-EYE: "));
    Serial.print(result);

    request->send(200, "text/plain", "OK");
  }
  else
  {
    request->send(400, "text/plain", "Bad Request");
  }
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
  delay(100);

  connectAWS();
  Serial.println("Starting Arduino BLE Server application...");

  NimBLEDevice::init("Receiver");

  NimBLEServer *pServer = NimBLEDevice::createServer();
  NimBLEService *pService = pServer->createService(SERVICE_UUID);
  NimBLECharacteristic *pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      NIMBLE_PROPERTY::READ |
          NIMBLE_PROPERTY::WRITE |
          NIMBLE_PROPERTY::NOTIFY);

  pCharacteristic->setCallbacks(new CharacteristicCallbacks());
  pCharacteristic->setValue("Arduino Peripheral");

  pService->start();

  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();

  int status = WL_IDLE_STATUS;
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
  // OTA part
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", "Hi! I am ESP32."); });

  AsyncElegantOTA.begin(&server); // Start ElegantOTA
                                  // Register the request handler for incoming results
  server.on("/result", HTTP_GET, handleResult);
  server.begin();
  Serial.println("HTTP server started");
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
  Serial.print(F(" Sound_level: "));
  Serial.print(sound_level);
  Serial.print(" dB");
  /* processBluetoothData(); // Process incoming Bluetooth data */
  publishMessage();
  client.loop();
  delay(100);
}
