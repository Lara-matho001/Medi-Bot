#ifndef STEPPER_TEST_POSITION_SETTINGS_H
#define STEPPER_TEST_POSITION_SETTINGS_H

#include <Arduino.h>

// Change all positioning/tuning values here.

const unsigned long SERIAL_BAUD = 9600;

// STEP pulse timing. Larger = slower. This delay is used for HIGH and LOW time.
const unsigned int STEP_PULSE_DELAY_US = 10000;

// Direction/enable states. Swap CLOCKWISE and COUNTERCLOCKWISE if rotation is backwards.
const byte ENABLE_ACTIVE_STATE = LOW;
const byte ENABLE_INACTIVE_STATE = HIGH;
const byte CLOCKWISE_STATE = HIGH;
const byte COUNTERCLOCKWISE_STATE = LOW;

// Safety limit for homing. If home is not found before this many steps, stop.
const int MAX_HOME_STEPS = 600;

// Pause after homing and after moving to a compartment.
const unsigned long SETTLE_DELAY_MS = 3000;

// Compartment positions measured as steps clockwise from home.
// Edit these numbers while tuning the rotary plate.
const byte COMPARTMENT_COUNT = 5;
const int COMPARTMENT_STEPS[COMPARTMENT_COUNT] = {
    8,    // compartment 0
    48,   // compartment 1
    88,   // compartment 2
    128,  // compartment 3
    168   // compartment 4
};

#endif
