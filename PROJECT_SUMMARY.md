# Medi-Bot — Project & Dispense Mechanism Summary

A single-file reference covering the whole project and, in detail, how the drug
dispensing mechanism works. Compiled from the repo, the firmware README, the
abstract, and Lara's design explanations.

---

## 1. Project at a glance

- **Unit / project:** ENG30002 ETS Project, Swinburne University of Technology, Semester 1, 2026.
- **Group:** Group 4. **Supervisor:** Ilya Kavalchuk.
- **Project title:** Medi-Bot — Autonomous Drug Delivery & Dispensing Robot.
- **Problem:** Medication administration errors (MAEs) in hospitals — nurses spend up to ~40% of their time on medication tasks, errors occur in ~1 in 5 doses, worsened by workload, interruptions and staffing shortages.
- **Solution:** A mobile robot that navigates a simulated ward, verifies a patient by RFID, and dispenses the correct pills from an onboard rotary carousel; medication schedules and patient records are managed from a Raspberry Pi web app. It supports nurses rather than replacing them, adding an automated verification step before medication is released.

### Team & subsystem ownership

| Member | Subsystem | Responsibility |
| --- | --- | --- |
| **Lara Matheson** | Drug Storage & Dispensing | Rotary carousel + active single-pill dispenser, dispensing firmware |
| **Marshall Dawson** | Navigation & Motion | Differential-drive base, LiDAR SLAM/AMCL/path planning, ROS2, Arduino motor controller |
| **Bisandi Ahangamage** | Patient Interaction & UI | RFID verification, buzzer alerts, Flask web app (patients/meds/schedule/logging) |
| **Daniel Nugraha** | Hardware & Security | Two-layer acrylic chassis, motor sourcing, 3D-printed parts, RFID placement, physical protection |
| **Sandaru Arachchige** | Power System | 18 V battery, dual-rail power, DC–DC conversion, distribution |

---

## 2. System & control architecture

**Two controllers:**
- **Raspberry Pi** — runs the Flask scheduling / patient-management web app (SQLite database), high-level navigation (ROS2, LiDAR SLAM), and sends serial commands to the Arduino.
- **Arduino Mega 2560** — low-level real-time control: base drive motors (L298 + encoders + PID, from ROSArduinoBridge), the dispenser (stepper + 2 servos), RFID (MFRC522) reading, and the buzzer.

**Serial protocol (Pi ⇄ Arduino):** 57600 baud, carriage-return (`\r`) terminated.

| Command | Meaning | Reply |
| --- | --- | --- |
| `m <l> <r>` | Drive base (left/right speed) | — |
| `D <slot>` | Dispense one pill from slot 1–5 | `DEBUG…` → `OK` → `COMPLETE`, or `ERROR:…` |
| `z` / `z <ms>` | Single buzzer beep (Pi strings these into RFID patterns) | `OK` |
| `RESET` | Clear a multi-pill safety halt | `DEBUG:RESET_RECEIVED…` |
| `RFID:<uid>` | (Arduino → Pi) tag UID seen, ~1/sec | — |

**Combined firmware:** `medi_bot_controller` merges three original sketches — ROSArduinoBridge (navigation base, master protocol), the dispense library (carousel + servos + IR), and the RFID reader + buzzer code.

---

## 3. The dispense mechanism (detailed)

The dispenser has two conceptual halves: a **single compartment** that actively isolates one pill, and a **shared carousel + drive** that lets one set of motors operate five compartments.

### 3.1 Single compartment — the active single-pill isolator
- The body is a **vertical cylinder cut from PVC pipe** that holds a stack of pills. The dispensing mechanism sits inside it.
- Inside the cylinder is a **fixed platform angled at 55°** (originally modelled at 45°, then raised to 55° so gravity more reliably keeps the remaining pills down on the low side and feeds a single pill into the cut-out).
- A **round plate** the diameter of the cylinder bore rests on the angled platform, so the plate rotates about a **tilted axis**. The plate carries **one pill-sized cut-out**. A small **track** on the plate's underside limits its travel to **180°**.
- A central **arm** beneath the plate passes down through the platform to a **universal joint**, which connects to a **drive gear** underneath the compartment.
- **Dispense cycle:** the plate rotates ~180° **down** so the cut-out reaches the low side, where exactly **one pill** drops into it → the plate rotates ~180° back **up** so the cut-out reaches the high side, where a **hole in the cylinder wall** lets that single pill fall out into the collection chute. The remaining pills stay held inside the cylinder throughout (transport-safe).
- **Universal joint purpose:** because the plate spins on the *tilted* axis of the angled platform, it can't be driven directly by a flat-mounted motor. The U-joint converts the plate's angled rotation into a **vertical drive axis** at the base, so a servo mounted flat on the robot can turn it.
- **Fabrication:** all compartment parts are **3D printed except the universal joint** (a standard component). Every compartment is **identical** (designed once, replicated 5×).
- **Isolation validated:** when driven directly **by hand** (bypassing the gear coupling), the mechanism released a single pill on most attempts — the core isolation principle works.

### 3.2 Carousel & indexing — selecting between compartments
- **Why a carousel:** giving each of 5 compartments its own motor would raise cost, exhaust Arduino pins, and add complexity. Instead, **selection is separated from release** so one shared drive can service all compartments.
- A **laser-cut acrylic table** has **5 cut-outs**, each holding one compartment.
- The table is mounted on a **stepper motor** and sits **~5 cm above the base platform**, leaving room for the drive motors below.
- The stepper **homes against an IR sensor**, then steps a known count to bring each compartment to a fixed **"dispense position"** (slot positions at **22 / 62 / 102 / 142 / 182 steps** from home — a uniform 40-step spacing). It **re-homes before every cycle** so errors don't accumulate.
- The **dispense position** is simply the fixed point where a compartment aligns over the mounted servo drive.

### 3.3 The detachable servo drive — Servo A + Servo B
- **Servo A (drive):** mounted vertically, gear facing up; it **meshes with the gear on the underside** of whichever compartment is at the dispense position. It rotates **180° down then 180° up** — the cycle that captures and releases one pill.
- **Problem:** if Servo A stayed permanently meshed, the carousel couldn't rotate to a different compartment, so it must **engage and disengage**.
- **Servo B (engagement):** drives a **rack-and-pinion**. Servo B's pinion slides the rack (carrying Servo A) **forward to engage** and **back to disengage**.
- **Full sequence:** B pushes A forward → A's gear meshes with the compartment gear → A actuates (dispense) → B pulls A back (disengage) → the stepper indexes the next compartment → repeat.
- **Known weak link:** the **gear engagement** was the main source of dispensing unreliability — Servo A's gear didn't always align cleanly with the compartment gear when driven forward. The isolation mechanism itself was reliable; the *coupling added to save motors* introduced an alignment dependency.

### 3.4 Verification → open-loop operation
- **Designed:** an **IR sensor in the chute** (which funnels released pills into the patient's cup) to confirm exactly one pill dropped — flagging 0 or >1 as an error and alerting a nurse. This was intended to satisfy design requirement **DR5**.
- **What happened:** during full-system integration a **wiring fault caused the chute IR sensors to overheat and burn out**. With limited time before the demo, the damaged sensors were **removed**; only the homing IR remained.
- **Consequence:** the dispenser runs **open-loop** — it assumes a successful single release rather than confirming one. Firmware flags `HAVE_PILL_DETECT_IR 0` / `HAVE_CUP_IR 0`; it runs `forced_dispense_cycles = 2` and assumes success (note: 2 actuations risk dropping 2 pills; set to 1 for a single release). The only `D` errors still possible are `ERROR:BAD_COMPARTMENT` and `ERROR:HOME_FAIL`.

### 3.5 Control, power-hold & integration
- All three actuators (carousel stepper, Servo A, Servo B) are controlled by the **Arduino Mega**, which is prompted only with which compartment to dispense (`D <slot>`).
- On `D <slot>`: **home → index to slot → engage (Servo B) → dispense (Servo A down/up) → disengage (Servo B) → `COMPLETE`**.
- **Motors are held energised for the whole cycle** so the carousel keeps holding torque while the servos actuate — otherwise the servos knock the plate out of alignment. All motors are de-energised after the cycle to avoid holding/heating while idle.
- **Authorship:** Lara designed the dispensing logic and wrote/tested the firmware; the dispense command was integrated with **Bisandi**, who wrote the Raspberry-Pi side that communicates with the Arduino. The rotating carousel plate, stepper-motor casing and servo rack-and-pinion housing were **fabricated by Daniel from Lara's mechanism design**.

### 3.6 Design requirements scorecard

| Req. | Requirement | Outcome |
| --- | --- | --- |
| **DR1** | Store & select multiple medications from one unit | **Met** — 5-compartment carousel |
| **DR2** | Compact, suitable for a small mobile robot | **Met** — self-contained, mounted on the robot |
| **DR3** | Mechanically simple; minimal actuators | **Met** — 5 compartments driven by 1 stepper + 2 servos |
| **DR4** | Active single-pill isolation, transport-safe | **Met** — isolates one pill; stock held secure (reliability caveat: gear engagement) |
| **DR5** | Verifiable dispense via sensor feedback | **Not met** — IR removed during integration; runs open-loop |

### 3.7 Testing notes
- **Uniform mini M&M pellets** used as pill substitutes throughout.
- Dispenser tested **in isolation** (hand-driven: mostly reliable single-pill release) vs. **integrated** (gear-alignment misses reduce reliability).
- A structured **30+ trial** dataset (single-pill rate, accuracy, timing) was planned for Section 7.4 — to be collected.

---

## 4. Other subsystems (brief)

- **Hardware & Security (Daniel):** two-layer **acrylic** chassis (laser-cut), lower platform thickened **3 mm → 6 mm** after it flexed under load; **rear encoded 12 V DC gear motors + front castor** drive; LiDAR on a 3D-printed pillar ~20 cm up; **RFID reader** by the medicine-collection area with a "smiley face" case; removable side covers; battery in a protected case. Security = RFID patient verification + physical enclosure of electronics. (Wheel adaptor reprinted in **PETG** after PLA failed under load.)
- **Navigation & Motion (Marshall):** differential-drive base; Pi↔Arduino over USB serial (velocity → PWM); SSH/ROS2 development; **LiDAR SLAM (SLAM Toolbox)** chosen over vSLAM/RGB-D for reliability and ROS2 support, producing an occupancy grid; AMCL localisation; A*/DWA planning. Limitation: SLAM depends on accurate odometry/encoders.
- **Patient Interaction & UI (Bisandi):** **RFID (MFRC522)** patient verification with **buzzer** beep patterns (prompt / correct / wrong / SOS); **Flask web app** on the Pi managing patients, medications, schedules, RFID-gated dispensing and delivery-history logging; **SQLite** database; **UART serial** Pi↔Arduino (chosen over I²C/wireless); IR chosen over load cell for collection confirmation.
- **Power (Sandaru):** **18 V tool battery** → fused split → **12 V motor rail** + **buck converter to 5 V logic rail** (Pi/Arduino/sensors); dual-rail keeps motor noise off the control electronics.

---

## 5. Plan-vs-reality adaptations (managed changes)

- **Open-loop dispensing** after the verification IR sensors failed (scope decision to protect the demo).
- **Controller split** to Pi (app/nav) + Arduino Mega + NEMA-17 stepper, instead of the abstract's single-Pi servo table.
- **Three firmware sketches merged** into one program (`medi_bot_controller`).
- **Dispenser + delivery robot combined** into one mobile platform (not a separate dispensing station).
- **Patient verification:** barcode (abstract) → **RFID** (built).
- **Build overran:** first full assembly ~Weeks 10–11; testing compressed into Week 12; a last-minute acrylic-flex fix cost most of the reserved test day, so movement testing was sacrificed to ensure dispensing testing; demo extended ~1 week (≈10 May); report & presentation in Week 13.

---

## 6. Repository map

```
Medi-Bot/
├── app_code/
│   └── medi_bot_app.py            # Flask web app: patients, meds, schedule, RFID verify, dispatch
├── arduino_code/
│   ├── medi_bot_controller/       # COMBINED firmware (upload this)
│   │   ├── medi_bot_controller.ino
│   │   ├── dispense.cpp/.h        # dispense state machine
│   │   ├── stepper_control.cpp/.h # NEMA-17 carousel homing + indexing
│   │   ├── servo_control.cpp/.h   # Servo A (drive) + Servo B (engage)
│   │   ├── rfid_reader.cpp/.h      # MFRC522 + buzzer
│   │   ├── config.h, motor_settings.h
│   │   └── README.md              # serial protocol, pin map, caveats
│   ├── libraries/ (dispense, ros_arduino_bridge)
│   └── tests/ (stepper_position, servo_spin, finalrfid, rfid, stepper_servo_ir, two_servos)
└── CAD/ (Dispenser Prototype 1 & 2 — SolidWorks)
```

### Arduino Mega pin map (key)

| Subsystem | Pins |
| --- | --- |
| Dispenser stepper (STEP/DIR/EN) | 23 / 22 / 24 |
| Dispenser servos A / B | 26 / 27 |
| IR homing (only sensor fitted) | 13 (pill-detect 11 & cup 12 removed) |
| RFID MFRC522 (SS / RST) | 4 / 5 (SPI 50/51/52, 53 HIGH) |
| Buzzer | 10 |
| Base motor PWM (L298) | 6, 7, 9, 8 (enables 34/35 jumpered HIGH) |
