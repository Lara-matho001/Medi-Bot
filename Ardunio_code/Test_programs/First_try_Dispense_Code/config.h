#ifndef DISPENSER_CONFIG_H
#define DISPENSER_CONFIG_H

#include <Arduino.h>
#include "motor_settings.h"

// Shared non-motor, non-pin settings live here.
// Use pin_config.h for wiring changes.
// Use motor_settings.h for stepper and servo tuning.

// Timing windows for pill detection.
const unsigned long DISPENSE_WINDOW_MS = 5000;
const unsigned long CHUTE_CLEAR_TIMEOUT_MS = 2000;

// Serial speed used by the controller sending commands to this Arduino.
const unsigned long SERIAL_BAUD = 9600;

// Tracks where the stepper thinks it is after homing.
extern int dispenserCurrentStepPosition;


// Change wiring here.
// Main ROSArduinoBridge uses D2, D3, D5, D6, D9, D10, D12, D13, A4, and A5.
// These dispenser pins are selected for an Arduino Mega so they do not overlap.

// IR sensors are wired with INPUT_PULLUP, so LOW means the sensor is triggered.
const byte IR_HOMEING_PIN = 7;
const byte IR_PILL_DETECTION_PIN = 6;

// Servo A spins the dispensing mechanism. Servo B engages/retracts the gear.
const byte SERVO_A_PIN = 10;
const byte SERVO_B_PIN = 9;

// Stepper driver pins for a STEP/DIR/ENABLE style driver.
// STEP receives one pulse for each motor step.
// DIR selects the motor direction.
// EN enables the driver. Most A4988/DRV8825-style drivers use LOW = enabled.
const byte STEPPER_STEP_PIN = 3;
const byte STEPPER_DIRECTION_PIN = 4;
const byte STEPPER_ENABLE_PIN = 8;

#endif
