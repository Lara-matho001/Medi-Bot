#ifndef DISPENSER_CONFIG_H
#define DISPENSER_CONFIG_H

#include <Arduino.h>
#include "pin_config.h"
#include "motor_settings.h"

// Shared non-motor, non-pin settings live here.
// Use pin_config.h for wiring changes.
// Use motor_settings.h for stepper and servo tuning.

// RFID disabled for motor/dispense testing.
// const unsigned long DISPENSER_RFID_SCAN_TIMEOUT_MS = 10000;

// Timing windows for pill detection.
const unsigned long DISPENSER_DISPENSE_WINDOW_MS = 5000;
const unsigned long DISPENSER_CHUTE_CLEAR_TIMEOUT_MS = 2000;

// Serial speed used by the controller sending commands to this Arduino.
const unsigned long DISPENSER_SERIAL_BAUD = 9600;

// Tracks where the stepper thinks it is after homing.
extern int dispenserCurrentStepPosition;

#endif
