/*********************************************************************
 *  Medi-Bot Controller
 *
 *  Combined Arduino firmware for the Medi-Bot medication robot. It merges
 *  three previously separate programs into one sketch:
 *
 *    1. ROSArduinoBridge  - differential-drive base controller (navigation),
 *                           serial command protocol, encoders and PID.
 *    2. dispense          - rotary carousel stepper + 2 dispense servos +
 *                           3 IR sensors (homing, pill-detect, cup-taken).
 *    3. finalrfid         - MFRC522 RFID patient verification + buzzer.
 *
 *  The ROSArduinoBridge single-letter serial protocol is the master protocol.
 *  Everything below the original BSD header is the unmodified ROSArduinoBridge
 *  sketch, with the dispenser + RFID folded in at four clearly marked points
 *  (look for "Medi-Bot"):
 *      - extra #include block
 *      - extra cases in runCommand()
 *      - extra init in setup()
 *      - extra poll in loop()
 *
 *  New commands added (see commands.h):
 *      D <slot>   dispense one pill from slot 1-5, e.g. "D 3"
 *      z          buzzer alert (wrong patient / needs human help)
 *  The RFID reader auto-reports every tag it sees as "RFID:AA BB CC DD".
 *
 *  Serial: 57600 baud (the ROSArduinoBridge BAUDRATE). The Raspberry Pi must
 *  connect at 57600. Command lines are terminated by a carriage return (\r).
 *
 *  --- Original ROSArduinoBridge header below ---
 *
    A set of simple serial commands to control a differential drive
    robot and receive back sensor and odometry data. Default
    configuration assumes use of an Arduino Mega + Pololu motor
    controller shield + Robogaia Mega Encoder shield.  Edit the
    readEncoder() and setMotorSpeed() wrapper functions if using
    different motor controller or encoder method.

    Created for the Pi Robot Project: http://www.pirobot.org
    and the Home Brew Robotics Club (HBRC): http://hbrobotics.org

    Authors: Patrick Goebel, James Nugen

    Inspired and modeled after the ArbotiX driver by Michael Ferguson

    Software License Agreement (BSD License)

    Copyright (c) 2012, Patrick Goebel.
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above
       copyright notice, this list of conditions and the following
       disclaimer in the documentation and/or other materials provided
       with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
    ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************/

#define USE_BASE      // Enable the base controller code
//#undef USE_BASE     // Disable the base controller code

/* Define the motor controller and encoder library you are using */
#ifdef USE_BASE
   /* The Pololu VNH5019 dual motor driver shield */
   //#define POLOLU_VNH5019

   /* The Pololu MC33926 dual motor driver shield */
   //#define POLOLU_MC33926

   /* The RoboGaia encoder shield */
   //#define ROBOGAIA

   /* Encoders directly attached to Arduino board */
   #define ARDUINO_ENC_COUNTER

   /* L298 Motor driver*/
   #define L298_MOTOR_DRIVER
#endif

//#define USE_SERVOS  // Enable use of PWM servos as defined in servos.h
#undef USE_SERVOS     // Disable use of PWM servos

/* Serial port baud rate */
#define BAUDRATE     57600

/* Maximum PWM signal */
#define MAX_PWM        255

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

/* Include definition of serial commands */
#include "commands.h"

/* Sensor functions */
#include "sensors.h"

/* ----------------------------------------------------------------------
   Medi-Bot: dispenser + RFID integration.
   These modules live alongside the ROSArduinoBridge files in this sketch
   folder. They do not touch any of the ROS names, pins or functions.
   ---------------------------------------------------------------------- */
#include "config.h"            // dispenser pins + timing
#include "stepper_control.h"   // carousel stepper
#include "servo_control.h"     // dispense servos A and B
#include "dispense.h"          // full dispense state machine
#include "rfid_reader.h"       // MFRC522 patient tag reader + buzzer

/* Include servo support if required */
#ifdef USE_SERVOS
   #include <Servo.h>
   #include "servos.h"
#endif

#ifdef USE_BASE
  /* Motor driver function definitions */
  #include "motor_driver.h"

  /* Encoder driver function definitions */
  #include "encoder_driver.h"

  /* PID parameters and functions */
  #include "diff_controller.h"

  /* Run the PID loop at 30 times per second */
  #define PID_RATE           30     // Hz

  /* Convert the rate into an interval */
  const int PID_INTERVAL = 1000 / PID_RATE;

  /* Track the next time we make a PID calculation */
  unsigned long nextPID = PID_INTERVAL;

  /* Stop the robot if it hasn't received a movement command
   in this number of milliseconds */
  #define AUTO_STOP_INTERVAL 2000
  long lastMotorCommand = AUTO_STOP_INTERVAL;
#endif

/* Variable initialization */

// A pair of varibles to help parse serial commands (thanks Fergs)
int arg = 0;
int index = 0;

// Variable to hold an input character
char chr;

// Variable to hold the current single-character command
char cmd;

// Character arrays to hold the first and second arguments
char argv1[16];
char argv2[16];

// The arguments converted to integers
long arg1;
long arg2;

/* Clear the current command parameters */
void resetCommand() {
  cmd = NULL;
  memset(argv1, 0, sizeof(argv1));
  memset(argv2, 0, sizeof(argv2));
  arg1 = 0;
  arg2 = 0;
  arg = 0;
  index = 0;
}

/* ----------------------------------------------------------------------
   Medi-Bot: protect the IR sensor pins from the raw pin-control commands.
   The IR signal lines sit on ir_homing_pin / ir_pill_detection_pin /
   ir_medication_cup_pin as INPUT_PULLUP (see setup()). The ROSArduinoBridge
   raw commands x (analogWrite), w (digitalWrite), c (pinMode) and p (PING)
   can reconfigure ANY pin as a driven OUTPUT. If one of them ever targets an
   IR pin, the Arduino's output driver fights the sensor's output transistor
   and can destroy the sensor. Refuse any raw pin command on a reserved pin.
   ---------------------------------------------------------------------- */
bool isReservedIrPin(long pin) {
  return pin == ir_homing_pin
      || pin == ir_pill_detection_pin
      || pin == ir_medication_cup_pin;
}

/* Run a command.  Commands are defined in commands.h */
int runCommand() {
  int i = 0;
  char *p = argv1;
  char *str;
  int pid_args[4];
  arg1 = atoi(argv1);
  arg2 = atoi(argv2);

  switch(cmd) {
  case GET_BAUDRATE:
    Serial.println(BAUDRATE);
    break;
  case ANALOG_READ:
    Serial.println(analogRead(arg1));
    break;
  case DIGITAL_READ:
    Serial.println(digitalRead(arg1));
    break;
  case ANALOG_WRITE:
    if (isReservedIrPin(arg1)) { Serial.println("ERROR:RESERVED_IR_PIN"); break; }
    analogWrite(arg1, arg2);
    Serial.println("OK");
    break;
  case DIGITAL_WRITE:
    if (isReservedIrPin(arg1)) { Serial.println("ERROR:RESERVED_IR_PIN"); break; }
    if (arg2 == 0) digitalWrite(arg1, LOW);
    else if (arg2 == 1) digitalWrite(arg1, HIGH);
    Serial.println("OK");
    break;
  case PIN_MODE:
    if (isReservedIrPin(arg1)) { Serial.println("ERROR:RESERVED_IR_PIN"); break; }
    if (arg2 == 0) pinMode(arg1, INPUT);
    else if (arg2 == 1) pinMode(arg1, OUTPUT);
    Serial.println("OK");
    break;
  case PING:
    if (isReservedIrPin(arg1)) { Serial.println("ERROR:RESERVED_IR_PIN"); break; }
    Serial.println(Ping(arg1));
    break;

  /* ----------------------------------------------------------------------
     Medi-Bot: dispenser + RFID commands.
     ---------------------------------------------------------------------- */
  case DISPENSE_PILL:
    // arg1 is the slot number 1-5 sent by the Pi (e.g. "D 3"). The dispenser
    // uses a 0-based index. dispense_compartment() prints its own progress
    // (DEBUG:...), then "OK" + "COMPLETE" on success, or "ERROR:..." on
    // failure, so we do not print an extra "OK" here.
    dispense_compartment(arg1 - 1);
    break;
  case BUZZER_ALERT:
    // "z" -> the default short beep. "z <ms>" -> beep for <ms>. The Pi strings
    // several of these together (with gaps) to make the RFID beep patterns.
    rfid_beep_alert(arg1 > 0 ? (unsigned long)arg1 : buzzer_short_ms);
    Serial.println("OK");
    break;

#ifdef USE_SERVOS
  case SERVO_WRITE:
    servos[arg1].setTargetPosition(arg2);
    Serial.println("OK");
    break;
  case SERVO_READ:
    Serial.println(servos[arg1].getServo().read());
    break;
#endif

#ifdef USE_BASE
  case READ_ENCODERS:
    Serial.print(readEncoder(LEFT));
    Serial.print(" ");
    Serial.println(readEncoder(RIGHT));
    break;
   case RESET_ENCODERS:
    resetEncoders();
    resetPID();
    Serial.println("OK");
    break;
  case MOTOR_SPEEDS:
    /* Reset the auto stop timer */
    lastMotorCommand = millis();
    if (arg1 == 0 && arg2 == 0) {
      setMotorSpeeds(0, 0);
      resetPID();
      moving = 0;
    }
    else moving = 1;
    leftPID.TargetTicksPerFrame = arg1;
    rightPID.TargetTicksPerFrame = arg2;
    Serial.println("OK");
    break;
  case MOTOR_RAW_PWM:
    /* Reset the auto stop timer */
    lastMotorCommand = millis();
    resetPID();
    moving = 0; // Sneaky way to temporarily disable the PID
    setMotorSpeeds(arg1, arg2);
    Serial.println("OK");
    break;
  case UPDATE_PID:
    while ((str = strtok_r(p, ":", &p)) != '\0') {
       pid_args[i] = atoi(str);
       i++;
    }
    Kp = pid_args[0];
    Kd = pid_args[1];
    Ki = pid_args[2];
    Ko = pid_args[3];
    Serial.println("OK");
    break;
#endif
  default:
    Serial.println("Invalid Command");
    break;
  }
}

/* Setup function--runs once at startup. */
void setup() {
  Serial.begin(BAUDRATE);

// Initialize the motor controller if used */
#ifdef USE_BASE
  #ifdef ARDUINO_ENC_COUNTER
    //set as inputs
    DDRD &= ~(1<<LEFT_ENC_PIN_A);
    DDRD &= ~(1<<LEFT_ENC_PIN_B);
    DDRC &= ~(1<<RIGHT_ENC_PIN_A);
    DDRC &= ~(1<<RIGHT_ENC_PIN_B);

    //enable pull up resistors
    PORTD |= (1<<LEFT_ENC_PIN_A);
    PORTD |= (1<<LEFT_ENC_PIN_B);
    PORTC |= (1<<RIGHT_ENC_PIN_A);
    PORTC |= (1<<RIGHT_ENC_PIN_B);

    // tell pin change mask to listen to left encoder pins
    PCMSK2 |= (1 << LEFT_ENC_PIN_A)|(1 << LEFT_ENC_PIN_B);
    // tell pin change mask to listen to right encoder pins
    PCMSK1 |= (1 << RIGHT_ENC_PIN_A)|(1 << RIGHT_ENC_PIN_B);

    // enable PCINT1 and PCINT2 interrupt in the general interrupt mask
    PCICR |= (1 << PCIE1) | (1 << PCIE2);
  #endif
  initMotorController();
  resetPID();
#endif

/* Attach servos if used */
  #ifdef USE_SERVOS
    int i;
    for (i = 0; i < N_SERVOS; i++) {
      servos[i].initServo(
          servoPins[i],
          stepDelay[i],
          servoInitPosition[i]);
    }
  #endif

  /* ----------------------------------------------------------------------
     Medi-Bot: bring up the dispenser hardware and the RFID reader.
     ---------------------------------------------------------------------- */

  // Stepper carousel + dispense servos.
  initialise_stepper();
  initialise_servos();

  // IR sensors use INPUT_PULLUP, so LOW = beam broken / triggered.
  pinMode(ir_homing_pin, INPUT_PULLUP);
  pinMode(ir_pill_detection_pin, INPUT_PULLUP);
  pinMode(ir_medication_cup_pin, INPUT_PULLUP);

  // RFID patient verification + buzzer.
  initialise_rfid();

  Serial.println("DEBUG:MEDI_BOT_READY");
}

/* Enter the main loop.  Read and parse input from the serial port
   and run any valid commands. Run a PID calculation at the target
   interval and check for auto-stop conditions.
*/
void loop() {
  while (Serial.available() > 0) {

    // Read the next character
    chr = Serial.read();

    // Terminate a command with a CR
    if (chr == 13) {
      if (arg == 1) argv1[index] = NULL;
      else if (arg == 2) argv2[index] = NULL;
      runCommand();
      resetCommand();
    }
    // Use spaces to delimit parts of the command
    else if (chr == ' ') {
      // Step through the arguments
      if (arg == 0) arg = 1;
      else if (arg == 1)  {
        argv1[index] = NULL;
        arg = 2;
        index = 0;
      }
      continue;
    }
    else {
      if (arg == 0) {
        // The first arg is the single-letter command
        cmd = chr;
      }
      else if (arg == 1) {
        // Subsequent arguments can be more than one character
        argv1[index] = chr;
        index++;
      }
      else if (arg == 2) {
        argv2[index] = chr;
        index++;
      }
    }
  }

// If we are using base control, run a PID calculation at the appropriate intervals
#ifdef USE_BASE
  if (millis() > nextPID) {
    updatePID();
    nextPID += PID_INTERVAL;
  }

  // Check to see if we have exceeded the auto-stop interval
  if ((millis() - lastMotorCommand) > AUTO_STOP_INTERVAL) {;
    setMotorSpeeds(0, 0);
    moving = 0;
  }
#endif

// Sweep servos
#ifdef USE_SERVOS
  int i;
  for (i = 0; i < N_SERVOS; i++) {
    servos[i].doSweep();
  }
#endif

  /* ----------------------------------------------------------------------
     Medi-Bot: continuously watch for RFID tags. Non-blocking - prints
     "RFID:AA BB CC DD" when a new tag is seen (rate-limited internally).
     ---------------------------------------------------------------------- */
  rfid_poll_and_report();
}
