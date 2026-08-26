/*
  SENTRA — ESP32 + MYOSA prototype firmware

  Hardware (one shared I2C bus):
    SDA GPIO 21, SCL GPIO 22, 3.3 V, common GND
    APDS9960  0x39  - fingertip pulse proxy
    MPU6050   0x68  - mobility
    BMP180    0x77  - respiratory pressure proxy
    SSD1306   0x3C  - 128x64 OLED

  Arduino Library Manager dependencies:
    Adafruit APDS9960 Library
    Adafruit MPU6050
    Adafruit BMP085 Library (supports BMP180)
    Adafruit SSD1306
    Adafruit GFX Library
    Adafruit Unified Sensor
    Adafruit BusIO

  Important: This is demonstration firmware, not a medical device. The APDS9960
  and BMP180 are not medical pulse/respiration sensors. Calibrate and validate
  every threshold on your physical prototype before demonstrating it.
*/

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_APDS9960.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------- User configuration ----------
const char *WIFI_SSID = "";       // Leave empty to run without Wi-Fi.
const char *WIFI_PASSWORD = "";

constexpr uint8_t SDA_PIN = 21;
constexpr uint8_t SCL_PIN = 22;
constexpr uint8_t OLED_ADDRESS = 0x3C;
constexpr int8_t OLED_RESET = -1;
constexpr uint16_t SCREEN_WIDTH = 128;
constexpr uint16_t SCREEN_HEIGHT = 64;

constexpr uint32_t FAST_SAMPLE_MS = 40;   // 25 Hz: APDS9960 + MPU6050
constexpr uint32_t PRESSURE_SAMPLE_MS = 100; // 10 Hz: BMP180 in low-power mode
constexpr uint32_t OUTPUT_INTERVAL_MS = 1000;
constexpr uint32_t OLED_INTERVAL_MS = 250;
constexpr uint32_t BASELINE_TIME_MS = 30000;

Adafruit_APDS9960 apds;
Adafruit_BMP085 bmp;
Adafruit_MPU6050 mpu;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
WebServer server(80);

bool apdsReady = false;
bool bmpReady = false;
bool mpuReady = false;
bool oledReady = false;

struct Measurements {
  float heartRate = 0.0f;
  float respiratoryRate = 0.0f;
  float mobilityPercent = 100.0f;
  float mlProbability = 0.0f;
  uint8_t proxyScore = 0;
  bool fingerPresent = false;
} data;

uint32_t startedAt = 0;
uint32_t lastFastSample = 0;
uint32_t lastPressureSample = 0;
uint32_t lastOutput = 0;
uint32_t lastOled = 0;

// Pulse detector state.
float pulseDc = 0.0f;
float pulseEnvelope = 1.0f;
float previousPulseAc = 0.0f;
uint32_t lastBeatAt = 0;

// Respiration detector state.
float pressureDc = 0.0f;
float pressureEnvelope = 0.2f;
float previousPressureAc = 0.0f;
uint32_t lastBreathAt = 0;

// Mobility baseline state.
float motionEnergy = 0.0f;
float motionBaselineSum = 0.0f;
uint32_t motionBaselineSamples = 0;
float motionBaseline = 0.035f;

float smoothValue(float current, float incoming, float alpha) {
  if (current <= 0.0f) return incoming;
  return current + alpha * (incoming - current);
}

float sigmoid(float value) {
  return 1.0f / (1.0f + expf(-value));
}

void detectPulse(uint8_t raw, uint32_t now) {
  data.fingerPresent = raw > 20;
  if (!data.fingerPresent) {
    data.heartRate = 0.0f;
    pulseDc = raw;
    pulseEnvelope = 1.0f;
    previousPulseAc = 0.0f;
    return;
  }

  if (pulseDc == 0.0f) pulseDc = raw;
  pulseDc += 0.025f * (static_cast<float>(raw) - pulseDc);
  float ac = static_cast<float>(raw) - pulseDc;
  pulseEnvelope += 0.05f * (fabsf(ac) - pulseEnvelope);
  float threshold = max(1.8f, pulseEnvelope * 1.25f);

  bool risingCrossing = previousPulseAc <= threshold && ac > threshold;
  if (risingCrossing && now - lastBeatAt > 330) {
    if (lastBeatAt != 0) {
      float bpm = 60000.0f / static_cast<float>(now - lastBeatAt);
      if (bpm >= 40.0f && bpm <= 180.0f) {
        data.heartRate = smoothValue(data.heartRate, bpm, 0.28f);
      }
    }
    lastBeatAt = now;
  }
  previousPulseAc = ac;

  if (lastBeatAt != 0 && now - lastBeatAt > 2500) data.heartRate = 0.0f;
}

void detectBreath(float pressurePa, uint32_t now) {
  if (pressureDc == 0.0f) pressureDc = pressurePa;
  pressureDc += 0.018f * (pressurePa - pressureDc);
  float ac = pressurePa - pressureDc;
  pressureEnvelope += 0.04f * (fabsf(ac) - pressureEnvelope);
  float threshold = max(0.16f, pressureEnvelope * 0.75f);

  bool risingCrossing = previousPressureAc <= threshold && ac > threshold;
  if (risingCrossing && now - lastBreathAt > 1200) {
    if (lastBreathAt != 0) {
      float breathsPerMinute = 60000.0f / static_cast<float>(now - lastBreathAt);
      if (breathsPerMinute >= 6.0f && breathsPerMinute <= 40.0f) {
        data.respiratoryRate = smoothValue(data.respiratoryRate, breathsPerMinute, 0.24f);
      }
    }
    lastBreathAt = now;
  }
  previousPressureAc = ac;

  if (lastBreathAt != 0 && now - lastBreathAt > 10000) data.respiratoryRate = 0.0f;
}

void updateMobility() {
  sensors_event_t accel, gyro, temperature;
  mpu.getEvent(&accel, &gyro, &temperature);

  float magnitude = sqrtf(
    accel.acceleration.x * accel.acceleration.x +
    accel.acceleration.y * accel.acceleration.y +
    accel.acceleration.z * accel.acceleration.z
  );
  float deviationFromGravity = fabsf(magnitude - SENSORS_GRAVITY_STANDARD);
  motionEnergy += 0.06f * (deviationFromGravity - motionEnergy);

  if (millis() - startedAt < BASELINE_TIME_MS) {
    motionBaselineSum += motionEnergy;
    motionBaselineSamples++;
    data.mobilityPercent = 100.0f;
  } else {
    if (motionBaselineSamples > 0) {
      motionBaseline = max(0.035f, motionBaselineSum / motionBaselineSamples);
      motionBaselineSamples = 0;
    }
    float percent = 100.0f * motionEnergy / motionBaseline;
    data.mobilityPercent = constrain(percent, 0.0f, 150.0f);
  }
}

void runEdgeModel() {
  // Compact 3-feature logistic-regression model for a demo TinyML pipeline.
  // Replace these example weights with coefficients trained on your own data.
  float normalizedHeart = data.heartRate > 0 ? (data.heartRate - 85.0f) / 20.0f : 0.0f;
  float normalizedResp = data.respiratoryRate > 0 ? (data.respiratoryRate - 18.0f) / 8.0f : 0.0f;
  float reducedMobility = (100.0f - data.mobilityPercent) / 50.0f;
  float logit = -1.65f + 0.90f * normalizedHeart + 1.05f * normalizedResp + 0.85f * reducedMobility;
  data.mlProbability = constrain(sigmoid(logit), 0.0f, 1.0f);

  data.proxyScore = 0;
  if (data.heartRate > 90.0f) data.proxyScore++;
  if (data.respiratoryRate > 22.0f) data.proxyScore++;
  if (millis() - startedAt >= BASELINE_TIME_MS && data.mobilityPercent < 50.0f) data.proxyScore++;
}

void drawOled() {
  if (!oledReady) return;
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("SENTRA"));
  display.setCursor(78, 0);
  display.print(F("RISK "));
  display.setTextSize(2);
  display.print(data.proxyScore);

  display.drawLine(0, 14, 127, 14, SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print(F("HR"));
  display.setCursor(27, 20);
  if (data.heartRate > 0) display.print(data.heartRate, 0); else display.print(F("--"));
  display.print(F(" bpm"));

  display.setCursor(0, 32);
  display.print(F("RR"));
  display.setCursor(27, 32);
  if (data.respiratoryRate > 0) display.print(data.respiratoryRate, 0); else display.print(F("--"));
  display.print(F(" /min"));

  display.setCursor(0, 44);
  display.print(F("MOVE"));
  display.setCursor(27, 44);
  display.print(data.mobilityPercent, 0);
  display.print(F("%"));

  display.drawLine(0, 54, 127, 54, SSD1306_WHITE);
  display.setCursor(0, 56);
  display.print(F("ML "));
  display.print(data.mlProbability * 100.0f, 0);
  display.print(F("%"));
  display.setCursor(73, 56);
  display.print(data.fingerPresent ? F("FINGER OK") : F("NO FINGER"));
  display.display();
}

String jsonPayload() {
  String json;
  json.reserve(280);
  json += F("{\"heartRate\":");
  json += String(data.heartRate, 1);
  json += F(",\"respiratoryRate\":");
  json += String(data.respiratoryRate, 1);
  json += F(",\"mobility\":");
  json += String(data.mobilityPercent, 1);
  json += F(",\"proxyScore\":");
  json += String(data.proxyScore);
  json += F(",\"mlProbability\":");
  json += String(data.mlProbability, 3);
  json += F(",\"fingerPresent\":");
  json += data.fingerPresent ? F("true") : F("false");
  json += F(",\"uptimeMs\":");
  json += String(millis());
  json += F("}");
  return json;
}

void configureWebServer() {
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/plain", "SENTRA ESP32 is online. GET /data for readings.");
  });
  server.on("/data", HTTP_GET, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json", jsonPayload());
  });
  server.onNotFound([]() { server.send(404, "application/json", "{\"error\":\"not found\"}"); });
  server.begin();
}

void showStartupMessage(const __FlashStringHelper *line1, const __FlashStringHelper *line2) {
  if (!oledReady) return;
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 16);
  display.println(line1);
  display.setCursor(0, 32);
  display.println(line2);
  display.display();
}

void setup() {
  Serial.begin(115200);
  delay(250);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);

  oledReady = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS, false, false);
  showStartupMessage(F("SENTRA starting"), F("Checking sensors..."));

  apdsReady = apds.begin(10, APDS9960_AGAIN_4X, 0x39, &Wire);
  if (apdsReady) {
    apds.setProxGain(APDS9960_PGAIN_4X);
    apds.setProxPulse(APDS9960_PPULSELEN_16US, 8);
    apds.setLED(APDS9960_LEDDRIVE_50MA, APDS9960_LEDBOOST_100PCNT);
    apds.enableProximity(true);
  }

  bmpReady = bmp.begin(BMP085_ULTRALOWPOWER, &Wire);
  mpuReady = mpu.begin(0x68, &Wire);
  if (mpuReady) {
    mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  }

  Serial.printf("Sensors: APDS=%s BMP180=%s MPU6050=%s OLED=%s\n",
    apdsReady ? "OK" : "FAIL", bmpReady ? "OK" : "FAIL",
    mpuReady ? "OK" : "FAIL", oledReady ? "OK" : "FAIL");

  if (strlen(WIFI_SSID) > 0) {
    showStartupMessage(F("Connecting WiFi"), F("Please wait..."));
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    uint32_t wifiStartedAt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - wifiStartedAt < 12000) delay(200);
    if (WiFi.status() == WL_CONNECTED) {
      configureWebServer();
      Serial.print(F("Dashboard data: http://"));
      Serial.print(WiFi.localIP());
      Serial.println(F("/data"));
    }
  }

  startedAt = millis();
  showStartupMessage(F("Sensors ready"), F("Baseline: 30 sec"));
  delay(900);
}

void loop() {
  uint32_t now = millis();
  if (WiFi.status() == WL_CONNECTED) server.handleClient();

  if (now - lastFastSample >= FAST_SAMPLE_MS) {
    lastFastSample = now;
    if (apdsReady) detectPulse(apds.readProximity(), now);
    if (mpuReady) updateMobility();
  }

  if (now - lastPressureSample >= PRESSURE_SAMPLE_MS) {
    lastPressureSample = now;
    if (bmpReady) detectBreath(static_cast<float>(bmp.readPressure()), now);
  }

  runEdgeModel();

  if (now - lastOled >= OLED_INTERVAL_MS) {
    lastOled = now;
    drawOled();
  }

  if (now - lastOutput >= OUTPUT_INTERVAL_MS) {
    lastOutput = now;
    Serial.println(jsonPayload());
  }
}
