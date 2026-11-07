#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

const char* TEN_WIFI = "FPT Tang 5";   // <- sua
const char* MAT_KHAU = "88888888";      // <- sua
constexpr int CHAN_LED = 2;

WebServer server(80);
bool ledBat = false;

void capNhatLed() { digitalWrite(CHAN_LED, ledBat ? HIGH : LOW); }

String trangWeb() {
  String h = "<!DOCTYPE html><html><head><meta charset='utf-8'>";
  h += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  h += "<title>ESP32 LED</title></head><body style='font-family:sans-serif;text-align:center'>";
  h += "<h1>ESP32 LED</h1>";
  h += ledBat ? "<h2 style='color:green'>LED: đang BẬT</h2>"
              : "<h2 style='color:red'>LED: đang TẮT</h2>";
  h += "<a href='/on'><button style='font-size:24px;margin:8px'>BẬT</button></a>";
  h += "<a href='/off'><button style='font-size:24px;margin:8px'>TẮT</button></a>";
  h += "<a href='/toggle'><button style='font-size:24px;margin:8px'>Toggle</button></a>";
  h += "</body></html>";
  return h;
}

void veTrangChu() {
  server.send(200, "text/html; charset=utf-8", trangWeb());
}

void chuyenVeTrangChu() {          
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  pinMode(CHAN_LED, OUTPUT);
  capNhatLed();

  WiFi.mode(WIFI_STA);
  WiFi.begin(TEN_WIFI, MAT_KHAU);
  Serial.printf("Dang noi Wi-Fi %s", TEN_WIFI);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nDa noi Wi-Fi!");
  Serial.print("Mo trinh duyet vao: http://");
  Serial.println(WiFi.localIP());

  server.on("/", veTrangChu);
  server.on("/on",     []() { ledBat = true;     capNhatLed(); chuyenVeTrangChu(); });
  server.on("/off",    []() { ledBat = false;    capNhatLed(); chuyenVeTrangChu(); });
  server.on("/toggle", []() { ledBat = !ledBat;  capNhatLed(); chuyenVeTrangChu(); });
  server.begin();
}

void loop() {
  server.handleClient();
}