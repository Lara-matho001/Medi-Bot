#ifndef DISPENSER_CONFIG_H
#define DISPENSER_CONFIG_H

#include <Arduino.h>
#include "motor_settings.h"

// Shared non-motor, non-pin dispenser settings live here.
// Use motor_settings.h for stepper and servo tuning.
//
// NOTE: the serial baud rate is NOT set here. In the combined firmware the
// ROSArduinoBridge owns the serial port and opens it at BAUDRATE (57600) in
// medi_bot_controller.ino. The Raspberry Pi must connect at 57600.

// How long to monitor the chute IR for late-falling pills after servos return home.
const unsigned long post_dispense_monitor_ms = 5000;

// How long a single chute IR trigger is allowed to stay blocked before timing out.
const unsigned long chute_clear_timeout_ms = 2000;

// How many times to retry a MISS before giving up and reporting an error.
const int dispense_max_retries = 5;

// How long to wait for the patient to take the medication cup after a successful dispense.
const unsigned long cup_taken_timeout_ms = 30000;

// ----- Dispenser pin map (Arduino Mega 2560) -----
// These pins are chosen so the dispenser does not clash with the base
// controller. The ROSArduinoBridge base uses pins 5, 6, 9, 10 (motor PWM)
// plus its encoder pins. Its motor ENABLE lines were moved off pins 12/13
// to spare pins (see motor_driver.h) so the IR sensors below can keep 12/13.
// The RFID reader uses pins 7, 8, 50-53 and the buzzer pin 4 (see rfid_reader.h).

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
