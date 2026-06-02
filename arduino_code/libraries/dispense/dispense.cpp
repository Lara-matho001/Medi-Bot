#include "dispense.h"
#include "config.h"
#include "stepper_control.h"
#include "servo_control.h"

void detect_chute_trigger(int &trigger_count) {

    if (digitalRead(ir_pill_detection_pin) != LOW) {
        return;
    }

    trigger_count++;
    Serial.print("DEBUG:CHUTE_TRIGGER_COUNT=");
    Serial.println(trigger_count);

    unsigned long chute_blocked_start = millis();
    while (digitalRead(ir_pill_detection_pin) == LOW &&
           millis() - chute_blocked_start < chute_clear_timeout_ms) {
        delay(1);
    }

    if (digitalRead(ir_pill_detection_pin) == LOW) {
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
        detect_chute_trigger(trigger_count);
    }
}

void move_servo_and_monitor(
    Servo &servo,
    const char *label,
    int position,
    int &trigger_count
) {

    move_servo_to(servo, label, position);
    monitor_chute_for(trigger_count, servo_move_delay_ms);
}

void rotate_to_compartment(int compartment_id) {

    // Slot positions are tuned in motor_settings.h as steps clockwise from home.
    int target_steps = dispenser_compartment_steps[compartment_id];

    Serial.print("DEBUG:ROTATE_TO_SLOT=");
    Serial.println(compartment_id);
    Serial.print("DEBUG:TARGET_STEPS=");
    Serial.println(target_steps);

    // Move clockwise by the calculated number of slot steps.
    stepper_step(true, target_steps);

    Serial.println("DEBUG:ROTATE_DONE");

    // Give the hardware a short moment to settle after movement.
    Serial.println("DEBUG:MOTOR_GAP_AFTER_STEPPER_ROTATE");
    delay(dispenser_motor_gap_ms);
}

String run_dispense_cycle() {

    Serial.println("DEBUG:DISPENSE_CYCLE_START");

    // Count how many times a pill breaks the chute IR beam.
    int trigger_count = 0;

    Serial.println("DEBUG:CHUTE_WATCH_START");

    Serial.println("DEBUG:SERVO_A_MOVE_ACTIVE_HOLD");
    move_servo_and_monitor(
        dispenser_servo_a,
        "SERVO_A",
        dispenser_servo_a_active_pos,
        trigger_count
    );

    Serial.println("DEBUG:SERVO_B_MOVE_ACTIVE");
    move_servo_and_monitor(
        dispenser_servo_b,
        "SERVO_B",
        dispenser_servo_b_active_pos,
        trigger_count
    );

    Serial.println("DEBUG:SERVO_B_RETURN_HOME");
    move_servo_and_monitor(
        dispenser_servo_b,
        "SERVO_B",
        servo_b_home_pos,
        trigger_count
    );

    Serial.println("DEBUG:SERVO_A_RETURN_HOME");
    move_servo_and_monitor(
        dispenser_servo_a,
        "SERVO_A",
        servo_a_home_pos,
        trigger_count
    );

    // Servos are home. Monitor for any late-falling pills for the full post-dispense window.
    unsigned long window_start = millis();
    while (millis() - window_start < post_dispense_monitor_ms) {
        detect_chute_trigger(trigger_count);
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

void wait_for_cup_taken() {

    Serial.println("DEBUG:WAITING_FOR_CUP_TAKEN");

    unsigned long start = millis();
    while (millis() - start < cup_taken_timeout_ms) {

        // With INPUT_PULLUP: LOW = cup blocking beam (present), HIGH = cup removed (taken).
        if (digitalRead(ir_medication_cup_pin) == HIGH) {
            Serial.println("COMPLETE");
            return;
        }

        delay(50);
    }

    Serial.println("WARNING:CUP_NOT_TAKEN");
}

void dispense_compartment(int compartment_id) {

    Serial.println("DEBUG:DISPENSE_COMPARTMENT_START");

    // Reject invalid slot numbers before moving any hardware.
    if (compartment_id < 0 || compartment_id >= dispenser_slot_count) {

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
        // dispense_compartment() report its own failure path clearly.
        Serial.println("ERROR:HOME_FAIL");

        return;
    }
    Serial.println("DEBUG:HOMING_OK");

    Serial.println("DEBUG:MOTOR_GAP_AFTER_HOMING");
    delay(dispenser_motor_gap_ms);

    // Move from home to the requested pill compartment.
    rotate_to_compartment(compartment_id);

    for (int attempt = 1; attempt <= dispense_max_retries; attempt++) {

        Serial.print("DEBUG:DISPENSE_ATTEMPT=");
        Serial.println(attempt);

        // Try dispensing and classify the chute sensor result.
        String result = run_dispense_cycle();

        Serial.print("DEBUG:DISPENSE_RESULT=");
        Serial.println(result);

        if (result == "OK") {

            // One pill was detected. Wait for the patient to take the cup.
            Serial.println("OK");
            wait_for_cup_taken();
            return;
        }
        else if (result == "MISS") {

            if (attempt == dispense_max_retries) {

                // All retries exhausted. Report failure and return to normal operation.
                Serial.println("ERROR:MAX_RETRIES_REACHED");
                return;
            }

            // No pill detected — retry the same aligned compartment.
            Serial.print("DEBUG:MISS_RETRY_");
            Serial.print(attempt);
            Serial.print("_OF_");
            Serial.println(dispense_max_retries);
            Serial.println("DEBUG:MOTOR_GAP_BEFORE_RETRY");
            delay(dispenser_motor_gap_ms);
        }
        else if (result == "OVER_DISPENSE") {

            // Multiple pills detected — stop until a human/controller resets it.
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
