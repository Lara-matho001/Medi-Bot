#ifndef STEPPER_CONTROL_H
#define STEPPER_CONTROL_H

#include <Arduino.h>

void initialise_stepper();
void stepper_step(bool clockwise, int steps);
bool home_stepper();

#endif