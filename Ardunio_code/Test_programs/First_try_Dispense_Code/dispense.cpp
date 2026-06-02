#include "dispense.h"
#include "config.h"
#include "stepper_control.h"
#include "servo_control.h"
// RFID disabled for motor/dispense testing.
// #include "rfid_control.h"

// RFID disabled for motor/dispense testing.
// void handle_rfid_scan() {
//
//     // millis() is used instead of delay() so this function can check repeatedly
//     // until either a card is found or the timeout expires.
//     unsigned long start_time = millis();
//
//     while (millis() - start_time < DISPENSER_RFID_SCAN_TIMEOUT_MS) {
//
//         if (rfid_card_present()) {
//
//             // Card detected: read the UID and send it back to the controller.
//             String uid = rfid_read_uid();
//
//             Serial.println("RFID:" + uid);
//
//             return;
//         }
//     }
//
//     // No card was detected during the scan window.
//     Serial.println("RFID:TIMEOUT");
// }

void count_chute_trigger(int &trigger_count) {

    if (digitalRead(DISPENSER_IR_CHUTE_PIN) != LOW) {
        return;
    }

    trigger_count++;
    Serial.print("DEBUG:CHUTE_TRIGGER_COUNT=");
    Serial.println(trigger_count);

    unsigned long chute_blocked_start = millis();
    while (digitalRead(DISPENSER_IR_CHUTE_PIN) == LOW &&
           millis() - chute_blocked_start < DISPENSER_CHUTE_CLEAR_TIMEOUT_MS) {
        delay(1);
    }

    if (digitalRead(DISPENSER_IR_CHUTE_PIN) == LOW) {
        Serial.println("DEBUG:CHUTE_STILL_BLOCKED_TIMEOUT");
    }
    else {
        Serial.println("DEBUG:CHUTE_CLEAR");
    }

    delay(200);
}

void monitor_chute_for(int &trigger_count, unsigned long duration_ms) {

    unsigned long start_time = millis();
    while (millis() - start_time < duration_ms) {
        count_chute_trigger(trigger_count);
    }
}

void move_servo_and_monitor(
    Servo &servo,
    const char *label,
    int position,
    int &trigger_count
) {

    move_servo_to(servo, label, position);
    monitor_chute_for(trigger_count, DISPENSER_SERVO_MOVE_DELAY_MS);
}

void rotate_to_compartment(int compartment_id) {

    // Slot positions are tuned in motor_settings.h as steps clockwise from home.
    int target_steps = DISPENSER_COMPARTMENT_STEPS[compartment_id];

    Serial.print("DEBUG:ROTATE_TO_SLOT=");
    Serial.println(compartment_id);
    Serial.print("DEBUG:TARGET_STEPS=");
    Serial.println(target_steps);

    // Move clockwise by the calculated number of slot steps.
    stepper_step(true, target_steps);

    // Remember where the carousel should now be.
    dispenserCurrentStepPosition = target_steps;

    Serial.println("DEBUG:ROTATE_DONE");

    // Give the hardware a short moment to settle after movement.
    Serial.println("DEBUG:MOTOR_GAP_AFTER_STEPPER_ROTATE");
    delay(DISPENSER_MOTOR_GAP_MS);
}

String run_dispense_cycle() {

    Serial.println("DEBUG:DISPENSE_CYCLE_START");

    // Count how many times a pill breaks the chute IR beam.
    int trigger_count = 0;
    unsigned long window_start = millis();

    Serial.println("DEBUG:CHUTE_WATCH_START");

    Serial.println("DEBUG:SERVO_A_MOVE_ACTIVE_HOLD");
    move_servo_and_monitor(
        dispenserServoA,
        "SERVO_A",
        DISPENSER_SERVO_A_ACTIVE_POS,
        trigger_count
    );

    Serial.println("DEBUG:SERVO_B_MOVE_ACTIVE");
    move_servo_and_monitor(
        dispenserServoB,
        "SERVO_B",
        DISPENSER_SERVO_B_ACTIVE_POS,
        trigger_count
    );

    Serial.println("DEBUG:SERVO_B_RETURN_HOME");
    move_servo_and_monitor(
        dispenserServoB,
        "SERVO_B",
        DISPENSER_SERVO_B_HOME_POS,
        trigger_count
    );

    Serial.println("DEBUG:SERVO_A_RETURN_HOME");
    move_servo_and_monitor(
        dispenserServoA,
        "SERVO_A",
        DISPENSER_SERVO_A_HOME_POS,
        trigger_count
    );

    while (millis() - window_start < DISPENSER_DISPENSE_WINDOW_MS) {
        count_chute_trigger(trigger_count);
    }

    Serial.println("DEBUG:CHUTE_WATCH_END");

    Serial.print("DEBUG:FINAL_TRIGGER_COUNT=");
    Serial.println(trigger_count);

    // No chute trigger means the system tried to dispense but did not see a pill.
    if (trigger_count == 0) {
        return "MISS";
    }

    // Exactly one trigger is the expected successful dispense.
    if (trigger_count == 1) {
        return "OK";
    }

    // More than one trigger means more than one pill probably dropped.
    return "OVER_DISPENSE";
}

void handle_dispense(int compartment_id) {

    Serial.println("DEBUG:HANDLE_DISPENSE_START");

    // Reject invalid slot numbers before moving any hardware.
    if (compartment_id < 0 || compartment_id >= DISPENSER_SLOT_COUNT) {

        Serial.print("DEBUG:BAD_COMPARTMENT=");
        Serial.println(compartment_id);
        Serial.println("ERROR:BAD_COMPARTMENT");

        return;
    }

    // Always home first so the slot movement starts from a known physical position.
    Serial.println("DEBUG:HOMING_START");
    bool success = home_stepper();

    if (!success) {

        // home_stepper() already prints this error too, but keeping it here makes
        // handle_dispense() report its own failure path clearly.
        Serial.println("ERROR:HOME_FAIL");

        return;
    }
    Serial.println("DEBUG:HOMING_OK");

    Serial.println("DEBUG:MOTOR_GAP_AFTER_HOMING");
    delay(DISPENSER_MOTOR_GAP_MS);

    // Move from home to the requested pill compartment.
    rotate_to_compartment(compartment_id);

    int attempt = 1;

    while (true) {

        Serial.print("DEBUG:DISPENSE_ATTEMPT=");
        Serial.println(attempt);

        // Try dispensing and classify the chute sensor result.
        String result = run_dispense_cycle();

        Serial.print("DEBUG:DISPENSE_RESULT=");
        Serial.println(result);

        if (result == "OK") {

            // One pill was detected.
            Serial.println("OK");
            return;
        }
        else if (result == "MISS") {

            // No pill was detected, so try the same aligned compartment again.
            Serial.println("RETRY");
            Serial.println("DEBUG:MISS_RETRYING_DISPENSE_CYCLE");
            Serial.println("DEBUG:MOTOR_GAP_BEFORE_RETRY");
            delay(DISPENSER_MOTOR_GAP_MS);
            attempt++;
        }
        else if (result == "OVER_DISPENSE") {

            // Multiple pills were detected, so stop until a human/controller resets it.
            Serial.println("ERROR:MULTI_PILL");

            enter_halt_state();
            return;
        }
    }
}

void enter_halt_state() {

    // This is a safety stop. The dispenser ignores all commands except RESET.
    Serial.println("DEBUG:HALT_STATE_ENTERED_WAITING_FOR_RESET");
    while (true) {

        if (Serial.available()) {

            String cmd = Serial.readStringUntil('\n');

            cmd.trim();

            if (cmd == "RESET") {
                // Return to loop() so normal command handling can continue.
                Serial.println("DEBUG:RESET_RECEIVED_EXITING_HALT");
                return;
            }
        }

        // Avoid hammering the serial buffer in a tight loop.
        delay(100);
    }
}
