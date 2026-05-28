#include <Servo.h>

Servo servoA;
Servo servoB;

// Keep track of whether servos are attached
bool servoAAttached = false;
bool servoBAttached = false;

void setup() {
  
  Serial.begin(9600);

  Serial.println("Type:");
  Serial.println("spin A");
  Serial.println("spin B");
}

void loop() {

  // Check if something was typed into Serial Monitor
  if (Serial.available()) {

    String command = Serial.readStringUntil('\n');
    command.trim();

    // =========================
    // SPIN SERVO A
    // =========================
    if (command == "spin A") {

      Serial.println("Spinning Servo A");

      // Attach servo to pin 9
      servoA.attach(9);
      servoAAttached = true;

      // Move one way
      servoA.write(180);
      delay(1000);

      // Move back
      servoA.write(0);
      delay(1000);

      // Stop sending power/signal
      servoA.detach();
      servoAAttached = false;

      Serial.println("Servo A finished");
    }

    // =========================
    // SPIN SERVO B
    // =========================
    else if (command == "spin B") {

      Serial.println("Spinning Servo B");

      // Attach servo to pin 10
      servoB.attach(10);
      servoBAttached = true;

      // Move one way
      servoB.write(180);
      delay(1000);

      // Move back
      servoB.write(0);
      delay(1000);

      // Stop sending power/signal
      servoB.detach();
      servoBAttached = false;

      Serial.println("Servo B finished");
    }

    else {
      Serial.println("Unknown command");
    }
  }
}