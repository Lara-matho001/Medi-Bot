#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include <Servo.h>

// Servo A spins the dispenser. Servo B engages/retracts the gear.
extern Servo dispenser_servo_a;
extern Servo dispenser_servo_b;

// Attaches the servo objects to their configured pins and sets starting positions.
void initialise_servos();

// Moves a positional servo to an angle and waits for it to arrive.
void move_servo_to(Servo &servo, const char *label, int position);

// (Re)attaches both servos for a dispense cycle so they hold their angle.
void engage_servos();

// Detaches both servos (stops PWM) so they draw no power once the cycle is done.
void relax_servos();

#endif
