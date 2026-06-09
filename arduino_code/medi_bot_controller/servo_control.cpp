#include "servo_control.h"
#include "config.h"

// Global servo objects used by the dispense code.
Servo dispenser_servo_a;
Servo dispenser_servo_b;

void initialise_servos() {

    // Connect each Servo object to its physical Arduino pin.
    dispenser_servo_a.attach(servo_a_pin);
    dispenser_servo_b.attach(servo_b_pin);

    // Standard positional servos: move each to its home angle at boot.
    dispenser_servo_a.write(servo_a_home_pos);
    dispenser_servo_b.write(servo_b_home_pos);

    Serial.println("DEBUG:SERVOS_READY");
}

void move_servo_to(Servo &servo, const char *label, int position) {

    Serial.print("DEBUG:");
    Serial.print(label);
    Serial.print("_MOVE_TO=");
    Serial.println(position);

    servo.write(position);
    delay(servo_move_delay_ms);
}

void engage_servos() {

    // Re-attach the servos for a dispense cycle. Each one resumes holding the
    // last angle it was commanded to (home, from the previous cycle or boot).
    dispenser_servo_a.attach(servo_a_pin);
    dispenser_servo_b.attach(servo_b_pin);
}

void relax_servos() {

    // Detach both servos so the Arduino stops sending PWM pulses. The servos
    // then draw no holding current and go limp — only do this once the dispense
    // cycle is fully complete.
    dispenser_servo_a.detach();
    dispenser_servo_b.detach();
    Serial.println("DEBUG:SERVOS_RELAXED");
}
