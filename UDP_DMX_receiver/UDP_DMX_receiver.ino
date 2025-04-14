#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESPDMX.h>

// WiFi接続設定
const char *ssid = "KARAKURI_ARTNET";

WiFiUDP udp;
const int udpPort = 12345;

// DMX出力用オブジェクト（ESPDMXライブラリ）
DMXESPSerial dmxOutput;
const int DMX_TX_PIN = 7;

// DMX受信用バッファ
uint8_t receivedDMX[512];

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

  // DMX出力初期化（TXピン指定）
  dmxOutput.setPin(DMX_TX_PIN);
  dmxOutput.begin();
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize == 512) {
    udp.read(receivedDMX, 512);

    // DMXスロットにデータをセット
    for (int i = 0; i < 512; i++) {
      dmxOutput.setSlot(i + 1, receivedDMX[i]); // 1chスタート
    }

    // DMX送信開始（BREAKなど含む正しいフォーマットで送信される）
    dmxOutput.update();

    Serial.println("UDP受信 → DMXスルー出力完了（安定版）");
  }
}
