#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         9
#define SS_PIN          10
#define ID_timeout      500
#define number_of_relay 12

MFRC522 mfrc522(SS_PIN, RST_PIN);
unsigned long int ID = 0;

void configure_master();
void master_operate();

void setup() {
  Serial.begin(9600);
  SPI.begin();
  configure_master();
  mfrc522.PCD_Init();
  delay(10);
  mfrc522.PCD_DumpVersionToSerial();
}

void loop() {
  master_operate();
  mehmetcan_loop();
}

unsigned long t = 0;
void mehmetcan_loop() {
  if ((millis() - t) > ID_timeout) {
    ID = 0;
  }

  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (i == 0) ID = 0;
    ID *= 16 * 16;
    ID += mfrc522.uid.uidByte[i];

  }
  t = millis();
}
