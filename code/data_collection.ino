#include <DHT.h>

#define DHTPIN 26
#define DHTTYPE DHT22 


DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  dht.begin();
  delay(2000); // Allow sensor to stabilize
  Serial.println("DHT22 sensor reading on ESP32...");
  Serial.println("Timestamp (ms)\tHumidity (%)\tTemperature (°C)\tTemperature (°F)");
}

void loop() {
  delay(2000);

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();          // Celsius
  float temperatureF = dht.readTemperature(true);     // Fahrenheit
  unsigned long timestamp = millis();                 // Timestamp in milliseconds

  // Error handling for sensor read failures
  if (isnan(humidity) || isnan(temperature) || isnan(temperatureF)) {
    Serial.println("Failed to read from DHT22 sensor!");
    return;
  }

  Serial.print(timestamp);
  Serial.print(" ms\t");
  Serial.print("Humidity: ");
  Serial.print(humidity, 2);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temperature, 2);
  Serial.print(" °C ");
  Serial.print(temperatureF, 2);
  Serial.println(" °F");
}
