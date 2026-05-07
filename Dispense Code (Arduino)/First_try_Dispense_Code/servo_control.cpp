#include "servo_control.h"
#include "config.h"

Servo servoA;
Servo servoB;

void initialise_servos() {

    servoA.attach(SERVO_A_PIN);
    servoB.attach(SERVO_B_PIN);

    servoA.write(0);
    servoB.write(SERVO_B_RETRACT_POS);
}

void rotate_servo_degrees(int degrees) {

    // Continuous rotation servo version
    servoA.write(180);

    delay(1000);

    servoA.write(90);
}