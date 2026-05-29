#ifndef DISPENSE_H
#define DISPENSE_H

#include <Arduino.h>

void handle_rfid_scan();
void handle_dispense(int compartment_id);

void rotate_to_compartment(int compartment_id);

String run_dispense_cycle();

void enter_halt_state();

#endif