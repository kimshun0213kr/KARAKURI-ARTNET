#include <WiFi.h>
#include <WiFiUdp.h>
#include <esp_dmx.h>

// WiFi設定
const char *ssid = "KARAKURI_ARTNET";
WiFiUDP udp;
const int udpPort = 12345;

// DMX設定
#define DMX_PORT DMX_NUM_1
const int DMX_TX_PIN = 7;  // 実際の接続に合わせて
const int DMX_RX_PIN = 6;  // 未使用でも設定が必要
const int DMX_RTS_PIN = 8; // DEピン（送信方向切り替え）に使う

// DMX受信用バッファ
uint8_t dmxData[DMX_PACKET_SIZE];

void setup() {
  Serial.begin(115200);

  // WiFi接続
  WiFi.begin(ssid);
  Serial.print("WiFi接続中...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi接続完了");
  Serial.print("IPアドレス: ");
  Serial.println(WiFi.localIP());

  // UDP開始
  udp.begin(udpPort);
  Serial.print("UDPポート ");
  Serial.print(udpPort);
  Serial.println(" を監視中");

  // DMXポート初期化
  dmx_config_t config = DMX_CONFIG_DEFAULT;
  dmx_driver_install(DMX_PORT, &config, nullptr, 0);
  dmx_set_pin(DMX_PORT, DMX_TX_PIN, DMX_RX_PIN, DMX_RTS_PIN);
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize == 512) {
    udp.read(dmxData, 512);

    // DMXに書き込み
    dmx_write(DMX_PORT, dmxData, 512);
    dmx_send(DMX_PORT);
    dmx_wait_sent(DMX_PORT, DMX_TIMEOUT_TICK);

    Serial.println("UDP受信 → DMX出力完了");
  }
}
