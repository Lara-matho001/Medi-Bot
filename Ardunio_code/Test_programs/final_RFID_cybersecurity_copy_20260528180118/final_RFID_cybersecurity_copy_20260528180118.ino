#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

byte block = 4;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println("MEDIBOT RFID UID AUTHENTICATION READY");
  Serial.println("Tap your RFID band...");
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }

  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  MFRC522::StatusCode status;

  status = rfid.PCD_Authenticate(
    MFRC522::PICC_CMD_MF_AUTH_KEY_A,
    block,
    &key,
    &(rfid.uid)
  );

  if (status != MFRC522::STATUS_OK) {
    Serial.println("AUTH_FAILED");
    stopRFID();
    delay(1000);
    return;
  }

  printUID();

  stopRFID();
  delay(1000);
}

void printUID() {
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) {
      Serial.print("0");
    }

    Serial.print(rfid.uid.uidByte[i], HEX);
  }

  Serial.println();
}

void stopRFID() {
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}