#include <Arduino.h>
#include "thongke.h"

#define LED1 18
#define ADC  34
#define CPU_BAN_MS 80          // 0 = không giả lập bận

const uint32_t timerMs = 500;
uint32_t loopTick = 0, lanTruocAdc = 0, lanTruocHeart = 0, daIn = 0;
bool ledOn = false;
ThongKe tkMillis(timerMs);

void updateLoopTimer() {
  if (millis() - loopTick < timerMs) return;   // chưa đến giờ
  loopTick += timerMs;                         // giữ nhịp đều
  if (millis() - loopTick >= timerMs)
    loopTick = millis();                       // trễ quá -> bắt nhịp lại

  ledOn = !ledOn;
  digitalWrite(LED1, ledOn);
  tkMillis.ghiNhan(micros());                  // đo khoảng
}

void setup() {
  Serial.begin(115200);
  pinMode(LED1, OUTPUT);
}

void loop() {
  updateLoopTimer();

  uint32_t now = millis();
  if (now - lanTruocAdc >= 2000) {
    lanTruocAdc = now;
    Serial.printf(">bienTro:%d\n", analogRead(ADC));
  }
  if (now - lanTruocHeart >= 5000) {
    lanTruocHeart = now;
    Serial.println("heartbeat");
  }

  inKetQua("millis", tkMillis, daIn);
  if (CPU_BAN_MS) cpuBan(CPU_BAN_MS);          // loop() bận
}