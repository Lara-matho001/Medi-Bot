#ifndef STEPPER_CONTROL_H
#define STEPPER_CONTROL_H

#include <Arduino.h>

// Sets the stepper driver pins as outputs.
void initialise_stepper();

// Energises the stepper coils so the motor can move and hold its position.
void stepper_enable();

// De-energises the stepper coils (motor free, no holding current / no heat).
void stepper_disable();

// Moves the stepper a requested number of logical steps.
void stepper_step(bool clockwise, int steps);

// Finds the home position using the home IR sensor.
bool home_stepper();

#endif
