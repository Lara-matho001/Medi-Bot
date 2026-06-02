#include "config.h"
#include "stepper_control.h"
#include "servo_control.h"
#include "dispense.h"

void setup() {

    // Start the serial connection first so setup debug messages are visible.
    Serial.begin(serial_baud);
    Serial.println("DEBUG:BOOT");

    // Prepare the stepper output pins before trying to move the carousel.
    Serial.println("DEBUG:INIT_STEPPER");
    initialise_stepper();

    // Attach both servos and move them to their starting positions.
    Serial.println("DEBUG:INIT_SERVOS");
    initialise_servos();

    // INPUT_PULLUP means the Arduino holds the pin HIGH until the sensor pulls it LOW.
    Serial.println("DEBUG:INIT_IR_SENSORS");
    pinMode(ir_homing_pin, INPUT_PULLUP);
    pinMode(ir_pill_detection_pin, INPUT_PULLUP);
    pinMode(ir_medication_cup_pin, INPUT_PULLUP);

    // Let the controller/computer know the dispenser sketch has booted.
    Serial.println("ARDUINO_READY");
    Serial.println("DEBUG:READY_FOR_COMMANDS");
    Serial.println("Send slot number 1-5 to dispense.");
}

void loop() {

    // Commands are a single digit 1-5 selecting the compartment slot.
    if (Serial.available()) {

        String raw = Serial.readStringUntil('\n');
        raw.trim();

        Serial.print("DEBUG:RAW_CMD=");
        Serial.println(raw);

        if (raw.length() == 1 && raw[0] >= '1' && raw[0] <= '5') {

            // User sends 1-5; internal compartment index is 0-4.
            int compartment_id = raw.toInt() - 1;

            Serial.print("DEBUG:DISPENSE_REQUEST_SLOT=");
            Serial.println(compartment_id);

            dispense_compartment(compartment_id);
        }
        else {

            Serial.print("DEBUG:UNKNOWN_CMD=");
            Serial.println(raw);
            Serial.println("ERROR:UNKNOWN_CMD");
            Serial.println("Send slot number 1-5 to dispense.");
        }
    }
}
