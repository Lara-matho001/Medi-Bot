#ifndef DISPENSE_H
#define DISPENSE_H

#include <Arduino.h>

// RFID disabled for motor/dispense testing.
// Waits for an RFID card and prints the UID over Serial.
// void handle_rfid_scan();

// Runs the complete dispense process for one compartment.
void handle_dispense(int compartment_id);

// Moves the carousel from the home position to the requested compartment.
void rotate_to_compartment(int compartment_id);

// Runs the servo/chute-sensor dispense check and returns OK, MISS, or OVER_DISPENSE.
String run_dispense_cycle();

// Stops normal operation after a serious dispense error until RESET is received.
void enter_halt_state();

#endif
