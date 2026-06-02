#ifndef DISPENSER_MOTOR_SETTINGS_H
#define DISPENSER_MOTOR_SETTINGS_H

#include <Arduino.h>

// Change motor/mechanical behaviour here.

// Stepper calibration.
const int STEPS_PER_REV = 200;

// Compartment positions measured as steps clockwise from home.
// Edit these numbers while tuning the rotary plate.
const byte DISPENSER_SLOT_COUNT = 5;
const int DISPENSER_COMPARTMENT_STEPS[DISPENSER_SLOT_COUNT] = {
    8,    // compartment 0
    48,   // compartment 1
    88,   // compartment 2
    128,  // compartment 3
    168   // compartment 4
};

// Controls stepper speed. Smaller delay = faster motor, but less torque.
// This is used as both the HIGH and LOW time for each STEP pulse.
const unsigned int STEPPER_STEP_DELAY_US = 10000;

// Pause between separate motor actions so movement is easier to see and debug.
const unsigned long DISPENSER_MOTOR_GAP_MS = 3000;

// Most common STEP/DIR drivers enable when EN is LOW.
const byte STEPPER_ENABLE_ACTIVE_STATE = LOW;
const byte STEPPER_ENABLE_INACTIVE_STATE = HIGH;

// Direction pin states. Swap these if the carousel turns the wrong way.
const byte STEPPER_CLOCKWISE_STATE = HIGH;
const byte STEPPER_COUNTERCLOCKWISE_STATE = LOW;

// Positional servo angles.
// For standard PWM servos, write(180) means move to 180 degrees and hold.
// write(0) means move back to 0 degrees and hold.
const int SERVO_A_HOME_POS = 0;
const int DISPENSER_SERVO_A_ACTIVE_POS = 180;
const int SERVO_B_HOME_POS = 0;
const int DISPENSER_SERVO_B_ACTIVE_POS = 180;

// Time to wait after each positional servo command so the servo can reach its angle.
const unsigned long SERVO_MOVE_DELAY_MS = 1000;

#endif
