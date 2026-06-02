#include "pin_config.h"
#include "position_settings.h"
#include "stepper_driver.h"

void print_help() {

    Serial.println("Commands:");
    Serial.println("  home");
    Serial.println("  dispense 0");
    Serial.println("  dispense 1");
    Serial.println("  dispense:2");
    Serial.println("Each dispense command homes first, then moves to the selected compartment.");
}

int parse_compartment(String command) {

    command.trim();
    command.toLowerCase();

    int separator = command.indexOf(':');
    if (separator == -1) {
        separator = command.indexOf(' ');
    }

    if (separator == -1) {
        return -1;
    }

    String value = command.substring(separator + 1);
    value.trim();

    return value.toInt();
}

void setup() {

    Serial.begin(SERIAL_BAUD);
    Serial.println("STEPPER_POSITION_TEST_READY");

    initialise_stepper();

    Serial.println("DEBUG:STARTUP_HOME");
    home_stepper();

    print_help();
}

void loop() {

    if (!Serial.available()) {
        return;
    }

    String command = Serial.readStringUntil('\n');
    command.trim();

    Serial.print("DEBUG:RAW_CMD=");
    Serial.println(command);

    String lowerCommand = command;
    lowerCommand.toLowerCase();

    if (lowerCommand == "home") {
        home_stepper();
        return;
    }

    if (lowerCommand.startsWith("dispense")) {
        int compartment = parse_compartment(lowerCommand);

        Serial.print("DEBUG:DISPENSE_REQUEST_COMPARTMENT=");
        Serial.println(compartment);

        if (!home_stepper()) {
            return;
        }

        move_to_compartment(compartment);
        return;
    }

    if (lowerCommand == "help") {
        print_help();
        return;
    }

    Serial.println("ERROR:UNKNOWN_CMD");
    print_help();
}
