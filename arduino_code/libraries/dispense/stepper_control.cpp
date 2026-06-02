#include "stepper_control.h"
#include "config.h"

void initialise_stepper() {
    // STEP/DIR/ENABLE drivers only need three Arduino outputs.
    pinMode(stepper_step_pin, OUTPUT);
    pinMode(stepper_direction_pin, OUTPUT);
    pinMode(stepper_enable_pin, OUTPUT);

    // Start with the driver enabled so the motor can move and hold position.
    digitalWrite(stepper_enable_pin, stepper_enable_active_state);
    digitalWrite(stepper_step_pin, LOW);
    digitalWrite(stepper_direction_pin, stepper_clockwise_state);

    Serial.println("DEBUG:STEPPER_READY");
}

void stepper_step(bool clockwise, int steps) {

    if (steps <= 0) {
        Serial.println("DEBUG:STEPPER_NO_STEPS_REQUESTED");
        return;
    }

    // Set the direction once before sending step pulses.
    digitalWrite(
        stepper_direction_pin,
        clockwise ? stepper_clockwise_state : stepper_counterclockwise_state
    );

    // Make sure the driver is enabled before moving.
    digitalWrite(stepper_enable_pin, stepper_enable_active_state);

    for (int i = 0; i < steps; i++) {

        // A STEP/DIR driver moves one step on the rising edge of the STEP pulse.
        digitalWrite(stepper_step_pin, HIGH);
        delayMicroseconds(stepper_step_delay_us);
        digitalWrite(stepper_step_pin, LOW);
        delayMicroseconds(stepper_step_delay_us);
    }

    // De-energise the coils after movement to prevent heat buildup while idle.
    digitalWrite(stepper_enable_pin, stepper_enable_inactive_state);
}

bool home_stepper() {

    int step_count = 0;
    Serial.print("DEBUG:HOME_SENSOR_START=");
    Serial.println(digitalRead(ir_homing_pin));

    // With INPUT_PULLUP, HIGH means the home sensor is not triggered yet.
    while (digitalRead(ir_homing_pin) == HIGH) {

        // Move slowly until the home sensor is reached.
        stepper_step(true, 1);

        step_count++;

        // If it rotates more than two full turns without finding home, something is wrong.
        if (step_count > steps_per_rev * 2) {
            Serial.print("DEBUG:HOME_FAIL_STEPS=");
            Serial.println(step_count);
            Serial.println("ERROR:HOME_FAIL");
            return false;
        }
    }

    Serial.print("DEBUG:HOME_FOUND_STEPS=");
    Serial.println(step_count);

    return true;
}
