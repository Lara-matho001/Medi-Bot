#include "servo_control.h"
#include "config.h"

// Global servo objects used by the dispense code.
Servo dispenserServoA;
Servo dispenserServoB;

void initialise_servos() {

    // Connect each Servo object to its physical Arduino pin.
    dispenserServoA.attach(DISPENSER_SERVO_A_PIN);
    dispenserServoB.attach(DISPENSER_SERVO_B_PIN);

    // Start each positional servo at its configured home angle.
    dispenserServoA.write(DISPENSER_SERVO_A_HOME_POS);

    dispenserServoB.write(DISPENSER_SERVO_B_HOME_POS);

    Serial.println("DEBUG:SERVOS_READY");
}

void move_servo_to(Servo &servo, const char *label, int position) {

    Serial.print("DEBUG:");
    Serial.print(label);
    Serial.print("_MOVE_TO=");
    Serial.println(position);

    servo.write(position);
    delay(DISPENSER_SERVO_MOVE_DELAY_MS);
}
