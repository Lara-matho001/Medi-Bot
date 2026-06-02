#ifndef STEPPER_TEST_DRIVER_H
#define STEPPER_TEST_DRIVER_H

#include <Arduino.h>

void initialise_stepper();
void stepper_step(bool clockwise, int steps);
bool home_stepper();
bool move_to_compartment(int compartment);

#endif
