#include <WiFi.h>

const char* ssid = "ESP32";      // ĐỔI theo tên nhóm, ví dụ "Nhom3_ESP32"
const char* pass = "12345678";    // ĐỔI, tối thiểu 8 ký tự

void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid, pass);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
}

void loop() {}