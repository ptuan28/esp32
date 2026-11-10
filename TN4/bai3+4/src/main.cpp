#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ================= CẤU HÌNH: SỬA 4 DÒNG NÀY =================
const char*    WIFI_SSID = "FPT Tang 5";
const char*    WIFI_PASS = "88888888";
const char*    MQTT_HOST = "192.168.1.188";   // IP máy chạy Mosquitto (KHÔNG dùng localhost)
const char*    SV_ID     = "nhom-07";        // phải trùng SV_ID trong Node-RED
// ============================================================

const uint16_t MQTT_PORT = 1883;

// Chân phần cứng
#define PIN_LED 14   // LED qua điện trở 220Ω về GND
#define PIN_BTN 4    // nút nhấn nối GND (INPUT_PULLUP)
#define PIN_SDA 21
#define PIN_SCL 18

// Topic (được ghép từ SV_ID trong setup)
char topicTelemetry[64];  // chiều đi: ESP32 -> Node-RED
char topicLedState[64];   // chiều đi: trạng thái LED
char topicCmdLed[64];     // chiều về: Node-RED -> ESP32

WiFiClient   wifiClient;
PubSubClient mqtt(wifiClient);
Adafruit_AHTX0 aht;
Adafruit_SSD1306 oled(128, 64, &Wire, -1);

bool ledOn = false;
bool oledOk = false;
bool ahtOk = false;
unsigned long lastPublish = 0;
unsigned long lastReconnect = 0;

void publishLedState() {
  mqtt.publish(topicLedState, ledOn ? "1" : "0", true);
}

void setLed(bool on) {
  ledOn = on;
  digitalWrite(PIN_LED, on ? HIGH : LOW);
  publishLedState();
  Serial.printf("LED -> %s\n", on ? "ON" : "OFF");
}

// Nhận lệnh từ Node-RED (chiều về)
void onMessage(char* topic, byte* payload, unsigned int len) {
  String msg;
  for (unsigned int i = 0; i < len; i++) msg += (char)payload[i];
  msg.trim();
  msg.toLowerCase();
  Serial.printf("Nhan [%s]: %s\n", topic, msg.c_str());

  if (strcmp(topic, topicCmdLed) == 0) {
    if (msg == "1" || msg == "on" || msg == "true")        setLed(true);
    else if (msg == "0" || msg == "off" || msg == "false") setLed(false);
  }
}

void connectWiFi() {
  Serial.printf("Ket noi WiFi %s ", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 20000) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\nWiFi OK, IP: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\nWiFi THAT BAI (kiem tra SSID/mat khau, chi dung 2.4GHz)");
  }
}

bool connectMqtt() {
  String clientId = String("esp32-") + SV_ID + "-" + String((uint32_t)ESP.getEfuseMac(), HEX);
  Serial.printf("Ket noi MQTT %s:%u ... ", MQTT_HOST, MQTT_PORT);
  if (mqtt.connect(clientId.c_str())) {
    Serial.println("OK");
    mqtt.subscribe(topicCmdLed);
    Serial.printf("Da subscribe: %s\n", topicCmdLed);
    publishLedState();
    return true;
  }
  Serial.printf("THAT BAI, rc=%d\n", mqtt.state());
  return false;
}

void drawOled(float t, float h, bool btn) {
  if (!oledOk) return;
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(0, 0);
  oled.println(SV_ID);
  oled.printf("WiFi: %s\n", WiFi.status() == WL_CONNECTED ? "OK" : "--");
  oled.printf("MQTT: %s\n", mqtt.connected() ? "OK" : "--");
  oled.printf("T: %.1f C  H: %.1f %%\n", t, h);
  oled.printf("Nut: %s\n", btn ? "NHAN" : "nha");
  oled.setTextSize(2);
  oled.setCursor(0, 48);
  oled.printf("LED %s", ledOn ? "ON" : "OFF");
  oled.display();
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n=== Bai 3: MQTT pub/sub + LED ===");

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  pinMode(PIN_BTN, INPUT_PULLUP);

  Wire.begin(PIN_SDA, PIN_SCL);
  ahtOk  = aht.begin();                 // AHT20, địa chỉ I2C 0x38
  oledOk = oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  Serial.printf("AHT20: %s | OLED: %s\n", ahtOk ? "OK" : "KHONG THAY", oledOk ? "OK" : "KHONG THAY");

  snprintf(topicTelemetry, sizeof(topicTelemetry), "aet2022/%s/telemetry", SV_ID);
  snprintf(topicLedState,  sizeof(topicLedState),  "aet2022/%s/state/led", SV_ID);
  snprintf(topicCmdLed,    sizeof(topicCmdLed),    "aet2022/%s/cmd/led",   SV_ID);

  connectWiFi();
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMessage);
}

void loop() {
  // Giữ kết nối
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }
  if (!mqtt.connected()) {
    if (millis() - lastReconnect > 5000) {
      lastReconnect = millis();
      connectMqtt();
    }
  } else {
    mqtt.loop();
  }

  // Nút nhấn trên mạch: mỗi lần nhấn thì đảo LED (có chống rung),
  // đồng thời publish trạng thái lên Node-RED
  static bool lastBtn = false;
  static unsigned long lastBtnChange = 0;
  bool nowBtn = (digitalRead(PIN_BTN) == LOW);
  if (nowBtn != lastBtn && millis() - lastBtnChange > 50) {
    lastBtnChange = millis();
    lastBtn = nowBtn;
    if (nowBtn) {
      setLed(!ledOn);       // đảo LED + publish aet2022/<sv_id>/state/led
      lastPublish = 0;      // gửi telemetry ngay để dashboard cập nhật
    }
  }

  // Cứ 2 giây đo và publish telemetry (chiều đi)
  if (millis() - lastPublish >= 2000) {
    lastPublish = millis();

    float t = NAN, h = NAN;
    if (ahtOk) {
      sensors_event_t evHum, evTemp;
      aht.getEvent(&evHum, &evTemp);
      t = evTemp.temperature;
      h = evHum.relative_humidity;
    }
    bool btn = (digitalRead(PIN_BTN) == LOW);
    if (isnan(t)) t = 0;
    if (isnan(h)) h = 0;

    char json[160];
    snprintf(json, sizeof(json),
             "{\"temp\":%.2f,\"hum\":%.2f,\"btn\":%d,\"led\":%d,\"uptime\":%lu}",
             t, h, btn ? 1 : 0, ledOn ? 1 : 0, millis() / 1000);

    if (mqtt.connected()) {
      mqtt.publish(topicTelemetry, json);
    }
    Serial.println(json);
    drawOled(t, h, btn);
  }
}


// bước 1: run vào terminal : pkill mosquitto; $(brew --prefix)/opt/mosquitto/sbin/mosquitto -c $(brew --prefix)/etc/mosquitto/mosquitto.conf -v
// bước 2: cmt + T : run tiếp : node-red 
// bước 3: Chỉnh sửa flow: http://localhost:1880
//                         http://localhost:1880/ui
// bước phụ : kiểm tra IP MQTT_HOST : run terminal : ipconfig getifaddr en0 
//                                                 -> có ip thì sửa vào line 12 
// cuối cùng mở MQTT Explorer ở trong app của macOS
