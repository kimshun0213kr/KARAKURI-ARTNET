#include <WiFi.h>
#include <WiFiUdp.h>

// WiFi設定
const char *ssid = "KARAKURI_ARTNET";
WiFiUDP udp;
const int udpPort = 12345;

// DMX用ピン
#define DMX_TX_PIN 7      // DIピン
#define DMX_DIR_PIN 8     // DE/REピン（送信時はHIGH）

HardwareSerial dmxSerial(1);
uint8_t dmxData[513];  // 0番目はStartCode（常に0）

void setup() {
  Serial.begin(115200);

  // DE/REピン設定
  pinMode(DMX_DIR_PIN, OUTPUT);
  digitalWrite(DMX_DIR_PIN, LOW);  // 初期は受信方向（安全）

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
  Serial.println("UDP受信開始");

  // UART初期化（DMX用）
  dmxSerial.begin(250000, SERIAL_8N2, -1, DMX_TX_PIN);  // RXは使わない
}

void sendDMX() {
  // DMX送信
  digitalWrite(DMX_DIR_PIN, HIGH);  // 送信モード

  // Break信号（88us LOW）
  Serial.flush();  // 念のため
  dmxSerial.flush();
  dmxSerial.end();
  pinMode(DMX_TX_PIN, OUTPUT);
  digitalWrite(DMX_TX_PIN, LOW);
  delayMicroseconds(100);  // Break
  digitalWrite(DMX_TX_PIN, HIGH);
  delayMicroseconds(12);   // Mark After Break (MAB)
  dmxSerial.begin(250000, SERIAL_8N2, -1, DMX_TX_PIN);  // 再初期化

  // データ送信（StartCode + 512チャンネル）
  dmxSerial.write(dmxData, 513);

  dmxSerial.flush();  // 送信完了まで待つ
  digitalWrite(DMX_DIR_PIN, LOW);  // 受信モードに戻す（他と共有してる場合）
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize == 512) {
    udp.read(dmxData + 1, 512);  // StartCodeは常に0なので +1
    dmxData[0] = 0;              // StartCode = 0 (standard DMX)
    Serial.print(dmxData[1]);

    sendDMX();
    Serial.println("\tUDP受信 → DMX出力完了");
  }

  delay(1);  // 少し待つ
}
