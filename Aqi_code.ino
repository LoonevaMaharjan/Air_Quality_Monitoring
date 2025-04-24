#include <DHT.h>
#include <SoftwareSerial.h>
#include <SdsDustSensor.h>
#include <ESP8266WiFi.h>
#include <time.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

const char* WALRUS_PUBLISHER_URL = "https://walrus-testnet-publisher.stakeengine.co.uk"; // Walrus URL for testnet

// WiFi Configuration
const char* ssid = "Chhauni Mesh";  // Change to your WiFi network name
const char* password = "iBriZ@2025"; // Change to your WiFi password

// DHT22 Setup for temperature and humidity
#define DHTPIN D4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// MQ135 for CO2 (using analog input)
#define MQ135_PIN A0

// SDS011 Setup for PM2.5 and PM10
SoftwareSerial sdsSerial(D5, D6); // RX, TX pins for SDS011
SdsDustSensor sds(sdsSerial);

// LEDs and Buzzer
#define RED_LED D2
#define BUZZER D7

void setup() {
  Serial.begin(9600);
  dht.begin();

  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi connected");

  // Turn on Red LED when WiFi is connected
  digitalWrite(RED_LED, HIGH);

  // Sync time using NTP (set UTC, adjust for Nepal timezone manually)
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  while (time(nullptr) < 100000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n⏱ NTP Time Synced");

  // Start SDS011 Dust Sensor
  sds.begin();
  sds.wakeup(); // Wake up the sensor
}

void loop() {
  // Get time and adjust for Nepal timezone (UTC +5:45 = +20700 seconds)
  time_t now = time(nullptr) + 20700;  // Adjust to Nepal Time
  struct tm* timeinfo = localtime(&now);
  char timeStr[30];
  strftime(timeStr, sizeof(timeStr), "%I:%M:%S %p", timeinfo);

  // Read sensors
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  int mq135Value = analogRead(MQ135_PIN);
  float co2_ppm = mq135Value * 10;  // Approximate CO2 ppm

  PmResult pm = sds.readPm();
  if (pm.isOk()) {
    int pm25 = pm.pm25;
    int pm10 = pm.pm10;

    // Evaluate AQI status based on CO2 and PM2.5
    String aqiStatus = getHybridAQIStatus(co2_ppm, pm25);

    // Update LEDs and buzzer based on AQI status
    updateIndicators(aqiStatus);

    // Display data on Serial Monitor
    Serial.println("==============================");
    Serial.print("🕒 Time        : "); Serial.println(timeStr);
    Serial.print("🌡 Temperature : "); Serial.print(temp); Serial.println(" °C");
    Serial.print("💧 Humidity    : "); Serial.print(hum); Serial.println(" %");
    Serial.print("🧪 CO₂ Level (Est.) : "); Serial.print(co2_ppm); Serial.println(" ppm");
    Serial.print("🌫 Fine Dust (PM2.5)   : "); Serial.print(pm25); Serial.println(" µg/m³");
    Serial.print("🌪 Coarse Dust (PM10)  : "); Serial.print(pm10); Serial.println(" µg/m³");
    Serial.print("📊 AQI Status  : "); Serial.println(aqiStatus);
    Serial.println("==============================\n");

    // Create payload to upload
  String payload = "{";
  payload += "\"time\": \"" + String(timeStr) + "\",";
  payload += "\"temperature\": " + String(temp) + ",";
  payload += "\"humidity\": " + String(hum) + ",";
  payload += "\"co2_ppm\": " + String(co2_ppm) + ",";
  payload += "\"pm25\": " + String(pm25) + ",";
  payload += "\"pm10\": " + String(pm10) + ",";
  payload += "\"aqi_status\": \"" + aqiStatus + "\"";
  payload += "}";


    // Upload data to Walrus
    uploadToWalrus(payload);
  }

  delay(10000);  // Wait for 10 seconds before next loop
}

// Function to evaluate AQI status based on CO2 and PM2.5 values
String getHybridAQIStatus(float co2_ppm, float pm25) {
  int co2_level = 0, pm25_level = 0;

  // AQI levels for CO2
  if (co2_ppm <= 1000) co2_level = 1;
  else if (co2_ppm <= 2000) co2_level = 2;
  else if (co2_ppm <= 5000) co2_level = 3;
  else if (co2_ppm <= 40000) co2_level = 4;
  else co2_level = 5;

  // AQI levels for PM2.5
  if (pm25 <= 12.0) pm25_level = 1;
  else if (pm25 <= 35.4) pm25_level = 2;
  else if (pm25 <= 150.4) pm25_level = 3;
  else if (pm25 <= 250.4) pm25_level = 4;
  else pm25_level = 5;

  // Combine AQI levels for CO2 and PM2.5
  int final_level = max(co2_level, pm25_level);

  // Return AQI status
  switch (final_level) {
    case 1: return "Good 🌿";
    case 2: return "Moderate 🌤";
    case 3: return "Unhealthy 😷";
    case 4: return "Very Unhealthy ☢";
    case 5: return "Hazardous ☠";
    default: return "Unknown";
  }
}

// Function to update LED and buzzer based on AQI status
void updateIndicators(String status) {
  digitalWrite(BUZZER, LOW);  // Turn off buzzer by default

  if (status == "Good 🌿") {
    // No alert for good air quality
  } else if (status == "Moderate 🌤") {
    // No alert for moderate air quality
  } else if (status == "Unhealthy 😷") {
    tone(BUZZER, 30000); delay(500); noTone(BUZZER);  // Short alert for unhealthy
  } else if (status == "Very Unhealthy ☢") {
    tone(BUZZER, 30000); delay(500); noTone(BUZZER);  // Alert for very unhealthy
  } else if (status == "Hazardous ☠") {
    for (int i = 0; i < 3; i++) {
      tone(BUZZER, 30000); delay(300); noTone(BUZZER); delay(200);  // Multiple alerts for hazardous
    }
  }
}

// Function to upload data to Walrus publisher
void uploadToWalrus(String payload) {
  if (WiFi.status() == WL_CONNECTED) {
  WiFiClientSecure client;
  client.setInsecure(); // Skip certificate validation
  client.setBufferSizes(512, 512); // Optional: helps with larger payloads

    

    HTTPClient https;
    String url = String(WALRUS_PUBLISHER_URL) + "/v1/blobs?epochs=5";

    Serial.println("🌐 Uploading to Walrus...");

    if (https.begin(client, url)) {
      https.addHeader("Content-Type", "text/plain");
      https.setTimeout(15000); // 10s timeout

      int httpCode = https.PUT(payload);
      if (httpCode > 0) {
        String response = https.getString();
        Serial.println("✅ Walrus Upload Response:");
        Serial.println(response);

        // Extract blob ID from JSON manually
        int idStart = response.indexOf("\"id\":\"") + 6;
        int idEnd = response.indexOf("\"", idStart);
        String blobId = response.substring(idStart, idEnd);

        // Serial.println("📦 Blob ID: " + blobId);
      } else {
        Serial.print("❌ Upload failed, error: ");
        Serial.println(https.errorToString(httpCode));
      }

      https.end();
    } else {
      Serial.println("❌ HTTPS begin failed");
    }
  } else {
    Serial.println("❌ WiFi not connected!");
  }
}

