#ifndef DISPENSER_PIN_CONFIG_H
#define DISPENSER_PIN_CONFIG_H

#include <Arduino.h>

// Change wiring here.
// Main ROSArduinoBridge uses D2, D3, D5, D6, D9, D10, D12, D13, A4, and A5.
// These dispenser pins are selected for an Arduino Mega so they do not overlap.

// IR sensors are wired with INPUT_PULLUP, so LOW means the sensor is triggered.
const byte DISPENSER_IR_HOME_PIN = 7;
const byte DISPENSER_IR_CHUTE_PIN = 6;

// Servo A spins the dispensing mechanism. Servo B engages/retracts the gear.
const byte DISPENSER_SERVO_A_PIN = 10;
const byte DISPENSER_SERVO_B_PIN = 9;

// RFID disabled for motor/dispense testing.
// MFRC522 RFID module pins. SS must match the SPI chip-select pin used by the reader.
// const byte DISPENSER_RFID_SS_PIN = 53;
// const byte DISPENSER_RFID_RST_PIN = 49;

// Stepper driver pins for a STEP/DIR/ENABLE style driver.
// STEP receives one pulse for each motor step.
// DIR selects the motor direction.
// EN enables the driver. Most A4988/DRV8825-style drivers use LOW = enabled.
const byte DISPENSER_STEPPER_STEP_PIN = 3;
const byte DISPENSER_STEPPER_DIR_PIN = 4;
const byte DISPENSER_STEPPER_ENABLE_PIN = 8;

#endif
