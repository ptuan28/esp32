#include <Arduino.h>
#define ADC 34       // ADC1_CH6, chỉ input
#define LED 18
#define BTN 0        // nút BOOT trên board
#define N   8

#define LEDC_CH   0  // kênh PWM
#define LEDC_FREQ 5000
#define LEDC_RES  8  // 8-bit: 0..255

int buf[N];
int idx = 0;
bool dungLoc = false;
bool btnTruoc = HIGH;

void setup() {
  Serial.begin(115200);
  pinMode(BTN, INPUT_PULLUP);

  analogReadResolution(12);                 // 0..4095
  analogSetPinAttenuation(ADC, ADC_11db);   // đo 0..3.3V

  ledcSetup(LEDC_CH, LEDC_FREQ, LEDC_RES);  // core 2.x
  ledcAttachPin(LED, LEDC_CH);

  int v = analogRead(ADC);                  // nạp đầy buffer lúc khởi động
  for (int i = 0; i < N; i++) buf[i] = v;
}

int loc(int m) {
  buf[idx] = m;
  idx = (idx + 1) % N;
  long t = 0;
  for (int i = 0; i < N; i++) t += buf[i];
  return t / N;
}

void loop() {
  // Bấm BOOT để đổi giữa THÔ và CÓ LỌC
  bool btn = digitalRead(BTN);
  if (btnTruoc == HIGH && btn == LOW) {
    dungLoc = !dungLoc;
    Serial.println(dungLoc ? "== CO LOC ==" : "== THO ==");
    delay(200);
  }
  btnTruoc = btn;

  int tho = analogRead(ADC);   // 0..4095
  int l   = loc(tho);          // luôn gọi để buffer được cập nhật

  int giaTri = dungLoc ? l : tho;
  ledcWrite(LEDC_CH, map(giaTri, 0, 4095, 0, 255));  // core 2.x: ghi theo kênh

  Serial.print(">tho:"); Serial.println(tho);
  Serial.print(">loc:"); Serial.println(l);
  delay(20);
}