#pragma once
#include <Arduino.h>

// Ghi nhận mỗi lần sự kiện xảy ra, tính khoảng thực tế và độ lệch so với chu kỳ đặt
struct ThongKe {
  uint32_t chuKyUs;
  volatile uint32_t last = 0, n = 0, khoang = 0, maxLech = 0;
  volatile uint64_t tongLech = 0;

  ThongKe(uint32_t ms) : chuKyUs(ms * 1000UL) {}

  void IRAM_ATTR ghiNhan(uint32_t nowUs) {
    if (last) {
      uint32_t kt   = nowUs - last;
      uint32_t lech = kt > chuKyUs ? kt - chuKyUs : chuKyUs - kt;
      khoang = kt;
      if (lech > maxLech) maxLech = lech;
      tongLech = tongLech + lech;
      n = n + 1;
    }
    last = nowUs;
  }
};

// In theo định dạng Teleplot (>ten:gia_tri), chỉ in khi có sự kiện mới
inline void inKetQua(const char* ten, ThongKe& t, uint32_t& daIn) {
  if (t.n == daIn || t.n == 0) return;
  daIn = t.n;
  Serial.printf(">khoang_%s:%lu\n",  ten, (unsigned long)t.khoang);
  Serial.printf(">lechTB_%s:%lu\n",  ten, (unsigned long)(t.tongLech / t.n));
  Serial.printf(">lechMax_%s:%lu\n", ten, (unsigned long)t.maxLech);
}

// Giả lập "CPU bận" bằng vòng chờ chiếm CPU (không dùng delay)
inline void cpuBan(uint32_t ms) {
  uint32_t t = millis();
  while (millis() - t < ms) { }
}