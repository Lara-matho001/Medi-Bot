#include "dispense.h"
#include "config.h"
#include "stepper_control.h"
#include "servo_control.h"
#include "rfid_control.h"

void handle_rfid_scan() {

    unsigned long start_time = millis();

    while (millis() - start_time < 10000) {

        if (rfid_card_present()) {

            String uid = rfid_read_uid();

            Serial.println("RFID:" + uid);

            return;
        }
    }

    Serial.println("RFID:TIMEOUT");
}

void rotate_to_compartment(int compartment_id) {

    int target_steps = compartment_id * STEPS_PER_SLOT;

    stepper_step(true, target_steps);

    current_step_position = target_steps;

    delay(200);
}

String run_dispense_cycle() {

    // Engage gear
    servoB.write(SERVO_B_ENGAGE_POS);

    delay(600);

    // Spin
    rotate_servo_degrees(SERVO_A_SPIN_DEG);

    delay(400);

    int trigger_count = 0;

    unsigned long window_start = millis();

    while (millis() - window_start < DISPENSE_WINDOW_MS) {

        if (digitalRead(IR_CHUTE_PIN) == LOW) {

            trigger_count++;

            while (digitalRead(IR_CHUTE_PIN) == LOW);

            delay(200);
        }
    }

    // Retract
    servoB.write(SERVO_B_RETRACT_POS);

    delay(600);

    if (trigger_count == 0) {
        return "MISS";
    }

    if (trigger_count == 1) {
        return "OK";
    }

    return "OVER_DISPENSE";
}

void handle_dispense(int compartment_id) {

    bool success = home_stepper();

    if (!success) {

        Serial.println("ERROR:HOME_FAIL");

        return;
    }

    rotate_to_compartment(compartment_id);

    String result = run_dispense_cycle();

    if (result == "OK") {

        Serial.println("OK");
    }
    else if (result == "MISS") {

        Serial.println("RETRY");
    }
    else if (result == "OVER_DISPENSE") {

        Serial.println("ERROR:MULTI_PILL");

        enter_halt_state();
    }
}

void enter_halt_state() {

    while (true) {

        if (Serial.available()) {

            String cmd = Serial.readStringUntil('\n');

            cmd.trim();

            if (cmd == "RESET") {
                return;
            }
        }

        delay(100);
    }
}