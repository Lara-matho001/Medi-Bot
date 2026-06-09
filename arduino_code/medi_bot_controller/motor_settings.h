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
    22,    // compartment 1
    62,    // compartment 2
    102,   // compartment 3
    142,   // compartment 4
    182    // compartment 5
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

// ===========================================================================
// SERVO CONTROL
//
// The dispenser servos are Hitec HS-322HD — STANDARD POSITIONAL servos.
// write(angle) moves the horn to that angle (roughly 0-180 degrees) and holds.
// They do NOT spin continuously: a stock HS-322HD has ~180 degrees of travel
// and internal stops. So the values below are physical ANGLES, not speeds.
//
// Tune the dispense motion by editing these angles and the per-move delay:
//   - sweep too small/large  -> change the *_active_pos angle
//   - servo not finished moving before the next step -> raise servo_move_delay_ms
// ===========================================================================
const int servo_a_home_pos = 0;                 // Servo A rest angle
const int dispenser_servo_a_active_pos = 180;   // Servo A dispense angle
const int servo_b_home_pos = 0;                 // Servo B retracted (gear out)
const int dispenser_servo_b_active_pos = 180;   // Servo B engaged (gear in)

// Time to wait after each servo command so it can reach the angle before the
// next move. HS-322HD turns ~60 degrees in ~0.15 s, so a full 180 sweep takes
// ~0.5 s; 1000 ms leaves a safe margin.
const unsigned long servo_move_delay_ms = 1000;

#endif
