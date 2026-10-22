#include <Arduino.h>
#define BUTTON_PIN    4     // Chân nút nhấn (ngắt ngoài)
#define LED_PIN       2     // Chân LED (điều khiển bằng PWM qua LEDC)
#define LED_CHANNEL   0     // Kênh LEDC dùng cho LED (core Arduino-ESP32 v2.x)
#define LED_FREQ      5000
#define LED_RES_BITS  8     // 8-bit -> giá trị PWM 0-255

// Cờ báo hiệu đã có sự kiện nhấn nút, dùng chung giữa ISR và loop()
volatile bool buttonPressed = false;

// ----------------- TRÌNH PHỤC VỤ NGẮT (ISR) -----------------
void IRAM_ATTR handleInterrupt() {
  buttonPressed = true;   // ISR ngắn gọn, chỉ đặt cờ
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n=== BAI 1: Ngat ngoai + PWM LED (ESP32 / PlatformIO) ===");

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Thiết lập kênh PWM (LEDC) cho LED: tần số 5000Hz, độ phân giải 8-bit (0-255)
  ledcSetup(LED_CHANNEL, LED_FREQ, LED_RES_BITS);
  ledcAttachPin(LED_PIN, LED_CHANNEL);

  // Gắn ngắt ngoài: khi tín hiệu tại BUTTON_PIN chuyển từ HIGH -> LOW (FALLING)
  // thì gọi hàm handleInterrupt()
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleInterrupt, FALLING);

  Serial.println("San sang. Nhan nut de xem LED nhay nhanh bao hieu ngat.");
}

void loop() {
  static int brightness = 0;
  static int step = 1;

  // Xử lý sự kiện nhấn nút NGAY khi cờ được đặt
  if (buttonPressed) {
    buttonPressed = false;
    Serial.println("Ngat: nut duoc nhan!");
    ledcWrite(LED_CHANNEL, 0);   delay(80);  // nháy tắt nhanh
    ledcWrite(LED_CHANNEL, 255); delay(80);  // nháy sáng nhanh
  }

  // Hiệu ứng fading: sáng dần rồi tối dần
  brightness += step;
  if (brightness <= 0 || brightness >= 255) step = -step;
  ledcWrite(LED_CHANNEL, brightness);
  delay(10);
}

/// tai = tuat