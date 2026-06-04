#ifndef DISPENSER_MOTOR_SETTINGS_H
#define DISPENSER_MOTOR_SETTINGS_H

#include <Arduino.h>

// Change motor/mechanical behaviour here.

// Stepper calibration.
const int steps_per_rev = 200;

// Compartment positions measured as steps clockwise from home.
// Edit these numbers while tuning the rotary plate.
const byte dispenser_slot_count = 5;
const int dispenser_compartment_steps[dispenser_slot_count] = {
    8,    // compartment 0
    48,   // compartment 1
    88,   // compartment 2
    128,  // compartment 3
    168   // compartment 4
};

// Controls stepper speed. Smaller delay = faster motor, but less torque.
// This is used as both the HIGH and LOW time for each STEP pulse.
const unsigned int stepper_step_delay_us = 10000;

// Pause between separate motor actions so movement is easier to see and debug.
const unsigned long dispenser_motor_gap_ms = 3000;

// Most common STEP/DIR drivers enable when EN is LOW.
const byte stepper_enable_active_state = LOW;
const byte stepper_enable_inactive_state = HIGH;

// Direction pin states. Swap these if the carousel turns the wrong way.
const byte stepper_clockwise_state = HIGH;
const byte stepper_counterclockwise_state = LOW;

// Positional servo angles.
// For standard PWM servos, write(180) means move to 180 degrees and hold.
// write(0) means move back to 0 degrees and hold.
const int servo_a_home_pos = 0;
const int dispenser_servo_a_active_pos = 180;
const int servo_b_home_pos = 0;
const int dispenser_servo_b_active_pos = 180;

// Time to wait after each positional servo command so the servo can reach its angle.
const unsigned long servo_move_delay_ms = 1000;

#endif
