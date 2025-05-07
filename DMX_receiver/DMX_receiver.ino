#include <Arduino.h>

#define DMX_RX_PIN 7     // MAX485のROピン（DMXデータ入力）

HardwareSerial dmxSerial(1);

const int DMX_CHANNELS = 512;
uint8_t dmxData[DMX_CHANNELS + 1];  // 0番はStartCode
int dmxIndex = 0;
bool receiving = false;
unsigned long lastByteTime = 0;

void setup() {
  Serial.begin(115200);

  // DMXデータ用シリアル初期化
  dmxSerial.begin(250000, -1, DMX_RX_PIN, -1);  // TXは使わない

  Serial.println("DMX受信開始...");
}

void loop() {
  while (dmxSerial.available()) {
    uint8_t byte = dmxSerial.read();

    unsigned long now = micros();
    // たとえば、1000us以上空いていたら新しいフレームのStartCodeと仮定
    if (now - lastByteTime > 1000) {
      dmxIndex = 0;
      receiving = true;
    }
    lastByteTime = now;

    if (receiving) {
      if (dmxIndex < DMX_CHANNELS + 1) {
        dmxData[dmxIndex] = byte;
        dmxIndex++;
      } else {
        receiving = false;
      }
    }
  }

  // チャンネル1の値を表示（StartCodeの次、つまりdmxData[1]）
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 20) {  // 20msごとに表示
    Serial.print("CH1: ");
    Serial.println(dmxData[1]);
    lastPrint = millis();
  }
}
