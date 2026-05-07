#ifndef CONFIG_H
#define CONFIG_H

// ================================
// CONSTANTS
// ================================

#define STEPS_PER_REV       200
#define STEPS_PER_SLOT      40

#define IR_HOME_PIN         2
#define IR_CHUTE_PIN        3

#define SERVO_A_PIN         9
#define SERVO_B_PIN         10

#define RFID_SS_PIN         53
#define RFID_RST_PIN        5

#define SERVO_B_ENGAGE_POS  150
#define SERVO_B_RETRACT_POS 0

#define SERVO_A_SPIN_DEG    360

#define DISPENSE_WINDOW_MS  5000

#define SERIAL_BAUD         9600

// Stepper pins
const int STEPPER_PINS[4] = {4, 5, 6, 7};

// ================================
// GLOBALS
// ================================

extern int current_step_position;

#endif