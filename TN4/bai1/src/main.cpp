#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_BME280.h>

const char* WIFI_SSID = "FPT Tang 5";
const char* WIFI_PASS = "88888888";

WebServer server(80);

Adafruit_AHTX0   aht;
Adafruit_BMP280  bmp;
Adafruit_BME280  bme;

enum PressType { P_NONE, P_BMP, P_BME };
PressType pressType = P_NONE;
const char* pressName = "none";

void handleSensor() {
  // Nhiệt độ + độ ẩm từ AHT20
  sensors_event_t hum, temp;
  aht.getEvent(&hum, &temp);

  float t = temp.temperature;
  float h = hum.relative_humidity;
  if (isnan(t) || isnan(h)) {
    server.send(500, "application/json", "{\"error\":\"AHT read failed\"}");
    return;
  }

  JsonDocument doc;
  doc["temp"] = t;
  doc["hum"]  = h;

  // Áp suất từ BMP280/BME280 (hPa)
  if (pressType == P_BMP)      doc["pressure"] = bmp.readPressure() / 100.0F;
  else if (pressType == P_BME) doc["pressure"] = bme.readPressure() / 100.0F;

  doc["uptime"] = millis() / 1000;
  // --- trường RIÊNG của bạn ---
  doc["id"]     = "25112120";
  doc["rssi"]   = WiFi.RSSI();

  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  if (!aht.begin()) {
    Serial.println("Khong tim thay AHT20 (0x38)! Kiem tra day.");
    while (true) delay(1000);
  }
  Serial.println("AHT20 OK");

  if (bmp.begin(0x77)) {
    pressType = P_BMP; pressName = "BMP280";
  } else if (bme.begin(0x77)) {
    pressType = P_BME; pressName = "BME280";
  }
  Serial.printf("Cam bien ap suat: %s\n", pressName);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Dang ket noi Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("IP cua ESP32: ");
  Serial.println(WiFi.localIP());

  server.on("/api/sensor", HTTP_GET, handleSensor);
  server.begin();
  Serial.println("Web server da chay (cong 80)");
}

void loop() {
  server.handleClient();
}