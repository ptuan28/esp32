#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_BME280.h>

// ===== CẤU HÌNH: sửa các dòng này =====
const char* WIFI_SSID = "VJU Office";
const char* WIFI_PASS = "VJuOffice@2023";
const char* MQTT_HOST = "172.16.11.134";
const int   MQTT_PORT = 1883;
const char* SV_ID     = "25112120";
// ======================================

WiFiClient wifi;
PubSubClient mqtt(wifi);
String TOPIC_TELE;
String CLIENT_ID;

Adafruit_AHTX0  aht;
Adafruit_BMP280 bmp;
Adafruit_BME280 bme;
enum PressType { P_NONE, P_BMP, P_BME };
PressType pressType = P_NONE;

unsigned long lan = 0;

void noiBroker() {
  while (!mqtt.connected()) {
    Serial.print("Dang noi broker...");
    if (mqtt.connect(CLIENT_ID.c_str())) {
      Serial.println(" OK");
    } else {
      Serial.printf(" that bai (rc=%d), thu lai sau 2s\n", mqtt.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  if (!aht.begin()) {
    Serial.println("Khong tim thay AHT20!");
    while (true) delay(1000);
  }
  if (bmp.begin(0x77))      pressType = P_BMP;
  else if (bme.begin(0x77)) pressType = P_BME;

  TOPIC_TELE = String("aet2022/") + SV_ID + "/telemetry";
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  CLIENT_ID = String("esp32-") + SV_ID + "-" + mac;

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

  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setBufferSize(512);

  Serial.println("Topic:     " + TOPIC_TELE);
  Serial.println("Client id: " + CLIENT_ID);
}

void loop() {
  if (!mqtt.connected()) noiBroker();
  mqtt.loop();

  if (millis() - lan >= 2000) {
    lan = millis();

    sensors_event_t hum, temp;
    aht.getEvent(&hum, &temp);

    JsonDocument doc;
    doc["temp"] = round(temp.temperature * 10) / 10.0;
    doc["hum"]  = round(hum.relative_humidity * 10) / 10.0;
    if (pressType == P_BMP)      doc["pressure"] = round(bmp.readPressure() / 10.0) / 10.0;
    else if (pressType == P_BME) doc["pressure"] = round(bme.readPressure() / 10.0) / 10.0;
    doc["uptime"] = millis() / 1000;
    doc["rssi"]   = WiFi.RSSI();
    doc["id"]     = SV_ID;

    char b[256];
    serializeJson(doc, b);
    mqtt.publish(TOPIC_TELE.c_str(), b);
    Serial.println(b);
  }
}



//  mosquitto -c ~/mosquitto.conf -v