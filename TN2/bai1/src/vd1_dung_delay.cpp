#include <Arduino.h>
#include "thongke.h"

#define LED1 18
#define ADC  34

ThongKe tkDelay(500);
uint32_t daIn = 0;

void datLed(bool m) {
  digitalWrite(LED1, m);
  tkDelay.ghiNhan(micros());
  inKetQua("delay", tkDelay, daIn);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED1, OUTPUT);
}

void loop() {
  datLed(HIGH); delay(500);
  datLed(LOW);  delay(500);

  int tho = analogRead(ADC);
  Serial.printf(">bienTro:%d\n", tho);
  delay(2000);

  Serial.println("heartbeat");
  delay(5000);
}