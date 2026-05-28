#include "stepper_control.h"
#include "config.h"

int stepSequence[4][4] = {
    {1,0,0,1},
    {1,0,0,0},
    {1,1,0,0},
    {0,1,0,0}
};

void initialise_stepper() {
    for (int i = 0; i < 4; i++) {
        pinMode(STEPPER_PINS[i], OUTPUT);
    }
}

void singleStep(int stepIndex) {
    for (int pin = 0; pin < 4; pin++) {
        digitalWrite(STEPPER_PINS[pin], stepSequence[stepIndex][pin]);
    }
}

void stepper_step(bool clockwise, int steps) {

    for (int i = 0; i < steps; i++) {

        for (int step = 0; step < 4; step++) {

            int idx = clockwise ? step : (3 - step);

            singleStep(idx);

            delayMicroseconds(2000);
        }
    }
}

bool home_stepper() {

    int step_count = 0;

    while (digitalRead(IR_HOME_PIN) == HIGH) {

        stepper_step(true, 1);

        step_count++;

        if (step_count > STEPS_PER_REV * 2) {
            Serial.println("ERROR:HOME_FAIL");
            return false;
        }
    }

    current_step_position = 0;

    return true;
}