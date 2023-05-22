#include <WiFi.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

// WIFI Configuration
const char* ssid = "";
const char* password = "";

// DHT11 Sensor Configuration
#define DHTPIN 27
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// API Configuration
const char* serverUrl = "http://127.0.0.1:8080/api/v1/sensor/temperature?token=1234567";

void setup() {
  Serial.begin(115200);

  dht.begin();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  Serial.println(WiFi.localIP());
}

void loop() {
  // every 1 minute
  delay(1 * 60 * 1000);

  float temperature = dht.readTemperature();
  float moisture = dht.readHumidity();

  if (isnan(temperature) || isnan(moisture)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  StaticJsonDocument<200> doc;
  doc["machineNo"] = "ESP32-001";
  doc["location"] = "Warehouse";
  doc["temperature"] = String(temperature);
  doc["moisture"] = String(moisture);
  String payload;
  serializeJson(doc, payload);

  Serial.println("HTTP Request payload: " + payload);

  WiFiClient client;
  HTTPClient http;

  http.begin(client, serverUrl);
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(payload);
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("HTTP Response code: " + String(httpResponseCode));
    Serial.println("HTTP Response content: " + response);
  } else {
    Serial.println("Failed to connect to API");
  }

  http.end();
}
