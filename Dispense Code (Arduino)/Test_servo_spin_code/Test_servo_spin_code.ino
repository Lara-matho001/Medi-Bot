#include <Servo.h>

Servo myServo;
int pos = 0;
String input = "";

void setup() {
  Serial.begin(9600);
  Serial.println("Type 'spin' to move the servo:");
}

void loop() {
  if (Serial.available() > 0) {
    input = Serial.readStringUntil('\n');
    input.trim();

    if (input == "spin") {
      Serial.println("Activating servo...");

      myServo.attach(9);  // activate servo

      // Move from 0 to 180
      for (pos = 0; pos <= 180; pos++) {
        myServo.write(pos);
        delay(15);
      }

      delay(500);

      // Move back from 180 to 0
      for (pos = 180; pos >= 0; pos--) {
        myServo.write(pos);
        delay(15);
      }

      delay(500);

      myServo.detach();  // deactivate servo
      Serial.println("Servo deactivated. Waiting for next command.");
    } 
    else {
      Serial.println("Unknown command. Type 'spin'.");
    }
  }
}