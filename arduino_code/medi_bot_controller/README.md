# medi_bot_controller

Combined Arduino firmware for the Medi-Bot. It merges three sketches into one:

| Source sketch | What it contributes |
| --- | --- |
| `ros_arduino_bridge` | Differential-drive base controller (navigation), serial command protocol, encoders, PID. **Master protocol.** |
| `dispense` | Rotary carousel stepper + 2 dispense servos + 3 IR sensors (homing, pill-detect, cup-taken). |
| `finalrfid` | MFRC522 RFID patient verification + buzzer. |

The ROSArduinoBridge files are kept byte-for-byte except for **one** change: the two
unused L298 motor-enable pins were moved off 12/13 (see below). The dispenser and RFID
code is folded into the single `setup()` / `loop()` / `runCommand()` at four points
marked with `Medi-Bot` comments in `medi_bot_controller.ino`.

## Serial protocol

- **Baud: 57600** (the ROSArduinoBridge `BAUDRATE`). The Raspberry Pi **must** use 57600.
- Command lines are terminated by a **carriage return `\r`**.

### Existing ROSArduinoBridge commands (unchanged)
`m`, `o`, `e`, `r`, `u`, `p`, `a`, `d`, `w`, `x`, `c`, `b` — drive/encoder/PID/IO. e.g. `m 50 50\r`.

### New Medi-Bot commands
| Command | Meaning | Arduino replies |
| --- | --- | --- |
| `D <slot>` | Dispense one pill from slot **1-5**, e.g. `D 3\r` | `DEBUG:...` progress, then `OK` and `COMPLETE` on success, or `ERROR:BAD_COMPARTMENT` / `ERROR:HOME_FAIL` / `ERROR:MAX_RETRIES_REACHED` / `ERROR:MULTI_PILL` |
| `z` | Buzzer alert (wrong patient / needs a human) | `OK` |
| `RESET` | Only meaningful after an `ERROR:MULTI_PILL` halt; clears the safety stop | `DEBUG:RESET_RECEIVED_EXITING_HALT` |

### RFID (automatic, no command needed)
The reader polls continuously and prints every tag it sees as:
```
RFID:04 A2 1B 8C
```
rate-limited to about once per second. The Pi reads these lines, looks the UID up in
its database, then sends `D <slot>` (correct patient) or `z` (wrong patient).

## Typical dispense flow (matches the project plan)
1. Pi drives the robot to the patient using `m` / encoder commands.
2. Patient scans tag → Arduino prints `RFID:...`.
3. Pi verifies the UID against its database.
   - Wrong → Pi sends `z` (beep).
   - Correct → Pi sends the pills one at a time: `D 1\r`, wait for `COMPLETE`, `D 1\r`, wait, `D 3\r`, ...

## Pin map (Arduino Mega 2560)

| Subsystem | Pins |
| --- | --- |
| Base motor PWM (L298) | 5, 6, 9, 10 |
| Base motor enable (L298) | **30, 31** (moved from 12, 13 — see note) |
| Base encoders | ROSArduinoBridge `PORTD`/`PORTC` pins (unchanged) |
| Dispenser stepper (STEP/DIR/EN) | 23, 22, 24 |
| Dispenser servos A / B | 26 / 28 |
| IR homing / pill-detect / cup | 11 / 12 / 13 |
| RFID MFRC522 (SS / RST) | 8 / 7 |
| RFID hardware SPI | 50 (MISO), 51 (MOSI), 52 (SCK), 53 held HIGH |
| Buzzer | 4 |

### Note on motor-enable pins (the only ROS change)
The L298 enable lines originally used pins **12 and 13**, which clash with the
`ir_pill_detection_pin` (12) and `ir_medication_cup_pin` (13). Because the enable
lines are not wired to the Arduino in this build (L298 ENA/ENB jumpers tied HIGH),
`RIGHT_MOTOR_ENABLE` / `LEFT_MOTOR_ENABLE` were moved to spare pins **30 / 31** in
`motor_driver.h`. If you ever wire the enables to the Arduino, repoint them there.

## Caveats / things to know
- **Dispensing is blocking.** A `D <slot>` command runs homing → rotate → dispense →
  wait-for-cup (up to 30 s) before the loop resumes. Drive commands are not processed
  during a dispense, and the base auto-stops anyway (intended: the robot is parked at
  the patient while dispensing).
- **Buzzer vs. drive PWM.** `tone()` uses Timer2, which on the Mega also drives PWM on
  pins 9/10 (motor forward). Don't beep while driving forward; beeping while parked is
  fine (the next motor command restores PWM).
- **Encoders.** The ROSArduinoBridge `ARDUINO_ENC_COUNTER` pin-change interrupt code is
  written for an ATmega328 (Uno/Nano) `PORTD`/`PORTC` layout and was left unchanged per
  request. Verify encoder counting on your actual Mega wiring before relying on PID.

## Required Arduino libraries
- `Servo` (bundled with the IDE)
- `MFRC522` (by GithubCommunity / Miguel Balboa) — install via Library Manager

## Build
Open `medi_bot_controller.ino` in the Arduino IDE (folder name matches the `.ino`),
select **Arduino Mega 2560**, and upload. All `.ino`/`.cpp`/`.h` files in this folder
compile together as one sketch.
