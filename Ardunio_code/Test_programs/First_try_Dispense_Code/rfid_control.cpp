#include "rfid_control.h"
#include "config.h"

#include <SPI.h>
#include <MFRC522.h>

MFRC522 mfrc522(RFID_SS_PIN, RFID_RST_PIN);

void initialise_rfid() {

    SPI.begin();
    mfrc522.PCD_Init();
}

bool rfid_card_present() {

    return mfrc522.PICC_IsNewCardPresent() &&
           mfrc522.PICC_ReadCardSerial();
}

String rfid_read_uid() {

    String uid = "";

    for (byte i = 0; i < mfrc522.uid.size; i++) {

        if (mfrc522.uid.uidByte[i] < 0x10) {
            uid += "0";
        }

        uid += String(mfrc522.uid.uidByte[i], HEX);
    }

    uid.toUpperCase();

    return uid;
}