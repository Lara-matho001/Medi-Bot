/***************************************************************
   Motor driver function definitions - by James Nugen
   *************************************************************/

#ifdef L298_MOTOR_DRIVER
  #define RIGHT_MOTOR_BACKWARD 5
  #define LEFT_MOTOR_BACKWARD  6
  #define RIGHT_MOTOR_FORWARD  9
  #define LEFT_MOTOR_FORWARD   10
  // NOTE: the motor ENABLE pins were originally 12 and 13. Those two pins
  // are now used by the dispenser IR sensors (ir_pill_detection_pin = 12,
  // ir_medication_cup_pin = 13 in config.h). The L298 enable lines are not
  // wired to the Arduino in this build (the L298 ENA/ENB jumpers are tied
  // HIGH on the board), so they have been moved to spare Mega pins to avoid
  // the clash. Repoint these if you ever wire the enables to the Arduino.
  #define RIGHT_MOTOR_ENABLE 30   // was 12
  #define LEFT_MOTOR_ENABLE 31    // was 13
#endif

void initMotorController();
void setMotorSpeed(int i, int spd);
void setMotorSpeeds(int leftSpeed, int rightSpeed);
