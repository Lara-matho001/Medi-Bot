#include "stepper_driver.h"
#include "pin_config.h"
#include "position_settings.h"

int currentStepPosition = 0;

void initialise_stepper() {

    pinMode(STEPPER_STEP_PIN, OUTPUT);
    pinMode(STEPPER_DIR_PIN, OUTPUT);
    pinMode(STEPPER_ENABLE_PIN, OUTPUT);
    pinMode(HOME_SENSOR_PIN, INPUT_PULLUP);

    digitalWrite(STEPPER_ENABLE_PIN, ENABLE_ACTIVE_STATE);
    digitalWrite(STEPPER_STEP_PIN, LOW);
    digitalWrite(STEPPER_DIR_PIN, CLOCKWISE_STATE);

    Serial.println("DEBUG:STEPPER_READY");
}

void stepper_step(bool clockwise, int steps) {

    if (steps <= 0) {
        Serial.println("DEBUG:NO_STEPS_REQUESTED");
        return;
    }

    digitalWrite(STEPPER_DIR_PIN, clockwise ? CLOCKWISE_STATE : COUNTERCLOCKWISE_STATE);
    digitalWrite(STEPPER_ENABLE_PIN, ENABLE_ACTIVE_STATE);

    for (int i = 0; i < steps; i++) {
        digitalWrite(STEPPER_STEP_PIN, HIGH);
        delayMicroseconds(STEP_PULSE_DELAY_US);
        digitalWrite(STEPPER_STEP_PIN, LOW);
        delayMicroseconds(STEP_PULSE_DELAY_US);
    }
}

bool home_stepper() {

    Serial.println("DEBUG:HOMING_START");
    Serial.print("DEBUG:HOME_SENSOR_START=");
    Serial.println(digitalRead(HOME_SENSOR_PIN));

    int stepsTaken = 0;

    while (digitalRead(HOME_SENSOR_PIN) == HIGH) {
        stepper_step(true, 1);
        stepsTaken++;

        if (stepsTaken > MAX_HOME_STEPS) {
            Serial.print("DEBUG:HOME_FAIL_STEPS=");
            Serial.println(stepsTaken);
            Serial.println("ERROR:HOME_FAIL");
            return false;
        }
    }

    currentStepPosition = 0;

    Serial.print("DEBUG:HOME_FOUND_STEPS=");
    Serial.println(stepsTaken);
    Serial.println("OK:HOME");

    delay(SETTLE_DELAY_MS);
    return true;
}

bool move_to_compartment(int compartment) {

    if (compartment < 0 || compartment >= COMPARTMENT_COUNT) {
        Serial.print("ERROR:BAD_COMPARTMENT=");
        Serial.println(compartment);
        return false;
    }

    int targetSteps = COMPARTMENT_STEPS[compartment];

    Serial.print("DEBUG:MOVE_TO_COMPARTMENT=");
    Serial.println(compartment);
    Serial.print("DEBUG:TARGET_STEPS_FROM_HOME=");
    Serial.println(targetSteps);

    stepper_step(true, targetSteps);
    currentStepPosition = targetSteps;

    Serial.print("OK:AT_COMPARTMENT=");
    Serial.println(compartment);

    delay(SETTLE_DELAY_MS);
    return true;
}
