#include "bsec.h"
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// WiFi credentials
const char *ssid = "sima_ftadi";
const char *password = "@Saiman12345";

// Server URL
const String serverURL = "http://192.168.1.86:5001/data";

// Sensor object
Bsec iaqSensor;

// Data smoothing variables
const int numReadings = 5;
float tempReadings[numReadings] = {0};
float humReadings[numReadings] = {0};
float iaqReadings[numReadings] = {0};
int readIndex = 0;
float tempTotal = 0;
float humTotal = 0;
float iaqTotal = 0;

// LED pin for status indication
const int ledPin = LED_BUILTIN;

void setup(void) {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  
  // Initialize I2C and sensor
  Wire.begin(21, 22);
  initializeSensor();
  
  // Connect to WiFi
  connectToWiFi();
}

void loop(void) {
  if (iaqSensor.run()) {
    // Apply smoothing to readings
    smoothReadings();
    
    // Calculate averages
    float avgTemp = tempTotal / numReadings;
    float avgHum = humTotal / numReadings;
    float avgIaq = iaqTotal / numReadings;
    
    // Create and send JSON payload
    sendSensorData(avgTemp, avgHum, avgIaq);
    
    delay(3000); // Send data every 30 seconds
  } else {
    checkIaqSensorStatus();
  }
}

void initializeSensor() {
  iaqSensor.begin(BME68X_I2C_ADDR_HIGH, Wire);
  checkIaqSensorStatus();

  bsec_virtual_sensor_t sensorList[13] = {
    BSEC_OUTPUT_IAQ,
    BSEC_OUTPUT_STATIC_IAQ,
    BSEC_OUTPUT_CO2_EQUIVALENT,
    BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
    BSEC_OUTPUT_RAW_TEMPERATURE,
    BSEC_OUTPUT_RAW_PRESSURE,
    BSEC_OUTPUT_RAW_HUMIDITY,
    BSEC_OUTPUT_RAW_GAS,
    BSEC_OUTPUT_STABILIZATION_STATUS,
    BSEC_OUTPUT_RUN_IN_STATUS,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
    BSEC_OUTPUT_GAS_PERCENTAGE
  };

  iaqSensor.updateSubscription(sensorList, 13, BSEC_SAMPLE_RATE_LP);
  checkIaqSensorStatus();

  // Initialize smoothing arrays
  for (int i = 0; i < numReadings; i++) {
    tempReadings[i] = iaqSensor.temperature;
    humReadings[i] = iaqSensor.humidity;
    iaqReadings[i] = iaqSensor.iaq;
    tempTotal += tempReadings[i];
    humTotal += humReadings[i];
    iaqTotal += iaqReadings[i];
  }
}

void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    digitalWrite(ledPin, !digitalRead(ledPin)); // Toggle LED while connecting
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    digitalWrite(ledPin, HIGH);
  } else {
    Serial.println("\nFailed to connect to WiFi");
    digitalWrite(ledPin, LOW);
  }
}

void sendSensorData(float temp, float hum, float iaq) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected. Attempting to reconnect...");
    connectToWiFi();
    return;
  }

  HTTPClient http;
  http.begin(serverURL);
  http.addHeader("Content-Type", "application/json");

  // Create JSON payload
  StaticJsonDocument<256> doc;
  doc["temperature"] = round(temp * 10) / 10.0;
  doc["humidity"] = round(hum * 10) / 10.0;
  doc["pressure"] = round(iaqSensor.pressure / 100);
  doc["iaq"] = round(iaq * 10) / 10.0;
  doc["iaq_category"] = getIaqCategory(iaq);
  doc["co2"] = round(iaqSensor.co2Equivalent);
  doc["voc"] = round(iaqSensor.breathVocEquivalent * 100) / 100.0;
  doc["sensor_id"] = "ESP32_Node1"; // Change this for multiple sensors

  String payload;
  serializeJson(doc, payload);

  // Send HTTP POST request
  int httpResponseCode = http.POST(payload);
  
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    Serial.println("Payload: " + payload);
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    Serial.println("Error sending data to server");
  }
  
  http.end();
}

void smoothReadings() {
  tempTotal -= tempReadings[readIndex];
  humTotal -= humReadings[readIndex];
  iaqTotal -= iaqReadings[readIndex];
  
  tempReadings[readIndex] = iaqSensor.temperature;
  humReadings[readIndex] = iaqSensor.humidity;
  iaqReadings[readIndex] = iaqSensor.iaq;
  
  tempTotal += tempReadings[readIndex];
  humTotal += humReadings[readIndex];
  iaqTotal += iaqReadings[readIndex];
  
  readIndex = (readIndex + 1) % numReadings;
}

String getIaqCategory(float iaq) {
  if (iaq <= 50) return "Excellent";
  if (iaq <= 100) return "Good";
  if (iaq <= 150) return "Moderate";
  if (iaq <= 200) return "Unhealthy";
  if (iaq <= 300) return "Very Unhealthy";
  return "Hazardous";
}

void checkIaqSensorStatus(void) {
  if (iaqSensor.bsecStatus != BSEC_OK) {
    Serial.print("BSEC error code: ");
    Serial.println(iaqSensor.bsecStatus);
    
    // Check for specific errors
    if (iaqSensor.bsecStatus < BSEC_OK) {
      Serial.println("BSEC error, sensor might be unavailable!");
      while (1); // Halt if there's a sensor error
    } else {
      Serial.println("BSEC warning");
    }
  }

  if (iaqSensor.bme68xStatus != BME68X_OK) {
    Serial.print("BME68X error code: ");
    Serial.println(iaqSensor.bme68xStatus);
    
    if (iaqSensor.bme68xStatus < BME68X_OK) {
      Serial.println("BME68X error, sensor might be unavailable!");
      while (1); // Halt if there's a sensor error
    } else {
      Serial.println("BME68X warning");
    }
  }
}
