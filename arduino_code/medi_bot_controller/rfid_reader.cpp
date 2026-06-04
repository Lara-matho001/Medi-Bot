#include "rfid_reader.h"
#include <SPI.h>
#include <MFRC522.h>

// The reader object and the report timer are private to this file.
static MFRC522 rfid(rfid_ss_pin, rfid_rst_pin);
static unsigned long last_report_ms = 0;

void initialise_rfid() {

    // Keep the Mega in SPI master mode.
    pinMode(mega_spi_ss_pin, OUTPUT);
    digitalWrite(mega_spi_ss_pin, HIGH);

    pinMode(rfid_ss_pin, OUTPUT);
    digitalWrite(rfid_ss_pin, HIGH);

    pinMode(buzzer_pin, OUTPUT);
    digitalWrite(buzzer_pin, LOW);

    SPI.begin();
    rfid.PCD_Init();
    delay(500);

    rfid.PCD_AntennaOn();
    rfid.PCD_SetAntennaGain(MFRC522::RxGain_max);

    Serial.println("DEBUG:RFID_READY");
    // Prints the reader firmware version - handy for confirming the wiring.
    rfid.PCD_DumpVersionToSerial();
}

void rfid_poll_and_report() {

    // Nothing to do unless a new card is in range and readable.
    if (!rfid.PICC_IsNewCardPresent()) {
        return;
    }

    if (!rfid.PICC_ReadCardSerial()) {
        return;
    }

    // Rate-limit so the same tag is not reported every loop while it is held
    // near the reader. We still halt the card so it can be re-detected later.
    if (millis() - last_report_ms < rfid_report_interval_ms) {
        rfid.PICC_HaltA();
        rfid.PCD_StopCrypto1();
        return;
    }
    last_report_ms = millis();

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
}

void rfid_beep_alert() {

    // NOTE: tone() uses Timer2, which on the Mega also drives PWM on pins 9
    // and 10 (the motor FORWARD lines). The beep is meant to sound while the
    // robot is stopped at a patient, so this is fine - the next motor command
    // re-enables PWM on those pins. Avoid beeping while driving forward.
    tone(buzzer_pin, buzzer_tone_hz);
    delay(buzzer_alert_ms);
    noTone(buzzer_pin);
}
