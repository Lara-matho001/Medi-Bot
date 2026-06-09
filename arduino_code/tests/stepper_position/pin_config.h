#ifndef STEPPER_TEST_PIN_CONFIG_H
#define STEPPER_TEST_PIN_CONFIG_H

#include <Arduino.h>

// Change stepper and sensor wiring here.
const byte STEPPER_STEP_PIN = 23;
const byte STEPPER_DIR_PIN = 22;
const byte STEPPER_ENABLE_PIN = 24;

// Home IR sensor. Uses INPUT_PULLUP, so LOW means the home marker is detected.
const byte HOME_SENSOR_PIN = ;

#endif
