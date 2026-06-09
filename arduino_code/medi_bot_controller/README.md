# medi_bot_controller

Combined Arduino firmware for the Medi-Bot. It merges three sketches into one:

| Source sketch | What it contributes |
| --- | --- |
| `ros_arduino_bridge` | Differential-drive base controller (navigation), serial command protocol, encoders, PID. **Master protocol.** |
| `dispense` | Rotary carousel stepper + 2 dispense servos + IR sensors (homing, pill-detect, cup-taken). **Pill-detect & cup IRs removed in this build — runs open-loop, see Caveats.** |
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
| `z` / `z <ms>` | One buzzer beep. `z` = the default ~300 ms beep; `z <ms>` beeps for `<ms>`. This is a single-beep primitive — the Pi strings several together (with gaps) to make the RFID patterns: 2 beeps to prompt, 3 short for a wrong tag, 1 long for a correct tag, and a short/long/short "SOS" when all attempts fail. | `OK` |
| `RESET` | Only meaningful after an `ERROR:MULTI_PILL` halt; clears the safety stop | `DEBUG:RESET_RECEIVED_EXITING_HALT` |

### RFID (automatic, no command needed)
The reader polls continuously and prints every tag it sees as:
```
RFID:04 A2 1B 8C
```
rate-limited to about once per second. The Pi reads these lines, looks the UID up in
its database, then sends `D <slot>` (correct patient) or buzzer beeps (wrong patient).

## Typical dispense flow (matches the project plan)
1. Pi drives the robot to the patient using `m` / encoder commands.
2. Patient scans tag → Arduino prints `RFID:...`.
3. Pi verifies the UID against its database.
   - Wrong → Pi beeps the buzzer (`z <ms>` patterns: 3 short beeps for a wrong tag, an SOS when it gives up).
   - Correct → Pi beeps once (long), then sends the pills one at a time: `D 1\r`, wait for `COMPLETE`, `D 1\r`, wait, `D 3\r`, ...

## Pin map (Arduino Mega 2560)

| Subsystem | Pins |
| --- | --- |
| Base motor PWM (L298) | 6, 7, 9, 8 (backward, backward, forward, forward) |
| Base motor enable (L298) | 34, 35 (not physically wired; tied HIGH on board) |
| Base encoders | PORTD/PORTC pins (interrupt-driven; see encoder_driver.h) |
| Dispenser stepper (STEP/DIR/EN) | 23 / 22 / 24 |
| Dispenser servos A / B | 26 / 27 |
| IR homing (only sensor fitted) | 13 — pill-detect (pin 11) & cup (pin 12) removed, see note |
| RFID MFRC522 (SS / RST) | 4 / 5 |
| RFID hardware SPI | 50 (MISO), 51 (MOSI), 52 (SCK), 53 held HIGH |
| Buzzer | 10 |

### Note on motor-enable pins
The L298 motor enable lines are not physically wired in this build (the L298 board
has ENA/ENB jumpers tied HIGH). `RIGHT_MOTOR_ENABLE` / `LEFT_MOTOR_ENABLE` are set
to unused pins **34 / 35** in `motor_driver.h` so the code compiles cleanly. If you
ever wire the enable lines to the Arduino, update these pin numbers accordingly.

## Caveats / things to know
- **Open-loop dispensing (no pill/cup IR).** After two IR sensors failed, the
  pill-detection IR (pin 11) and cup IR (pin 12) were unplugged. The homing IR
  moved to pin 13. With `HAVE_PILL_DETECT_IR 0` / `HAVE_CUP_IR 0` in `config.h`,
  a `D <slot>` actuates the servos `forced_dispense_cycles` times (default **2**)
  and *assumes* success — there is no MISS retry and no MULTI_PILL halt, so the
  only `D` errors still possible are `ERROR:BAD_COMPARTMENT` and `ERROR:HOME_FAIL`.
  After dispensing it waits `cup_assumed_taken_ms` (default 5 s) and prints
  `COMPLETE`. **NOTE:** 2 actuations may drop 2 pills — set `forced_dispense_cycles`
  to 1 for a single release. To return to full sensing, re-wire the sensors and
  set the two flags back to `1`.
- **Dispensing is blocking.** A `D <slot>` command runs homing → rotate → dispense →
  wait-for-cup (up to 30 s) before the loop resumes. Drive commands are not processed
  during a dispense, and the base auto-stops anyway (intended: the robot is parked at
  the patient while dispensing).
- **Motors hold for the whole dispense cycle.** A `D <slot>` energises the stepper and
  servos for the entire cycle, so the carousel keeps its holding torque while the
  dispense servos actuate — otherwise the servos knock the plate out of alignment. When
  the cycle finishes, power to *all* motors is cut (stepper de-energised, servos
  detached) so nothing holds or heats while idle. The stepper also boots de-energised
  (it is re-homed at the start of every cycle). Exception: a `MULTI_PILL` halt keeps the
  motors powered/holding until `RESET`, then releases them.
- **Buzzer vs. drive PWM.** The buzzer is on pin 10 and `tone()` uses Timer2, which on
  the Mega also drives PWM on pins 9/10. Pin 9 is the right motor's forward line, so a
  beep suspends its PWM until the next motor command restores it. Don't beep while
  driving forward; beeping while parked is fine.
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
