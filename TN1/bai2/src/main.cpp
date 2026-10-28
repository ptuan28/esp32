#include <Arduino.h>
#define BUTTON_PIN   4     // Chân nút nhấn (ngắt ngoài)
#define LED_PIN      2     // Chân LED báo hiệu
#define DEBOUNCE_MS  50    // Cửa sổ chống dội: bỏ qua sườn xuống trong 50ms sau sườn đầu

volatile uint32_t debouncedCount = 0;       // Số lần nhấn nút THỰC SỰ (đã chống dội)
volatile uint32_t rawEdgeCount = 0;         // Số sườn xuống THÔ (để đối chiếu, chưa chống dội)
volatile unsigned long lastInterruptTime = 0; // Mốc thời gian (ms) của sườn xuống hợp lệ gần nhất

// ----------------- ISR: CÓ CHỐNG DỘI BẰNG millis() -----------------
void IRAM_ATTR handleInterrupt() {
  rawEdgeCount++;   // Ghi nhận mọi sườn xuống thô, chỉ để đối chiếu

  unsigned long now = millis();
  if (now - lastInterruptTime >= DEBOUNCE_MS) {
    // Đã qua 50ms kể từ sườn xuống hợp lệ trước đó -> đây là lần nhấn MỚI
    debouncedCount++;
    lastInterruptTime = now;
  }
  // Ngược lại: sườn xuống này rơi vào "cửa sổ chống dội" -> bỏ qua, không đếm
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n=== BAI 2: Demo Debounce bang millis() trong ISR ===");

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleInterrupt, FALLING);

  Serial.println("San sang. Nhan nut va so sanh 'THO' voi 'DA CHONG DOI'.");
}

void loop() {
  static uint32_t lastShownCount = 0;

  // Sao chép biến dùng chung trong lúc tạm tắt ngắt để đọc cho an toàn
  noInterrupts();
  uint32_t deb = debouncedCount;
  uint32_t raw = rawEdgeCount;
  interrupts();

  // Chỉ xử lý khi có lần nhấn mới
  if (deb != lastShownCount) {
    lastShownCount = deb;

    digitalWrite(LED_PIN, !digitalRead(LED_PIN));   // Đảo LED mỗi lần nhấn thật

    Serial.print("Nhan nut #");
    Serial.print(deb);
    Serial.print("  |  suon xuong THO: ");
    Serial.print(raw);
    Serial.print("  |  DA CHONG DOI: ");
    Serial.println(deb);
  }
}