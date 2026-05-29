#include "config.h"
#include "stepper_control.h"
#include "servo_control.h"
#include "rfid_control.h"
#include "dispense.h"

int current_step_position = 0;

void setup() {

    initialise_stepper();

    initialise_servos();

    pinMode(IR_HOME_PIN, INPUT);
    pinMode(IR_CHUTE_PIN, INPUT);

    initialise_rfid();

    Serial.begin(SERIAL_BAUD);

    Serial.println("ARDUINO_READY");
}

void loop() {

    if (Serial.available()) {

        String raw = Serial.readStringUntil('\n');

        raw.trim();

        int separator = raw.indexOf(':');

        String verb;
        String param;

        if (separator != -1) {

            verb = raw.substring(0, separator);
            param = raw.substring(separator + 1);
        }
        else {

            verb = raw;
        }

        if (verb == "SCAN_RFID") {

            handle_rfid_scan();
        }
        else if (verb == "DISPENSE") {

            int compartment_id = param.toInt();

            handle_dispense(compartment_id);
        }
        else {

            Serial.println("ERROR:UNKNOWN_CMD");
        }
    }
}
