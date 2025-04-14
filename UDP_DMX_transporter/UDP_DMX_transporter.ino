#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESPDMX.h>

// アクセスポイント設定
const char *ap_ssid = "KARAKURI_ARTNET";

// UDPブロードキャスト設定
WiFiUDP udp;
const int udpPort = 12345;
IPAddress broadcastIP(192, 168, 4, 255);  // ブロードキャストアドレス（AP側）

// DMX受信＆出力
DMXESPSerial dmxReceiver;  // DMX IN (RX)
DMXESPSerial dmxSender;    // DMX OUT (TX)

uint8_t dmxBuffer[512];

// ピン設定
const int DMX_RX_PIN = 6;   // DMX IN（MAX485 RO）
const int DMX_TX_PIN = 7;   // DMX OUT（MAX485 DI）
const int DMX_CTRL_PIN = 8; // DE/RE 制御ピン（HIGH=送信）

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

  // DMX受信初期化（RX = D6）
  dmxReceiver.setPin(DMX_RX_PIN);
  dmxReceiver.begin();

  // DMX出力初期化（TX = D7）
  pinMode(DMX_CTRL_PIN, OUTPUT);
  digitalWrite(DMX_CTRL_PIN, HIGH);  // 送信モード
  dmxSender.setPin(DMX_TX_PIN);
  dmxSender.begin();

  Serial.println("DMX受信・スルー出力準備完了");

  // 準備完了をLEDで知らせる
  digitalWrite(10, HIGH);
}

void loop() {
  // DMXデータ取得
  for (int i = 0; i < 512; i++) {
    dmxBuffer[i] = dmxReceiver.read(i + 1);  // 1ch始まり
  }

  // UDPブロードキャスト送信
  udp.beginPacket(broadcastIP, udpPort);
  udp.write(dmxBuffer, 512);
  udp.endPacket();

  // DMXスルー出力
  for (int i = 0; i < 512; i++) {
    dmxSender.write(i + 1, dmxBuffer[i]);
  }
  dmxSender.update();

  Serial.println("DMX受信 → UDP送信 → DMXスルー出力 完了");

  delay(20);  // 20ms周期（50fps相当）
}
