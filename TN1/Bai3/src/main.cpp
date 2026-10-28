#include <Arduino.h>

#define ADC_PIN     34      // ADC1_CH6 (đổi sang chân ADC2 ở phần ADC2)
#define LED_PIN     18
#define LED_CHANNEL 0       // chỉ dùng cho core 2.x
#define N           8       // số mẫu của bộ lọc trung bình trượt
#define USE_FILTER  0       // 0 = LED theo số liệu THÔ, 1 = LED theo số liệu ĐÃ LỌC

int buf[N];
int idx = 0;
bool bufReady = false;

// Lọc trung bình trượt N mẫu
int loc(int m) {
  if (!bufReady) {                          // lần đầu: điền đầy buffer bằng mẫu đầu tiên
    for (int i = 0; i < N; i++) buf[i] = m;
    bufReady = true;
  }
  buf[idx] = m;
  idx = (idx + 1) % N;
  long t = 0;
  for (int i = 0; i < N; i++) t += buf[i];
  return t / N;
}

void setup() {
  Serial.begin(115200);
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(LED_PIN, 5000, 8);
#else
  ledcSetup(LED_CHANNEL, 5000, 8);
  ledcAttachPin(LED_PIN, LED_CHANNEL);
#endif
}

void loop() {
  int tho = analogRead(ADC_PIN);            // 0 - 4095
  int l   = loc(tho);

#if USE_FILTER
  int v = l;
#else
  int v = tho;
#endif

  int duty = map(v, 0, 4095, 0, 255);
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(LED_PIN, duty);
#else
  ledcWrite(LED_CHANNEL, duty);
#endif

  // Định dạng Teleplot (tiện ích VS Code), giống slide
  Serial.print(">tho:"); Serial.println(tho);
  Serial.print(">loc:"); Serial.println(l);

  delay(20);
}