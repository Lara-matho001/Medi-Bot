#ifndef RFID_READER_H
#define RFID_READER_H

#include <Arduino.h>

// ----- RFID / buzzer pin map (Arduino Mega 2560) -----
// The MFRC522 uses the Mega's hardware SPI bus: MOSI=51, MISO=50, SCK=52.
// SS and RST are the only two extra pins it needs.
const byte rfid_ss_pin = 4;     // MFRC522 SDA/SS
const byte rfid_rst_pin = 5;    // MFRC522 RST
const byte buzzer_pin = 10;     // active/passive buzzer for the wrong-patient alert

// On a Mega the hardware SPI slave-select pin (53) must be held HIGH/OUTPUT
// so the board stays in SPI master mode.
const byte mega_spi_ss_pin = 53;

// Minimum gap between repeated reports of the same tag. The poll is run every
// loop, so without this a tag left near the reader would be printed dozens of
// times per second. 1 s matches the original sketch's behaviour.
const unsigned long rfid_report_interval_ms = 1000;

// Buzzer alert tone (wrong patient / human intervention needed).
const unsigned int buzzer_tone_hz = 1000;
// Default beep length when "z" is sent with no duration. A duration can be given
// as "z <ms>" (e.g. the Pi sends "z 3000" as the long centre beep of its
// short/long/short "all RFID attempts failed" SOS).
const unsigned long buzzer_short_ms = 300;

// Powers up the RFID reader and prepares the buzzer pin.
void initialise_rfid();

// Non-blocking: call every loop. Prints "RFID:AA BB CC DD" when a new tag is
// seen, rate-limited by rfid_report_interval_ms.
void rfid_poll_and_report();

// Blocking buzzer beep for `duration_ms` milliseconds.
void rfid_beep_alert(unsigned long duration_ms);

#endif
