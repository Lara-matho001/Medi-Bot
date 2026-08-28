# Medi-Bot Final Report — Structure & Section Plan

## Context

This is a planning document for the **Group 4 ENG30002 Final Report** (group submission, due 12/06/2026). Lara is coordinating and will hand individual sections to groupmates, so every section below names an **owner** and gives a short, directive "what to write here" brief that a groupmate can execute without further explanation.

Three constraints drove this plan:
1. **It must beat the abstract.** The tutor allows copy/pasting the first (individual) reports' background and existing-tech, but the final report should add real value: the **actual built system**, **test results**, **integration**, and **reflection** — not just reworded abstract.
2. **Hybrid organisation (chosen).** A short shared base, then one self-contained section per person that merges existing-tech + design + build + results + a mini-reflection — so each subsystem reads as one flowing block, duplication is minimised, and ownership is obvious (names go next to each contribution).
3. **Reflection is the new headline.** One dedicated **Reflection & Future Work** chapter near the end (with a personal reflection per member), plus 1–3 sentence reflective notes woven into each subsystem section. This is where "what succeeded / what went wrong / what we'd do differently / future work" lives. The rubric explicitly rewards *evidence of managing the project*, and the gap between what was planned and what was built is strong material for it.

**Key fact for the report:** what the abstract proposed and what was actually built differ — this is an asset, not a problem. See "Plan-vs-Reality Deltas" near the end; weave these into §6, §12.4 and §14.

---

## Ownership at a glance (hand this table to the group)

| Chapter | Owner | Reuse from abstract? |
| --- | --- | --- |
| 1. Executive Summary | Daniel | Rewrite (now covers whole built system) |
| 3. Problem Statement | Marshall (or whoever wrote it first) | **Reuse** + add depth |
| 4. Introduction + 4.1 Aim | Lara | Light reuse + reflect |
| 4.2 Research Questions | All (each writes own) | New framing |
| 5. Background (importance, shared existing tech, challenges, focus) | Bisandi (shared lit) + group | **Reuse** the shared medical-automation lit, write ONCE |
| 6. Dispensing | **Lara** | Partial reuse + lots of new build/results |
| 7. Navigation | Marshall | Partial reuse + new build/results |
| 8. Sensors & Patient Interaction | Bisandi | Partial reuse + new build/results |
| 9. UI, Hardware & Security | Daniel | Partial reuse + new build/results |
| 10. Power System | Sandaru | Partial reuse + new build/results |
| 11. System Integration | Lara / group | New |
| 12. Project Management | Lara | New (Gantt reused) |
| 13. Deliverable Outcomes | Group | New |
| 14. Reflection & Future Work | Group + each member | **All new** |
| 15. Conclusion | Daniel | New |
| 16. References | Each owner keeps own; Lara merges later | Reuse |
| 17. Appendices | Lara collates | New |

---

## Full annotated outline

### 1. Executive Summary — *Daniel*
Half a page, non-specialist (write for a hospital manager, not a roboticist).
Cover: the problem (MAEs in hospitals) → the solution (autonomous medication delivery robot, "Medi-Bot") → the five subsystems in one line each → the headline outcome (a working prototype that drives to a patient, verifies them by RFID, and dispenses pills from a rotary carousel).
*Rubric: Argument for audience (1 pt) — easy mark, avoid jargon.*

### 2. Table of Contents
Auto-generate last.

### 3. Problem Statement — *Marshall (reuse)*
**Reuse the abstract's problem statement** (40% of nurse hours on admin, 1-in-5 dose errors, 500% rise, interruption rates, robotics reducing incidents 77.8%). Add one short paragraph making the *scope* explicit: this is a broad, complex, system-level problem with mechanical, electrical, software and human-factors dimensions — which is why it needs a multi-subsystem robot.
*Rubric: Project context & scope (3 pts) — make the breadth/complexity obvious.*

### 4. Introduction — *Lara*
Introduce the **whole system**: mobile delivery robot + onboard rotary dispenser + Raspberry-Pi scheduling app + Arduino motor/sensor controller. One short paragraph on what the robot must do end-to-end: navigate the ward → detect/approach patient → verify identity (RFID) → dispense the correct pills → confirm and move on.

**4.1 Research Aim — *Lara***
State the group aim (compact, reliable medication delivery + dispensing that integrates into one robot) and your individual aim (a compact, reliable single-pill dispensing mechanism). Add a forward-looking line: *"This was successfully / partially achieved in Medi-Bot; §14 reflects on what we'd extend."*
*Rubric: Aims (3 pts).*
_________________________________________________________
4.1 Research Aim — Lara
Within this, the drug storage and dispensing subsystem aimed to develop a self-contained mechanism that stores several medications and reliably releases a single correct pill on demand, while remaining small and mechanically simple enough to mount on the robot. This aim was largely achieved — the final rotary-carousel dispenser holds five medications and dispenses each on command from the robot — but only partially in one respect: reliable per-dose drop verification was not realised after the sensing hardware failed (Section 7.6), which defines the clearest direction for future work.
---------------------------------------------------------



**4.2 Research Questions — *each member writes their own***
State the 3 overarching group RQs (from the abstract), then a short per-member block of focused RQs:
- 4.2.1 Lara (dispensing reliability/compactness/simplicity)
- 4.2.2 Marshall (SLAM vs AMCL; path-planning; global+local integration)
- 4.2.3 Bisandi (sensor selection for safe HRI + verification)
- 4.2.4 Daniel (motors/materials/security choices)
- 4.2.5 Sandaru (battery/voltage architecture; dual-rail)
Each member adds **one sentence** answering/standing-on their own RQ (full reflection goes in §14).
*Rubric: Aims & research questions (3 pts).*

### 5. Background / Literature Review — *Bisandi (shared) + group*
This is the literature-review section; the rubric wants **critique, not summary**.

- **5.1 Importance of the Problem** — *group/reuse*: MAE rates, system pressure, robotics-as-trend. Cite multiple sources.
- **5.2 Existing Medication-Automation Technologies** — *Bisandi, write ONCE*: the shared lit that all five abstracts duplicated (ADCs, ADDs, ROWA Vmax, the Japanese Drug Station/Mini DimeRo, Moxi/TUG service robots, ASRS/carousel). For **each**, state in one line what it does **and its limitation / gap** that motivates Medi-Bot (e.g. "internal-pharmacy focus, no direct patient delivery"). This single section replaces the five near-identical copies in the abstracts.
- **5.3 Engineering Challenges** — *group*: synthesise the cross-cutting challenges (compactness vs mechanical complexity, dispensing accuracy, integrating subsystems on a mobile platform, 12-week/cost limits).
- **5.4 Focus of This Research** — *group*: narrow from the broad field to exactly what Medi-Bot tackles and why off-the-shelf systems don't just solve it.
*Rubric: Appraise current literature (3 pts) — each tech MUST have an explicit critique.*

> Subsystem-specific literature does **not** go here — it lives in each person's §6–§10 so their section is self-contained.

---

### Subsystem sections (§6–§10) — shared template

Every subsystem section uses the **same five-part template** so the report flows and marking is even. Put the owner's name in the heading (e.g. "6. Drug Storage & Dispensing — Lara").

| Sub-part | What to write |
| --- | --- |
| **X.1 Existing approaches & critique** | The subsystem-specific prior art (reuse your abstract's existing-tech), each with a limitation that motivates your choice. *Rubric: literature critique.* |
| **X.2 Design options & justification** | The alternatives you considered + why you picked yours. Include a comparison table where you have one. *Rubric: identify/evaluate/justify approaches (2 pts).* |
| **X.3 Methodology & build** | What you actually did and built — components, control method, fabrication, stages. Describe the **real** system, not the abstract's plan. *Rubric: methodology & research methods (2 pts).* |
| **X.4 Results & testing** | Measurements/trial data + what they show. Flag clearly if data is still to be collected. |
| **X.5 Mini-reflection** | 1–3 sentences: what worked, what didn't, pointer to §14 for the fuller reflection. |

### 6. Drug Storage & Dispensing — *Lara* (your main section)
Write this to the template above, grounded in the **actual build** (don't describe the abstract's Raspberry-Pi-servo-table plan as if it were built):

- **6.1 Existing storage/dispensing tech** — reuse abstract: matrix/ASRS, carousel, gravity chute, shelf/screw, vending, gumball — each with its limitation.
- **6.2 Design options & justification** — keep the **five-design comparison table** (Matrix / Carousel / Gravity Chute / Modular Cartridge / Rotary Table). Don't just show the table — explain the reasoning behind key scores, then justify Design 5 against the criteria and the "five rights." Mention the Gemini concept images if including visuals.
- **6.3 Methodology & build** — the real prototype: **Arduino Mega 2560** controller; **NEMA-17 stepper** indexing a 5-slot carousel (homed via IR on pin 13, slot positions {22,62,102,142,182} steps); **two HS-322HD positional servos** (chute + gate) for active single-pill release; combined firmware (`medi_bot_controller`) merging the ROS base controller, dispense library and RFID; serial protocol `D <slot>` at 57600 baud; motors held energised through a cycle then de-energised. Walk the 7 build stages from abstract §6.4, but describe what was *actually* done at each.
- **6.4 Results & testing** — **ACTION NEEDED: there is no trial data in the repo.** Your abstract promised a test report of **30+ trials** (dispensing accuracy / single-pill rate / release timing / transit reliability). Collect and tabulate this; put the table here and raw data in Appendix A7. Without it you lose easy methodology/results marks.
- **6.5 Mini-reflection** — e.g. the **two IR sensors (pill-detect + cup) failed and were removed**, forcing **open-loop dispensing** with 2 forced servo actuations (risk: may drop 2 pills; the closed-loop MISS-retry and MULTI_PILL safety halt are disabled). One line on what you'd fix → §14.

### 7. Navigation & Motion Platform — *Marshall*
Template above. 7.1 reuse abstract's SLAM/AMCL/A*/DWA lit (each critiqued). 7.2 the algorithm comparison table + justification. 7.3 the real build: differential-drive base on the L298 + encoders + PID (`ros_arduino_bridge`), and whatever SLAM/AMCL/path-planning was actually run on the Pi (link the ROS demo + simulated-environment videos in Appendix). 7.4 results (mapping accuracy, navigation success). 7.5 mini-reflection (note the encoder code is ATmega328-layout and unverified on the Mega — honest limitation).

### 8. Sensors & Patient Interaction — *Bisandi*
Template above. 8.1 reuse ultrasonic/PIR/RFID/load-cell lit + critique. 8.2 sensor-selection justification. 8.3 the real build — **RFID (MFRC522) patient verification + buzzer alert patterns** are the parts that actually shipped; be honest that ultrasonic/PIR/load-cell were planned and note which were realised. The RFID-verification logic lives in the Flask app (`verify_rfid_for_room`, beep patterns) — coordinate with Daniel on who claims it. 8.4 results (RFID read success rate, verification timing). 8.5 mini-reflection.

### 9. User Interface, Hardware & Security — *Daniel*
Template above. 9.1 reuse motors/materials/security lit + critique. 9.2 justify motor choices (DC drive, stepper steer, servo gate) + acrylic + the multi-layer security model. 9.3 the real build — **the Raspberry-Pi Flask web app (`medi_bot_app.py`)**: patient/medication/schedule database, the background scheduler, RFID-gated dispensing, delivery-history logging. This is the biggest software deliverable and is largely UI/control — claim it here (credit RFID logic to Bisandi, the `D <slot>` protocol to Lara). 9.4 results (does scheduling/verification/dispense flow work end-to-end). 9.5 mini-reflection.

### 10. Power System — *Sandaru*
Template above. 10.1 reuse DC-DC converter lit (buck/boost/synchronous) + critique. 10.2 justify buck + dual-rail. 10.3 the real build — 18V tool battery → fused split → 12V motor rail + buck to 5V logic rail (Pi/Arduino/sensors); show the wiring photo (abstract Fig 4). 10.4 results (measured rail voltages/ripple/runtime). 10.5 mini-reflection.

---

### 11. System Integration — *Lara / group*
How the subsystems become one robot. Describe the end-to-end flow and the interfaces: Pi Flask app ⇄ Arduino over **57600-baud serial** (`m` drive, `D <slot>` dispense, `z` buzzer, `RFID:` lines back); the RFID→verify→dispense sequence; what runs on the Pi vs the Mega. A simple block/data-flow diagram here pays off. This section shows the project is an integrated whole, not five disconnected parts.

### 12. Project Management — *Lara*
*Rubric: Plan & manage a research project (3 pts) + PM processes & tools (2 pts) — 5 pts total, don't underwrite this.*
- **12.1 Phases & timeline** — the 6 phases + milestones (Abstract Wk3, Mid-sem Wk6, Demo Wk9, Final Wk12/13). Reference the Gantt (Appendix A1).
- **12.2 PM tools & processes** — name them explicitly: Gantt chart, WBS task breakdown, division-of-responsibilities table, GitHub (the repo history is real evidence of iterative work), shared docs / meeting notes. "Professional use of multiple tools" is required for full marks — be explicit.
- **12.3 Division of responsibilities** — the subsystem-ownership table + each member's personal task list (Lara's LAR.1–LAR.13, etc.).
- **12.4 Evidence of managing to plan** — **the highest-value paragraph in the chapter.** Show you *managed*, not just planned: what finished on time, what slipped, and the **adaptations** made — e.g. IR sensors failed → pivoted to open-loop; control consolidated onto Arduino Mega + Pi rather than the originally-proposed single Raspberry-Pi servo table; three sketches merged into one firmware. Reference git commits as proof.

### 13. Deliverable Outcomes — *group*
State what was actually delivered, per subsystem: working dispenser prototype (CAD + physical + firmware), navigation/ROS demo, RFID verification + scheduling app, power board, and the test data. For each, one line on "what success looked like" (e.g. single pill per cycle, correct-patient-only dispense). Pull each owner's headline result up from their §X.4.

### 14. Reflection & Future Work — *group + each member* (the new centrepiece)
This is the chapter that lifts the report above the abstract. Structure:
- **14.1 What succeeded** — group: the wins (end-to-end dispense works, RFID gating works, scheduler works, one integrated robot).
- **14.2 What went wrong / plan-vs-reality** — group: honest account (sensor failures → open-loop; controller architecture changed; subsystems planned but not fully realised). Tie to the deltas list below.
- **14.3 What we'd do differently** — group: process + technical (earlier integration, more robust sensors, fewer last-minute pin reassignments).
- **14.4 Future work** — group: re-add closed-loop pill/cup sensing, stack carousels for more drugs, full SLAM autonomy, load-cell collection confirmation, etc.
- **14.5 Individual reflections** — *one short subsection per member* (14.5.1 Lara … 14.5.5 Sandaru): each reflects on their own subsystem — what they achieved vs their RQ, what they'd improve. (This satisfies your "personal reflection within the group chapter" choice.)

### 15. Conclusion — *Daniel*
Strong, non-specialist close: why the project matters and why the approach is sound, framed for a hospital manager / general engineer. No jargon.
*Rubric: Argument for audience (1 pt).*

### 16. References — *each owner keeps own; Lara merges last*
IEEE style (EndNote). Keep each person's list separate while drafting; Lara consolidates/dedupes at the end. Favour scientific sources.
*Rubric: Format & citations (0.5 pt).*

### 17. Appendices — *Lara collates*
A1 Gantt chart · A2 360° robot video · A3 successful pill-dispense video · A4 ROS demo video · A5 simulated-environment video · A6 CAD drawings/sketches · **A7 raw dispensing test data (30+ trials)**.

---

## Plan-vs-Reality deltas (weave into §6, §12.4, §14)

These are the concrete differences between the abstract plan and the built system — your richest reflection/management evidence:

1. **Controller:** abstract proposed a single **Raspberry Pi** driving a **servo** rotary table; built system uses a **Raspberry Pi (Flask app/scheduler) + Arduino Mega (real-time motor/sensor control)**, with a **NEMA-17 stepper** indexing the carousel and servos only for the gate.
2. **Sensing:** abstract proposed closed-loop dispensing with pill-detect + cup IR sensors; **both failed and were removed → open-loop** (2 forced actuations, no MISS-retry, no over-dispense halt). Honest limitation + clear future-work item.
3. **Firmware:** three separate sketches (ROS bridge, dispense, RFID) were **merged into one** `medi_bot_controller` — a real integration/management story (git history backs it).
4. **Scope realised vs planned:** RFID verification + scheduling + dispensing shipped and integrate; some planned sensors (ultrasonic/PIR/load-cell) and full SLAM autonomy were partially realised — state plainly which.

---

## How to use this plan (handoff checklist)

1. **Resolve one ownership question first:** who claims the Flask app (`medi_bot_app.py`)? Recommend **Daniel (UI/control)** as primary, crediting RFID-verification to Bisandi and the `D <slot>` dispense protocol to Lara. Settle this before people write §8/§9.
2. **Hand each member their rows** from the "Ownership at a glance" table + the subsystem template. Tell them: *reuse your abstract lit/design, then ADD §X.3 real build, §X.4 results, §X.5 mini-reflection, and your §14.5 personal reflection.*
3. **Collect the missing test data now** — Lara's 30+ dispense trials (and ideally each subsystem's headline measurements). This is the single biggest gap and an easy block of marks.
4. **Mark every contribution with a name** (as you intend) and keep references split per author until final merge.
5. **Two rubric reminders to print at the top of the shared doc:** (a) literature must be *critiqued*, not summarised; (b) PM must show *evidence of managing to plan* (§12.4), not just a Gantt.

## Rubric coverage check (20 pts)

| Rubric criterion | Lives in |
| --- | --- |
| Plan & manage a research project (3) | §12, esp. 12.4 |
| PM processes & tools (2) | §12.2 |
| Project context & scope (3) | §3 |
| Aims & research questions (3) | §4.1, §4.2 |
| Appraise current literature (3) | §5 + each §X.1 |
| Methodology & research methods (2) | each §X.3 + §11 |
| Identify/evaluate/justify approaches (2) | each §X.2 (esp. §6.2 table) |
| Clarity of writing (0.5) | whole report |
| Format & citations (0.5) | §16 + figures/tables |
| Argument for audience (1) | §1, §15 (non-specialist) |
