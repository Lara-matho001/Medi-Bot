/*
  BASIC MEDI-BOT HARDWARE TEST PROGRAM
  ------------------------------------
  Tests:
  - NEMA 17 Stepper Motor
  - Servo A
  - Servo B
  - IR Sensor

  Designed to be EASY TO MODIFY later.

  HOW TO USE:
  Open Serial Monitor at 9600 baud.
  Type commands like:

    step 200
    step -200
    servoA
    servoB
    ir

  NOTES:
  - This assumes:
      * NEMA 17 uses ULN2003 or L298N style driver
      * 4 control wires connected to Arduino
      * Servos connected directly to PWM pins
  - You WILL probably need to change pin numbers.
  - If the stepper jitters but doesn't rotate:
      -> reorder the IN1-IN4 pins.
*/

#include <Servo.h>
#include <Stepper.h>

//////////////////////////////////////////////////////
// STEP 1 — CHANGE THESE PINS TO MATCH YOUR WIRING
//////////////////////////////////////////////////////

// ===== STEPPER PINS =====
const int IN1 = 4;
const int IN2 = 5;
const int IN3 = 6;
const int IN4 = 7;

// ===== SERVO PINS =====
const int SERVO_A_PIN = 9;
const int SERVO_B_PIN = 10;

// ===== IR SENSOR PIN =====
const int IR_PIN = 2;

//////////////////////////////////////////////////////
// STEP 2 — STEPPER SETTINGS
//////////////////////////////////////////////////////

// Common NEMA17 value
const int STEPS_PER_REV = 200;

// Create stepper object
Stepper stepperMotor(STEPS_PER_REV, IN1, IN3, IN2, IN4);

//////////////////////////////////////////////////////
// STEP 3 — CREATE SERVOS
//////////////////////////////////////////////////////

Servo servoA;
Servo servoB;

//////////////////////////////////////////////////////
// SETUP
//////////////////////////////////////////////////////

void setup() {

  Serial.begin(9600);

  // Attach servos
  servoA.attach(SERVO_A_PIN);
  servoB.attach(SERVO_B_PIN);

  // Set IR sensor pin
  pinMode(IR_PIN, INPUT);

  // Stepper speed
  stepperMotor.setSpeed(60);

  Serial.println("=================================");
  Serial.println("MEDI-BOT HARDWARE TEST STARTED");
  Serial.println("=================================");
  Serial.println("");
  Serial.println("Commands:");
  Serial.println("step 200");
  Serial.println("step -200");
  Serial.println("servoA");
  Serial.println("servoB");
  Serial.println("ir");
  Serial.println("");
}

//////////////////////////////////////////////////////
// MAIN LOOP
//////////////////////////////////////////////////////

void loop() {

  if (Serial.available()) {

    String command = Serial.readStringUntil('\n');
    command.trim();

    //////////////////////////////////////////////////
    // TEST STEPPER
    //////////////////////////////////////////////////

    if (command.startsWith("step")) {

      // Extract number after "step"
      int steps = command.substring(5).toInt();

      Serial.print("Moving stepper: ");
      Serial.println(steps);

      stepperMotor.step(steps);

      Serial.println("Done");
    }

    //////////////////////////////////////////////////
    // TEST SERVO A
    //////////////////////////////////////////////////

    else if (command == "servoA") {

      Serial.println("Testing Servo A");

      servoA.write(0);
      delay(1000);

      servoA.write(180);
      delay(1000);

      servoA.write(90);
      delay(500);

      Serial.println("Servo A complete");
    }

    //////////////////////////////////////////////////
    // TEST SERVO B
    //////////////////////////////////////////////////

    else if (command == "servoB") {

      Serial.println("Testing Servo B");

      servoB.write(0);
      delay(1000);

      servoB.write(180);
      delay(1000);

      servoB.write(90);
      delay(500);

      Serial.println("Servo B complete");
    }

    //////////////////////////////////////////////////
    // TEST IR SENSOR
    //////////////////////////////////////////////////

    else if (command == "ir") {

      Serial.println("Reading IR sensor for 10 seconds...");
      Serial.println("Move something in front of it.");

      unsigned long startTime = millis();

      while (millis() - startTime < 10000) {

        int sensorValue = digitalRead(IR_PIN);

        Serial.print("IR State: ");
        Serial.println(sensorValue);

        delay(300);
      }

      Serial.println("IR test complete");
    }

    //////////////////////////////////////////////////
    // UNKNOWN COMMAND
    //////////////////////////////////////////////////

    else {

      Serial.println("Unknown command");
    }
  }
}