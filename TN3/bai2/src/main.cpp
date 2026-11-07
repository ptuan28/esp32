#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>

#define TEN_NHOM "NHÓM_07"      // <- doi thanh ten nhom cua ban
#define SV_UUID  "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CH_UUID  "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class GhiGiaTri : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* c) override {
    auto v = c->getValue();
    Serial.printf("Da nhan gia tri moi: %s\n", v.c_str());
  }
};

void setup() {
  Serial.begin(115200);
  BLEDevice::init(TEN_NHOM);
  auto* sv = BLEDevice::createServer();
  auto* se = sv->createService(SV_UUID);
  auto* ch = se->createCharacteristic(
      CH_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  ch->setValue("25112120");          // thong diep rieng: ma so sinh vien
  ch->setCallbacks(new GhiGiaTri());
  se->start();
  BLEDevice::getAdvertising()->start();
  Serial.printf("BLE dang quang ba voi ten: %s\n", TEN_NHOM);
}

void loop() {
  delay(2000);
}