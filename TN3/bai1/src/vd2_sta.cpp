#include <WiFi.h>

const char* ssid = "FPT Tang 5";      // ĐÚNG tên AP muốn nối (AP của board 1 / router / hotspot)
const char* pass = "88888888";    // ĐÚNG mật khẩu của AP đó

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nIP: ");
  Serial.println(WiFi.localIP());
}

void loop() {}