#ifndef DISPENSER_CONFIG_H
#define DISPENSER_CONFIG_H

#include <Arduino.h>
#include "motor_settings.h"

// Shared non-motor, non-pin settings live here.
// Use pin_config.h for wiring changes.
// Use motor_settings.h for stepper and servo tuning.

// How long to monitor the chute IR for late-falling pills after servos return home.
const unsigned long post_dispense_monitor_ms = 5000;

// How long a single chute IR trigger is allowed to stay blocked before timing out.
const unsigned long chute_clear_timeout_ms = 2000;

// How many times to retry a MISS before giving up and reporting an error.
const int dispense_max_retries = 5;

// How long to wait for the patient to take the medication cup after a successful dispense.
const unsigned long cup_taken_timeout_ms = 30000;

// Serial speed used by the controller sending commands to this Arduino.
const unsigned long serial_baud = 9600;

// Change wiring here.
// Main ROSArduinoBridge uses D2, D3, D5, D6, D9, D10, D12, D13, A4, and A5.
// These dispenser pins are selected for an Arduino Mega so they do not overlap.

// IR sensors are wired with INPUT_PULLUP, so LOW means the sensor is triggered.
const byte ir_homing_pin = 11;
const byte ir_pill_detection_pin = 12;
// Cup sensor: LOW = cup present/blocking beam, HIGH = cup has been taken.
const byte ir_medication_cup_pin = 13;

// Servo A spins the dispensing mechanism. Servo B engages/retracts the gear.
// pins 26 & 28
const byte servo_a_pin = 26;
const byte servo_b_pin = 28;

// Stepper driver pins for a STEP/DIR/ENABLE style driver.
// STEP receives one pulse for each motor step.
// DIR selects the motor direction.
// EN enables the driver. Most A4988/DRV8825-style drivers use LOW = enabled.
const byte stepper_direction_pin = 22;
const byte stepper_step_pin = 23;
const byte stepper_enable_pin = 24;

#endif
