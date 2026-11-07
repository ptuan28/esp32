#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

const char* TEN_WIFI  = "VJU Office";   // <- sua
const char* MAT_KHAU  = "VJuOffice@2023";      // <- sua
const char* BROKER    = "broker.emqx.io";
const char* TOPIC_DEM = "bcse/th3/nhom-07";
const char* TOPIC_LED = "bcse/th3/nhom-07/led";
const char* CLIENT_ID = "esp32-nhom07-kit1";  // khac nhau cho moi kit
constexpr int CHAN_LED = 2;

WiFiClient wifi;
PubSubClient mqtt(wifi);
int dem = 0;
unsigned long lanGuiCuoi = 0;

void nhanLenh(char* topic, byte* payload, unsigned int len) {
  Serial.printf("Nhan [%s]: %.*s\n", topic, len, (char*)payload);
  if (len > 0 && payload[0] == '1') digitalWrite(CHAN_LED, HIGH);
  if (len > 0 && payload[0] == '0') digitalWrite(CHAN_LED, LOW);
}

void noiBroker() {
  while (!mqtt.connected()) {
    Serial.print("Dang noi broker...");
    if (mqtt.connect(CLIENT_ID)) {
      Serial.println(" OK");
      mqtt.subscribe(TOPIC_LED);
    } else {
      Serial.printf(" that bai (rc=%d), thu lai sau 2 giay\n", mqtt.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  pinMode(CHAN_LED, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(TEN_WIFI, MAT_KHAU);
  Serial.printf("Dang noi Wi-Fi %s", TEN_WIFI);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nDa noi Wi-Fi!");

  mqtt.setServer(BROKER, 1883);
  mqtt.setCallback(nhanLenh);
}

void loop() {
  if (!mqtt.connected()) noiBroker();
  mqtt.loop();

  if (millis() - lanGuiCuoi >= 2000) {
    lanGuiCuoi = millis();
    String s = String(dem++);
    mqtt.publish(TOPIC_DEM, s.c_str());
    Serial.printf("Gui [%s]: %s\n", TOPIC_DEM, s.c_str());
  }
}

