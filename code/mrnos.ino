#include "DHT.h"
#include <Wire.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <HTTPClient.h>

// WiFi Credentials
const char* ssid = "_____";
const char* password = "_____";

// ThingSpeak API
const char* apiKey = "JRT402JKXZ86O0SC";
const char* server = "http://api.thingspeak.com/update";

// Pin Definitions
const int trigPin = 5;
const int echoPin = 18;
const int buzzer = 19;
const int led = 4;
const int DHTPIN = 23;

#define DHTTYPE DHT22
#define SOUND_SPEED 0.034
#define THRESHOLD_DISTANCE 15

DHT dht(DHTPIN, DHTTYPE);

// OLED Display (SH1106 1.3" I2C)
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

long duration;
float distanceCm;
unsigned long lastUpdate = 0;

// Graph Data
#define MAX_POINTS 50
float tempHistory[MAX_POINTS];
float humidityHistory[MAX_POINTS];

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(led, OUTPUT);
  dht.begin();

  u8g2.begin();
  delay(2000);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  unsigned long startAttemptTime = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect! Check WiFi credentials.");
  } else {
    Serial.println("Connected to WiFi.");
  }
}

void loop() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distanceCm = duration * SOUND_SPEED / 2;

  // Update every 2 seconds
  if (millis() - lastUpdate >= 2000) {
    lastUpdate = millis();

    float temperatureC = dht.readTemperature();
    float humidity = dht.readHumidity();
    float temperatureF = temperatureC * 9.0 / 5.0 + 32.0;

    // Print in one line on Serial Monitor
    Serial.print("Humidity: ");
    Serial.print(humidity, 1);
    Serial.print("%  |  Temp: ");
    Serial.print(temperatureC, 1);
    Serial.print("°C  |  ");
    Serial.print(temperatureF, 1);
    Serial.print("°F  |  Distance: ");
    Serial.print(distanceCm, 1);
    Serial.println(" cm");

    // Send Data to ThingSpeak
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      String url = String(server) + "?api_key=" + apiKey + "&field1=" + String(humidity, 1) + "&field2=" + String(temperatureC, 1) + "&field3=" + String(temperatureF, 1) + "&field4=" + String(distanceCm, 1);
      http.begin(url);
      int httpResponseCode = http.GET();
      http.end();
    } else {
      Serial.println("WiFi Disconnected! Reconnecting...");
      WiFi.begin(ssid, password);
    }

    // Shift Data for OLED Graph
    for (int i = 0; i < MAX_POINTS - 1; i++) {
      tempHistory[i] = tempHistory[i + 1];
      humidityHistory[i] = humidityHistory[i + 1];
    }

    // Add New Value
    tempHistory[MAX_POINTS - 1] = temperatureC;
    humidityHistory[MAX_POINTS - 1] = humidity;

    // Update OLED Display
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);

    u8g2.setCursor(5, 10);
    u8g2.print("H: ");
    u8g2.print(humidity, 1);
    u8g2.print("%    T: ");
    u8g2.print(temperatureC, 1);
    u8g2.print("°C");

    // Draw Axes
    u8g2.drawLine(8, 20, 8, 60);
    u8g2.drawLine(8, 60, 120, 60);

    // Plot Temperature Graph 📈
    for (int i = 0; i < MAX_POINTS - 1; i++) {
      int x1 = 10 + i * 2;
      int x2 = 10 + (i + 1) * 2;
      int y1 = map(tempHistory[i], 0, 50, 60, 20);
      int y2 = map(tempHistory[i + 1], 0, 50, 60, 20);
      u8g2.drawLine(x1, y1, x2, y2);
    }

    // Plot Humidity Graph 📈
    for (int i = 0; i < MAX_POINTS - 1; i++) {
      int x1 = 10 + i * 2;
      int x2 = 10 + (i + 1) * 2;
      int y1 = map(humidityHistory[i], 0, 100, 60, 20);
      int y2 = map(humidityHistory[i + 1], 0, 100, 60, 20);
      u8g2.drawLine(x1, y1, x2, y2);
    }

    u8g2.sendBuffer();  // Update display
  }

  // Obstacle Detection
  if (distanceCm <= THRESHOLD_DISTANCE && distanceCm > 0) {
    digitalWrite(buzzer, HIGH);
    digitalWrite(led, HIGH);
    delay(200);
  } else {
    digitalWrite(buzzer, LOW);
    digitalWrite(led, LOW);
  }

  delay(500);
}
