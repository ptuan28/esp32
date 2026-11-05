#include <Arduino.h>
#include <sys/time.h>

constexpr uint8_t  CHAN_LED  = 2;     // LED + dien tro 220-330 ohm
constexpr uint32_t GIAY_NGU  = 10;
constexpr uint32_t MS_CHO_USB = 1500; // cho Serial Monitor ket noi lai
constexpr int      GIAY_SANG  = 6;    // LED sang 3 giay khi dang thuc

RTC_DATA_ATTR int     bienRTC  = 0;   // GIU qua deep sleep
RTC_DATA_ATTR int64_t usLucNgu = 0;   // moc thoi gian luc bat dau ngu
int bienRAM = 0;                      // MAT qua deep sleep

int64_t layUs() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return (int64_t)tv.tv_sec * 1000000LL + tv.tv_usec;
}

const char* tenNguyenNhan(esp_sleep_wakeup_cause_t c) {
  switch (c) {
    case ESP_SLEEP_WAKEUP_TIMER:     return "TIMER (het gio ngu)";
    case ESP_SLEEP_WAKEUP_UNDEFINED: return "CAP NGUON / RESET";
    default:                         return "KHAC";
  }
}

void setup() {
  int64_t usLucThuc = layUs();        // lay moc ngay khi vua thuc
  Serial.begin(115200);
  delay(MS_CHO_USB);
  pinMode(CHAN_LED, OUTPUT);

  Serial.println("\n=== TH2 Bai 2: Deep sleep dinh gio ===");
  Serial.printf("Nguyen nhan thuc : %s\n", tenNguyenNhan(esp_sleep_get_wakeup_cause()));
  Serial.printf("VUA THUC         : bienRTC=%d | bienRAM=%d\n", bienRTC, bienRAM);
  if (usLucNgu > 0) {
    Serial.printf("Thoi gian ngu thuc te: %.2f giay\n", (usLucThuc - usLucNgu) / 1000000.0);
  }

  bienRTC++;
  bienRAM++;

  digitalWrite(CHAN_LED, HIGH);       // LED sang = CPU dang chay
  for (int i = GIAY_SANG; i > 0; i--) {
    Serial.printf("CPU dang chay, con %d giay roi ngu...\n", i);
    delay(1000);
  }

  digitalWrite(CHAN_LED, LOW);        // LED tat truoc khi ngu
  Serial.printf("TRUOC KHI NGU  : bienRTC=%d | bienRAM=%d\n", bienRTC, bienRAM);
  Serial.printf("-> Deep sleep %u giay...\n", (unsigned)GIAY_NGU);
  Serial.flush();

  usLucNgu = layUs();
  esp_sleep_enable_timer_wakeup(GIAY_NGU * 1000000ULL);
  esp_deep_sleep_start();
}

void loop() {}