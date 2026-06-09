/***************************************************************
   Motor driver function definitions - by James Nugen
   *************************************************************/

#ifdef L298_MOTOR_DRIVER
  #define RIGHT_MOTOR_BACKWARD 6
  #define LEFT_MOTOR_BACKWARD  7
  #define RIGHT_MOTOR_FORWARD  9
  #define LEFT_MOTOR_FORWARD   8
  // NOTE: the motor ENABLE pins are not physically wired in this build
  // (the L298 ENA/ENB jumpers are tied HIGH on the board). They are set
  // to unused pins so the code compiles. Update them if you ever wire the
  // enable lines to the Arduino.
  #define RIGHT_MOTOR_ENABLE 34
  #define LEFT_MOTOR_ENABLE 35
#endif

void initMotorController();
void setMotorSpeed(int i, int spd);
void setMotorSpeeds(int leftSpeed, int rightSpeed);
