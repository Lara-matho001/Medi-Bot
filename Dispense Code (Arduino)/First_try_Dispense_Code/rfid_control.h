#ifndef RFID_CONTROL_H
#define RFID_CONTROL_H

#include <Arduino.h>

void initialise_rfid();
bool rfid_card_present();
String rfid_read_uid();

#endif