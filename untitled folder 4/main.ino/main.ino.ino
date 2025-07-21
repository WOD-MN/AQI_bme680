#include "bsec.h"
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>

// WiFi credentials
const char *ssid = "sima_ftadi";
const char *password = "@Saiman12345";

// Server URL
const String serverURL = "http://192.168.1.86:5001/data";

// BME680 sensor object
Bsec iaqSensor;

// TSL2561 light sensor object (default I2C address 0x29)
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_LOW, 12345);

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

// Altitude at your location (in meters)
const float ALTITUDE = 200.0;

void setup(void) {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  // I2C pins
  Wire.begin(21, 22);

  // Initialize TSL2561
  if (!tsl.begin()) {
    Serial.println("TSL2561 not found!");
    while (1);
  }
  tsl.setGain(TSL2561_GAIN_1X);
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);

  // Initialize BME680
  initializeSensor();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
}

void loop(void) {
  // Read light
  sensors_event_t lightEvent;
  tsl.getEvent(&lightEvent);
  float lux = (lightEvent.light) ? lightEvent.light : 0;

  // Read BME680
  if (iaqSensor.run()) {
    smoothReadings();

    float avgTemp = tempTotal / numReadings;
    float avgHum = humTotal / numReadings;
    float avgIaq = iaqTotal / numReadings;

    float stationPressure = iaqSensor.pressure / 100.0; // hPa
    float seaLevelPressure = calculateSeaLevelPressure(stationPressure, ALTITUDE);

    // Serial Plotter output
    Serial.print(lux, 2); Serial.print(",");
    Serial.print(avgTemp, 2); Serial.print(",");
    Serial.print(avgHum, 2); Serial.print(",");
    Serial.print(seaLevelPressure, 2); Serial.print(",");
    Serial.print(avgIaq, 2); Serial.print(",");
    Serial.print(iaqSensor.co2Equivalent, 0); Serial.print(",");
    Serial.println(iaqSensor.breathVocEquivalent, 2);

    // Send to webapp
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(serverURL);
      http.addHeader("Content-Type", "application/json");

      String payload = "{";
      payload += "\"lux\":" + String(lux, 2) + ",";
      payload += "\"temperature\":" + String(avgTemp, 2) + ",";
      payload += "\"humidity\":" + String(avgHum, 2) + ",";
      payload += "\"pressure\":" + String(seaLevelPressure, 2) + ",";
      payload += "\"iaq\":" + String(avgIaq, 2) + ",";
      payload += "\"co2\":" + String(iaqSensor.co2Equivalent, 0) + ",";
      payload += "\"voc\":" + String(iaqSensor.breathVocEquivalent, 2);
      payload += "}";

      int httpResponseCode = http.POST(payload);
      if (httpResponseCode > 0) {
        Serial.print("Sent! HTTP ");
        Serial.println(httpResponseCode);
      } else {
        Serial.print("Failed to send. Code: ");
        Serial.println(httpResponseCode);
      }
      http.end();
    } else {
      Serial.println("WiFi not connected.");
    }

    digitalWrite(ledPin, HIGH);
  } else {
    checkIaqSensorStatus();
    digitalWrite(ledPin, LOW);
  }

  delay(1000);  // 1-second loop
}

float calculateSeaLevelPressure(float stationPressure, float altitude) {
  return stationPressure / pow(1 - (altitude / 44330.0), 5.255);
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

  for (int i = 0; i < numReadings; i++) {
    tempReadings[i] = iaqSensor.temperature;
    humReadings[i] = iaqSensor.humidity;
    iaqReadings[i] = iaqSensor.iaq;
    tempTotal += tempReadings[i];
    humTotal += humReadings[i];
    iaqTotal += iaqReadings[i];
  }
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

void checkIaqSensorStatus(void) {
  if (iaqSensor.bsecStatus != BSEC_OK) {
    Serial.print("BSEC error code: ");
    Serial.println(iaqSensor.bsecStatus);

    if (iaqSensor.bsecStatus < BSEC_OK) {
      Serial.println("BSEC fatal error!");
      while (1);
    } else {
      Serial.println("BSEC warning.");
    }
  }

  if (iaqSensor.bme68xStatus != BME68X_OK) {
    Serial.print("BME68X error code: ");
    Serial.println(iaqSensor.bme68xStatus);

    if (iaqSensor.bme68xStatus < BME68X_OK) {
      Serial.println("BME68X fatal error!");
      while (1);
    } else {
      Serial.println("BME68X warning.");
    }
  }
}
