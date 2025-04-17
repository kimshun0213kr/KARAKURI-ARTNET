#include <WiFi.h>
#include <WiFiUdp.h>
#include <esp_dmx.h>

// アクセスポイント設定
const char *ap_ssid = "KARAKURI_ARTNET";

// UDP設定
WiFiUDP udp;
const int udpPort = 12345;
IPAddress broadcastIP(192, 168, 4, 255);  // ブロードキャストアドレス

// DMX設定
#define DMX_PORT_RX DMX_NUM_1
#define DMX_PORT_TX DMX_NUM_2

const int DMX_RX_PIN = 6;
const int DMX_TX_PIN = 7;
const int DMX_RTS_PIN = 8; // DE/RE 共通制御ピン

// 受信バッファ
uint8_t dmxBuffer[DMX_PACKET_SIZE];

void setup() {
  pinMode(10, OUTPUT);
  Serial.begin(115200);

  // アクセスポイント開始
  WiFi.softAP(ap_ssid);
  delay(100);
  Serial.println("AP起動完了");
  Serial.print("SSID: "); Serial.println(ap_ssid);
  Serial.print("IPアドレス: "); Serial.println(WiFi.softAPIP());

  // UDP初期化
  udp.begin(udpPort);

  // DMX受信初期化
  dmx_config_t config = DMX_CONFIG_DEFAULT;
  dmx_driver_install(DMX_PORT_RX, &config, NULL, 0);
  dmx_set_pin(DMX_PORT_RX, -1, DMX_RX_PIN, -1); // RXのみ使う

  // DMX送信初期化
  dmx_driver_install(DMX_PORT_TX, &config, NULL, 0);
  dmx_set_pin(DMX_PORT_TX, DMX_TX_PIN, -1, DMX_RTS_PIN); // TXのみ使う（DE制御あり）

  Serial.println("DMX受信・スルー出力準備完了");

  digitalWrite(10, HIGH); // 準備完了表示
}

void loop() {
  // DMX受信（最大512バイト）
  int length = dmx_read(DMX_PORT_RX, dmxBuffer, DMX_PACKET_SIZE);

  if (length > 0) {
    // UDPブロードキャスト送信
    udp.beginPacket(broadcastIP, udpPort);
    udp.write(dmxBuffer, length);
    udp.endPacket();

    // DMXスルー出力
    dmx_write(DMX_PORT_TX, dmxBuffer, length);
    dmx_send(DMX_PORT_TX);
    dmx_wait_sent(DMX_PORT_TX, DMX_TIMEOUT_TICK);

    Serial.println("DMX受信 → UDP送信 → DMXスルー出力 完了");
  }

  delay(20); // 約50fps
}
