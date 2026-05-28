#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

byte block = 4;
byte buffer[18];
byte size = 18;

void setup() {

  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(" MEDIBOT IS HERE! ");
  Serial.println(" Tap Your Band Please:) ");
}

void loop() {

  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }

  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  Serial.println("\nRFID DETECTED");

  Serial.print("UID: ");

  for (byte i = 0; i < rfid.uid.size; i++) {

    if (rfid.uid.uidByte[i] < 0x10) {
      Serial.print("0");
    }

    Serial.print(rfid.uid.uidByte[i], HEX);
    Serial.print(" ");
  }

  Serial.println();

  MFRC522::StatusCode status;

  status = rfid.PCD_Authenticate(
             MFRC522::PICC_CMD_MF_AUTH_KEY_A,
             block,
             &key,
             &(rfid.uid)
           );

  if (status != MFRC522::STATUS_OK) {

    Serial.print("Authentication Failed: ");
    Serial.println(rfid.GetStatusCodeName(status));

    stopRFID();
    return;
  }
  status = rfid.MIFARE_Read(block, buffer, &size);

  if (status != MFRC522::STATUS_OK) {

    Serial.print("Read Failed: ");
    Serial.println(rfid.GetStatusCodeName(status));

    stopRFID();
    return;
  }

  String patientData = "";

  for (byte i = 0; i < 16; i++) {

    if (buffer[i] != 0) {
      patientData += (char)buffer[i];
    }
  }

  Serial.print("Patient Data: ");
  Serial.println(patientData);

  if (patientData == "Patient1") {
    Serial.println("Dispense Medicine for Patient 1");
  }

  else if (patientData == "Patient2") {
    Serial.println("Dispense Medicine for Patient 2");
  }

  else if (patientData == "Patient3") {
    Serial.println("Dispense Medicine for Patient 3");
  }

  else if (patientData == "Patient4") {
    Serial.println("Dispense Medicine for Patient 4");
  }

  else if (patientData == "Patient5") {
    Serial.println("Dispense Medicine for Patient 5");
  }

  else {
    Serial.println("Unknown Card");
  }

  stopRFID();

  delay(2000);
}

void stopRFID() {
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}