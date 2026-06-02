#include "config.h"
#include "stepper_control.h"
#include "servo_control.h"
#include "dispense.h"

// Stores the current stepper position after the dispenser has been homed.
int dispenserCurrentStepPosition = 0;

void setup() {

    // Start the serial connection first so setup debug messages are visible.
    Serial.begin(SERIAL_BAUD);
    Serial.println("DEBUG:BOOT");

    // Prepare the stepper output pins before trying to move the carousel.
    Serial.println("DEBUG:INIT_STEPPER");
    initialise_stepper();

    // Attach both servos and move them to their starting positions.
    Serial.println("DEBUG:INIT_SERVOS");
    initialise_servos();

    // INPUT_PULLUP means the Arduino holds the pin HIGH until the sensor pulls it LOW.
    Serial.println("DEBUG:INIT_IR_SENSORS");
    pinMode(IR_HOMEING_PIN, INPUT_PULLUP);
    pinMode(IR_PILL_DETECTION_PIN, INPUT_PULLUP);

    // Let the controller/computer know the dispenser sketch has booted.
    Serial.println("ARDUINO_READY");
    Serial.println("DEBUG:READY_FOR_COMMANDS");
    Serial.println("Type DISPENSE:<slot>");
}

void loop() {

    // Commands arrive as one line of text, for example: DISPENSE:2
    if (Serial.available()) {

        String raw = Serial.readStringUntil('\n');

        // Remove spaces, carriage returns, and newline leftovers.
        raw.trim();

        Serial.print("DEBUG:RAW_CMD=");
        Serial.println(raw);

        // Commands may have a parameter after a colon.
        int separator = raw.indexOf(':');

        String verb;
        String param;

        if (separator != -1) {

            // Example: "DISPENSE:2" becomes verb="DISPENSE", param="2".
            verb = raw.substring(0, separator);
            param = raw.substring(separator + 1);
        }
        else {

            verb = raw;
        }

        Serial.print("DEBUG:VERB=");
        Serial.println(verb);
        Serial.print("DEBUG:PARAM=");
        Serial.println(param);

        if (verb == "DISPENSE") {

            // Convert the slot number text into an integer and run the full dispense process.
            int compartment_id = param.toInt();

            Serial.print("DEBUG:DISPENSE_REQUEST_SLOT=");
            Serial.println(compartment_id);

            handle_dispense(compartment_id);
        }
        else {

            // Anything else is not part of this sketch's command set.
            Serial.print("DEBUG:UNKNOWN_CMD=");
            Serial.println(raw);
            Serial.println("ERROR:UNKNOWN_CMD");
        }
    }
}
