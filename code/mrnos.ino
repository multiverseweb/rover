/*--------------------------------------------------------------------------
| - Code for triggering warning when obstacle comes in the way             |
| - Collecting and sending humidity and temperature data to serial monitor |
| - Visualising humidity and temperature data on OLED screen               |
|                                                           Tejas Codes :) |
--------------------------------------------------------------------------*/

#include "DHT.h"
#include <Wire.h>
#include <U8g2lib.h>

// Pin Definitions
const int trigPin = 5;
const int echoPin = 18;
const int buzzer = 19;
const int led = 4;
const int DHTPIN = 23;

#define DHTTYPE DHT22
#define SOUND_SPEED 0.034
#define THRESHOLD_DISTANCE 10

DHT dht(DHTPIN, DHTTYPE);

// OLED Display (SH1106 1.3" I2C)
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

long duration;
float distanceCm;
unsigned long lastUpdate = 0;

// Graph Data
#define MAX_POINTS 50  // Number of data points in graph
float tempHistory[MAX_POINTS];  
float humidityHistory[MAX_POINTS];  
int dataIndex = 0;  

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(led, OUTPUT);
  dht.begin();
  
  u8g2.begin();
  delay(2000);
}

void loop() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  distanceCm = duration * SOUND_SPEED / 2;

  // Update every 10 seconds
  if (millis() - lastUpdate >= 10000) {
    lastUpdate = millis();
    
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    // Print in one line on Serial Monitor
    Serial.print("Temp: ");
    Serial.print(temperature, 1);
    Serial.print("C  |  Humidity: ");
    Serial.print(humidity, 1);
    Serial.println("%");

    // Shift Data to Left (Scrolling Effect)
    for (int i = 0; i < MAX_POINTS - 1; i++) {
      tempHistory[i] = tempHistory[i + 1];
      humidityHistory[i] = humidityHistory[i + 1];
    }
    
    // Add New Value
    tempHistory[MAX_POINTS - 1] = temperature;
    humidityHistory[MAX_POINTS - 1] = humidity;

    // Update OLED Display
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);

    // Display Temp & Humidity
    u8g2.setCursor(5, 10);
    u8g2.print("T: ");
    u8g2.print(temperature, 1);
    u8g2.print("C    H: ");
    u8g2.print(humidity, 1);
    u8g2.print("%");

    // Draw X and Y Axes
    u8g2.drawLine(8, 20, 8, 60);   // Y-axis
    u8g2.drawLine(8, 60, 120, 60); // X-axis

    // Plot Temperature Trend Graph 📈
    for (int i = 0; i < MAX_POINTS - 1; i++) {
      int x1 = 10 + i * 2;
      int x2 = 10 + (i + 1) * 2;
      int y1 = map(tempHistory[i], 0, 50, 60, 20);
      int y2 = map(tempHistory[i + 1], 0, 50, 60, 20);
      u8g2.drawLine(x1, y1, x2, y2);
    }

    // Plot Humidity Trend Graph 📈
    for (int i = 0; i < MAX_POINTS - 1; i++) {
      int x1 = 10 + i * 2;
      int x2 = 10 + (i + 1) * 2;
      int y1 = map(humidityHistory[i], 0, 100, 60, 20);
      int y2 = map(humidityHistory[i + 1], 0, 100, 60, 20);
      u8g2.drawLine(x1, y1, x2, y2);
    }

    u8g2.sendBuffer(); // Update display
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
