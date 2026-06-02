// RFID disabled for motor/dispense testing.
// #include "rfid_control.h"
// #include "config.h"
//
// #include <SPI.h>
// #include <MFRC522.h>
//
// // RFID reader object from the MFRC522 library.
// MFRC522 mfrc522(DISPENSER_RFID_SS_PIN, DISPENSER_RFID_RST_PIN);
//
// void initialise_rfid() {
//
//     // The MFRC522 communicates over SPI.
//     SPI.begin();
//
//     // Put the reader into a ready state.
//     mfrc522.PCD_Init();
// }
//
// bool rfid_card_present() {
//
//     // Both checks are needed: first detect a card, then read its serial/UID data.
//     return mfrc522.PICC_IsNewCardPresent() &&
//            mfrc522.PICC_ReadCardSerial();
// }
//
// String rfid_read_uid() {
//
//     String uid = "";
//
//     // Build the UID byte-by-byte as hex text.
//     for (byte i = 0; i < mfrc522.uid.size; i++) {
//
//         // Add a leading zero so every byte is always two hex characters.
//         if (mfrc522.uid.uidByte[i] < 0x10) {
//             uid += "0";
//         }
//
//         uid += String(mfrc522.uid.uidByte[i], HEX);
//     }
//
//     // Make the output easier to compare with stored card IDs.
//     uid.toUpperCase();
//
//     return uid;
// }
