#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 8
#define RST_PIN 7
#define BUZZER_PIN 4

MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(9600);

  // Required for Arduino Mega SPI
  pinMode(53, OUTPUT);
  digitalWrite(53, HIGH);

  pinMode(SS_PIN, OUTPUT);
  digitalWrite(SS_PIN, HIGH);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  SPI.begin();
  rfid.PCD_Init();
  delay(500);

  rfid.PCD_AntennaOn();
  rfid.PCD_SetAntennaGain(MFRC522::RxGain_max);

  Serial.println("RFID reader ready");
  rfid.PCD_DumpVersionToSerial();
}

void loop() {
  // Receive response from Raspberry Pi
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "WRONG") {
      Serial.println("Wrong RFID received from Pi");
      beepWrongRFID();
    } 
    else if (command == "CORRECT") {
      Serial.println("Correct RFID received from Pi");
      // Add pill dispensing code here later
    }
  }

  // Read RFID tag
  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }

  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  Serial.print("RFID:");

  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) {
      Serial.print("0");
    }

    Serial.print(rfid.uid.uidByte[i], HEX);

    if (i < rfid.uid.size - 1) {
      Serial.print(" ");
    }
  }

  Serial.println();

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  delay(1000);
}

void beepWrongRFID() {
  tone(BUZZER_PIN, 1000);
  delay(4000);
  noTone(BUZZER_PIN);
}