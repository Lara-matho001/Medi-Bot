#include "stepper_control.h"
#include "config.h"

void initialise_stepper() {
    // STEP/DIR/ENABLE drivers only need three Arduino outputs.
    pinMode(STEPPER_STEP_PIN, OUTPUT);
    pinMode(STEPPER_DIRECTION_PIN, OUTPUT);
    pinMode(STEPPER_ENABLE_PIN, OUTPUT);

    // Start with the driver enabled so the motor can move and hold position.
    digitalWrite(STEPPER_ENABLE_PIN, STEPPER_ENABLE_ACTIVE_STATE);
    digitalWrite(STEPPER_STEP_PIN, LOW);
    digitalWrite(STEPPER_DIRECTION_PIN, STEPPER_CLOCKWISE_STATE);

    Serial.println("DEBUG:STEPPER_READY");
}

void stepper_step(bool clockwise, int steps) {

    if (steps <= 0) {
        Serial.println("DEBUG:STEPPER_NO_STEPS_REQUESTED");
        return;
    }

    // Set the direction once before sending step pulses.
    digitalWrite(
        STEPPER_DIRECTION_PIN,
        clockwise ? STEPPER_CLOCKWISE_STATE : STEPPER_COUNTERCLOCKWISE_STATE
    );

    // Make sure the driver is enabled before moving.
    digitalWrite(STEPPER_ENABLE_PIN, STEPPER_ENABLE_ACTIVE_STATE);

    for (int i = 0; i < steps; i++) {

        // A STEP/DIR driver moves one step on the rising edge of the STEP pulse.
        digitalWrite(STEPPER_STEP_PIN, HIGH);
        delayMicroseconds(STEPPER_STEP_DELAY_US);
        digitalWrite(STEPPER_STEP_PIN, LOW);
        delayMicroseconds(STEPPER_STEP_DELAY_US);
    }
}

bool home_stepper() {

    int step_count = 0;
    Serial.print("DEBUG:HOME_SENSOR_START=");
    Serial.println(digitalRead(IR_HOMEING_PIN));

    // With INPUT_PULLUP, HIGH means the home sensor is not triggered yet.
    while (digitalRead(IR_HOMEING_PIN) == HIGH) {

        // Move slowly until the home sensor is reached.
        stepper_step(true, 1);

        step_count++;

        // If it rotates more than two full turns without finding home, something is wrong.
        if (step_count > STEPS_PER_REV * 2) {
            Serial.print("DEBUG:HOME_FAIL_STEPS=");
            Serial.println(step_count);
            Serial.println("ERROR:HOME_FAIL");
            return false;
        }
    }

    // Home sensor was found, so this physical position becomes step zero.
    dispenserCurrentStepPosition = 0;

    Serial.print("DEBUG:HOME_FOUND_STEPS=");
    Serial.println(step_count);

    return true;
}
