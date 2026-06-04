#ifndef STEPPER_CONTROL_H
#define STEPPER_CONTROL_H

#include <Arduino.h>

// Sets the stepper driver pins as outputs.
void initialise_stepper();

// Moves the stepper a requested number of logical steps.
void stepper_step(bool clockwise, int steps);

// Finds the home position using the home IR sensor.
bool home_stepper();

#endif
