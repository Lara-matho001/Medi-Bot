#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include <Servo.h>

extern Servo servoA;
extern Servo servoB;

void initialise_servos();
void rotate_servo_degrees(int degrees);

#endif