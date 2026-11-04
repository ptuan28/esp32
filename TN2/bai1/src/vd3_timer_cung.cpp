#include <Arduino.h>
#include "thongke.h"

#define LED1 18
#define ADC  34
#define CPU_BAN_MS 80          // giống vd2 để so sánh công bằng

const uint32_t timerMs = 500;
hw_timer_t* hwTimer = nullptr;
volatile bool ledOn = false;
uint32_t lanTruocAdc = 0, lanTruocHeart = 0, daIn = 0;
ThongKe tkHw(timerMs);

void IRAM_ATTR onTimer() {                     // ISR: ngắn gọn
  uint32_t now = micros();
  ledOn = !ledOn;
  digitalWrite(LED1, ledOn);
  tkHw.ghiNhan(now);                           // đo khoảng
}

void khoiTaoTimer() {
  hwTimer = timerBegin(0, 80, true);           // timer0, chia 80 -> 1 tick = 1 us
  timerAttachInterrupt(hwTimer, &onTimer, true);
  timerAlarmWrite(hwTimer, timerMs * 1000, true);
  timerAlarmEnable(hwTimer);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED1, OUTPUT);
  khoiTaoTimer();
}

void loop() {
  uint32_t now = millis();
  if (now - lanTruocAdc >= 2000) {
    lanTruocAdc = now;
    Serial.printf(">bienTro:%d\n", analogRead(ADC));
  }
  if (now - lanTruocHeart >= 5000) {
    lanTruocHeart = now;
    Serial.println("heartbeat");
  }

  inKetQua("hw", tkHw, daIn);                  // in ở loop, không in trong ISR
  if (CPU_BAN_MS) cpuBan(CPU_BAN_MS);
}