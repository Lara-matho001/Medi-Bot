"""
Medi-Bot Flask Application v3
Medication dispensing robot — scheduling and patient management dashboard.

Database : SQLite (patients.db — created automatically on first run)
Hardware : Arduino connected via USB serial (/dev/ttyACM0)
Platform : Ubuntu 24.04 on Raspberry Pi
"""

# ==============================================================================
# IMPORTS
# ==============================================================================

import sqlite3
import threading
import time
from datetime import datetime

import serial
from flask import Flask, redirect, render_template_string, request
from markupsafe import Markup

# ==============================================================================
# CONFIGURATION
# ==============================================================================

# --- Database ---
DB_PATH = "patients.db"

# --- Arduino ---
SERIAL_PORT    = "/dev/ttyACM0"
SERIAL_BAUD    = 9600
SERIAL_TIMEOUT = 1

# --- Flask ---
FLASK_HOST = "0.0.0.0"
FLASK_PORT = 5000

# ==============================================================================
# FLASK APP
# ==============================================================================

app = Flask(__name__)

# ==============================================================================
# DATABASE
# ==============================================================================

def get_db():
    """Return a new SQLite connection."""
    return sqlite3.connect(DB_PATH)


def init_db():
    """
    Create tables, run any needed schema migrations, and seed default patients.

    Safe to call on both a fresh database and an existing one from an older
    version of the app — migrations add missing columns without losing data.
    """
    conn = get_db()
    c = conn.cursor()

    # ── patients ──────────────────────────────────────────────────────────────
    c.execute("""
        CREATE TABLE IF NOT EXISTS patients (
            id         INTEGER PRIMARY KEY AUTOINCREMENT,
            first_name TEXT NOT NULL,
            last_name  TEXT NOT NULL,
            room       TEXT NOT NULL,
            medication TEXT NOT NULL,
            rfid       TEXT NOT NULL
        )
    """)

    # Migration: older schema used a single "name" column
    c.execute("PRAGMA table_info(patients)")
    patient_cols = {row[1] for row in c.fetchall()}

    if "name" in patient_cols and "first_name" not in patient_cols:
        c.execute("ALTER TABLE patients ADD COLUMN first_name TEXT NOT NULL DEFAULT ''")
        c.execute("ALTER TABLE patients ADD COLUMN last_name  TEXT NOT NULL DEFAULT ''")
        c.execute("UPDATE patients SET first_name = name, last_name = ''")
        print("DB migration: patients.name → first_name + last_name")

    # ── schedules ─────────────────────────────────────────────────────────────
    c.execute("""
        CREATE TABLE IF NOT EXISTS schedules (
            id                  INTEGER PRIMARY KEY AUTOINCREMENT,
            patient_id          INTEGER NOT NULL,
            patient_name        TEXT,
            room                TEXT,
            medication          TEXT,
            delivery_date       TEXT,
            delivery_time       TEXT,
            recurring           INTEGER DEFAULT 0,
            last_triggered_date TEXT,
            status              TEXT,
            FOREIGN KEY (patient_id) REFERENCES patients(id)
        )
    """)

    # Migration: add new columns if upgrading from older schema
    c.execute("PRAGMA table_info(schedules)")
    sched_cols = {row[1] for row in c.fetchall()}
    for col, defn in [
        ("delivery_date",       "TEXT"),
        ("recurring",           "INTEGER DEFAULT 0"),
        ("last_triggered_date", "TEXT"),
    ]:
        if col not in sched_cols:
            c.execute(f"ALTER TABLE schedules ADD COLUMN {col} {defn}")
            print(f"DB migration: added schedules.{col}")

    # ── seed data ─────────────────────────────────────────────────────────────
    c.execute("SELECT COUNT(*) FROM patients")
    if c.fetchone()[0] == 0:
        seed = [
            ("John",  "Smith", "101", "Paracetamol", "PATIENT001"),
            ("Alice", "Jones", "102", "Aspirin",     "PATIENT002"),
            ("Bob",   "Brown", "103", "Ibuprofen",   "PATIENT003"),
        ]
        for row in seed:
            c.execute(
                "INSERT INTO patients (first_name, last_name, room, medication, rfid)"
                " VALUES (?,?,?,?,?)",
                row,
            )

    conn.commit()
    conn.close()


# ==============================================================================
# ARDUINO
# ==============================================================================

arduino           = None
arduino_connected = False


def connect_arduino():
    """Attempt to open the serial connection to the Arduino."""
    global arduino, arduino_connected
    try:
        arduino = serial.Serial(SERIAL_PORT, SERIAL_BAUD, timeout=SERIAL_TIMEOUT)
        time.sleep(2)
        arduino_connected = True
        print(f"Arduino connected on {SERIAL_PORT}")
    except serial.SerialException as e:
        arduino_connected = False
        print(f"Warning: Arduino not available — {e}")
        print("Running without hardware. RFID verification will be skipped.")


def normalise_rfid(rfid):
    """Strip known prefixes and normalise an RFID string to uppercase."""
    return rfid.replace("RFID:", "").replace("UID:", "").strip().upper()


def wait_for_rfid_scan():
    """Block until a valid RFID line is received from the Arduino."""
    print("Waiting for RFID scan...")
    while True:
        line = arduino.readline().decode(errors="ignore").strip()
        if line.startswith("RFID:") or line.startswith("UID:"):
            scanned = normalise_rfid(line)
            print(f"Scanned RFID: {scanned}")
            return scanned


def verify_rfid_for_room(room):
    """
    Look up the expected RFID for a room, scan the presented tag, and compare.
    Sends CORRECT or WRONG back to the Arduino. Returns True on a match.
    """
    if arduino is None:
        print(f"Arduino unavailable — skipping RFID check for room {room}")
        return False

    conn = get_db()
    c = conn.cursor()
    c.execute("SELECT rfid FROM patients WHERE room = ?", (room,))
    result = c.fetchone()
    conn.close()

    if result is None:
        print(f"No RFID registered for room {room}")
        return False

    expected = normalise_rfid(result[0])
    scanned  = wait_for_rfid_scan()
    print(f"Expected: {expected} | Got: {scanned}")

    if scanned == expected:
        arduino.write(b"CORRECT\n")
        return True

    arduino.write(b"WRONG\n")
    return False


# ==============================================================================
# SCHEDULER
# ==============================================================================

def scheduler_loop():
    """
    Background thread: poll every 10 seconds for deliveries due right now.

    One-time schedules: match by delivery_date (today) AND delivery_time.
    Recurring schedules: match by delivery_time only, skipping days already
                         triggered (tracked via last_triggered_date).

    After a recurring job completes, status resets to 'Pending' and
    last_triggered_date is set to today so it does not fire again until tomorrow.
    """
    while True:
        today = datetime.now().strftime("%Y-%m-%d")
        now   = datetime.now().strftime("%H:%M")

        conn = get_db()
        c = conn.cursor()
        c.execute("""
            SELECT schedules.id, patients.room, schedules.recurring
            FROM   schedules
            JOIN   patients ON schedules.patient_id = patients.id
            WHERE  schedules.delivery_time = ?
              AND  schedules.status = 'Pending'
              AND  (
                       (schedules.recurring = 0 AND schedules.delivery_date = ?)
                    OR (schedules.recurring = 1 AND (
                           schedules.last_triggered_date IS NULL
                        OR schedules.last_triggered_date != ?
                       ))
                   )
        """, (now, today, today))
        jobs = c.fetchall()

        for job_id, room, is_recurring in jobs:
            c.execute(
                "UPDATE schedules SET status = 'Waiting for RFID' WHERE id = ?",
                (job_id,),
            )
            conn.commit()

            rfid_ok = verify_rfid_for_room(room)

            if is_recurring:
                # Reset to Pending for tomorrow; record today so it won't re-fire
                c.execute(
                    "UPDATE schedules SET status = 'Pending', last_triggered_date = ? WHERE id = ?",
                    (today, job_id),
                )
                result_label = "RFID Correct" if rfid_ok else "Wrong RFID"
                print(f"Recurring job {job_id} (room {room}): {result_label}")
                # TODO: trigger dispense on success: arduino.write(b"DISPENSE\n")
            else:
                if rfid_ok:
                    c.execute(
                        "UPDATE schedules SET status = 'RFID Correct' WHERE id = ?",
                        (job_id,),
                    )
                    # TODO: trigger dispense: arduino.write(b"DISPENSE\n")
                else:
                    c.execute(
                        "UPDATE schedules SET status = 'Wrong RFID' WHERE id = ?",
                        (job_id,),
                    )

        conn.commit()
        conn.close()

        time.sleep(10)


# ==============================================================================
# TEMPLATES
# ==============================================================================

STYLE = """
<style>
    * { box-sizing: border-box; }

    body {
        font-family: Arial, sans-serif;
        background: linear-gradient(135deg, #081a33, #123c69);
        color: white;
        margin: 0;
        padding: 0;
        font-size: 22px;
        min-height: 100vh;
    }

    /* ── Navigation ─────────────────────────────────── */
    .nav {
        background: rgba(0, 0, 0, 0.35);
        padding: 16px 60px;
        display: flex;
        align-items: center;
        gap: 6px;
        border-bottom: 1px solid rgba(255,255,255,0.1);
    }

    .nav .brand {
        font-size: 22px;
        font-weight: bold;
        color: #d7e8ff;
        margin-right: auto;
        letter-spacing: 0.5px;
    }

    .nav a {
        color: rgba(255,255,255,0.72);
        font-size: 20px;
        font-weight: bold;
        text-decoration: none;
        padding: 10px 20px;
        border-radius: 10px;
        margin: 0;
        transition: background 0.18s, color 0.18s;
    }

    .nav a:hover {
        background: rgba(255,255,255,0.15);
        color: white;
    }

    /* ── Page wrapper ────────────────────────────────── */
    .page { padding: 50px 80px; }

    h1 { font-size: 50px; margin-bottom: 12px; }
    h2 { font-size: 32px; margin-top: 0; }

    .subtitle {
        font-size: 20px;
        color: #c8dcf5;
        margin-bottom: 36px;
    }

    /* ── Status chips ────────────────────────────────── */
    .status-bar {
        display: flex;
        gap: 16px;
        margin-bottom: 38px;
        flex-wrap: wrap;
    }

    .chip {
        display: inline-flex;
        align-items: center;
        gap: 8px;
        padding: 7px 18px;
        border-radius: 30px;
        font-size: 17px;
        font-weight: bold;
    }

    .chip.ok   { background: rgba(40,167,69,0.22);  border: 1px solid #28a745; color: #7dffaa; }
    .chip.warn { background: rgba(255,193,7,0.18);  border: 1px solid #ffc107; color: #ffe082; }

    .dot { width: 9px; height: 9px; border-radius: 50%; display: inline-block; }
    .dot.green  { background: #28a745; }
    .dot.yellow { background: #ffc107; }

    /* ── White box (forms) ───────────────────────────── */
    .box {
        background: #f2f6fb;
        color: #111;
        padding: 38px 40px;
        border-radius: 22px;
        max-width: 900px;
        margin-bottom: 32px;
        box-shadow: 0 8px 28px rgba(0,0,0,0.25);
    }

    /* ── Home cards ──────────────────────────────────── */
    .home-grid {
        display: flex;
        gap: 28px;
        margin-top: 8px;
        flex-wrap: wrap;
    }

    .home-card {
        background: white;
        color: #111;
        width: 370px;
        min-height: 210px;
        padding: 32px;
        border-radius: 22px;
        box-shadow: 0 8px 28px rgba(0,0,0,0.22);
        transition: transform 0.15s, box-shadow 0.15s;
    }

    .home-card:hover {
        transform: translateY(-5px);
        box-shadow: 0 16px 40px rgba(0,0,0,0.32);
    }

    .home-card h2 { font-size: 28px; margin-bottom: 12px; color: #0b1f3a; }
    .home-card p  { font-size: 19px; color: #555; line-height: 1.45; }

    .main-button {
        display: block;
        background: #0b1f3a;
        color: white;
        text-align: center;
        padding: 15px;
        border-radius: 13px;
        margin-top: 20px;
        font-size: 21px;
        font-weight: bold;
        text-decoration: none;
        transition: background 0.18s;
    }

    .main-button:hover { background: #1a3a6b; color: white; }

    /* ── Form inputs ─────────────────────────────────── */
    label {
        font-size: 20px;
        font-weight: bold;
        display: block;
        margin-top: 10px;
        color: #111;
    }

    input, select {
        width: 100%;
        padding: 18px 20px;
        margin-top: 8px;
        margin-bottom: 16px;
        font-size: 21px;
        border-radius: 12px;
        border: 2px solid #b8c4d1;
        outline: none;
        transition: border-color 0.2s;
        color: #111;
    }

    input:focus, select:focus { border-color: #0b1f3a; }

    button {
        width: 100%;
        padding: 18px;
        margin-top: 6px;
        font-size: 21px;
        border-radius: 12px;
        border: none;
        background: #0b1f3a;
        color: white;
        font-weight: bold;
        cursor: pointer;
        transition: background 0.18s;
    }

    button:hover { background: #1a3a6b; }

    /* ── Side-by-side name fields ────────────────────── */
    .name-row {
        display: flex;
        gap: 20px;
    }

    .name-row > div { flex: 1; }

    /* ── Recurring checkbox row ──────────────────────── */
    .check-row {
        display: flex;
        align-items: center;
        gap: 14px;
        margin: 6px 0 18px;
        padding: 16px 18px;
        background: #e8f0fb;
        border-radius: 12px;
        border: 2px solid #b8c4d1;
        cursor: pointer;
    }

    .check-row input[type="checkbox"] {
        width: 26px;
        height: 26px;
        margin: 0;
        padding: 0;
        flex-shrink: 0;
        cursor: pointer;
        accent-color: #0b1f3a;
    }

    .check-row label {
        font-size: 20px;
        color: #111;
        margin: 0;
        cursor: pointer;
        user-select: none;
    }

    /* ── Links ───────────────────────────────────────── */
    a { color: white; text-decoration: none; }

    .btn {
        display: inline-block;
        background: #0b1f3a;
        color: white;
        padding: 13px 26px;
        border-radius: 12px;
        font-size: 19px;
        font-weight: bold;
        text-decoration: none;
        transition: background 0.18s;
        margin-right: 12px;
    }

    .btn:hover { background: #1a3a6b; color: white; }

    .darklink {
        color: #0b1f3a;
        font-weight: bold;
        padding: 4px 10px;
        border-radius: 8px;
        transition: background 0.15s;
        margin-right: 4px;
    }

    .darklink:hover { background: #dde8f8; }

    .danger-link {
        color: #b91c1c;
        font-weight: bold;
        padding: 4px 10px;
        border-radius: 8px;
        transition: background 0.15s;
    }

    .danger-link:hover { background: #fde8e8; }

    /* ── Tables ──────────────────────────────────────── */
    table {
        background: white;
        color: #111;
        border-collapse: collapse;
        width: 100%;
        max-width: 1400px;
        font-size: 20px;
        border-radius: 18px;
        overflow: hidden;
        box-shadow: 0 8px 28px rgba(0,0,0,0.22);
    }

    thead th { background: #0b1f3a; color: white; font-size: 21px; }
    th, td   { padding: 18px 22px; border: 1px solid #dde3ea; text-align: left; }
    tbody tr:hover { background: #f0f5fb; }

    /* ── Status / type badges ────────────────────────── */
    .badge {
        display: inline-block;
        padding: 5px 14px;
        border-radius: 20px;
        font-size: 17px;
        font-weight: bold;
    }

    .badge-pending   { background: #fff3cd; color: #856404; }
    .badge-waiting   { background: #ffe0b2; color: #b25e00; }
    .badge-correct   { background: #d4edda; color: #155724; }
    .badge-wrong     { background: #f8d7da; color: #721c24; }
    .badge-recurring { background: #e9d8fd; color: #5b21b6; }
    .badge-default   { background: #e2e8f0; color: #444;    }

    /* ── Confirmation card ───────────────────────────── */
    .confirm-card {
        background: #f2f6fb;
        color: #111;
        padding: 42px;
        border-radius: 22px;
        max-width: 600px;
        box-shadow: 0 8px 28px rgba(0,0,0,0.25);
        margin-top: 18px;
    }

    .confirm-card h2  { color: #0b1f3a; margin-bottom: 18px; font-size: 30px; }
    .confirm-card p   { font-size: 20px; margin: 10px 0; color: #333; }
    .confirm-card .actions { margin-top: 28px; display: flex; gap: 16px; flex-wrap: wrap; }

    /* ── Empty state ─────────────────────────────────── */
    .empty-state {
        color: #aac4e0;
        font-size: 22px;
        padding: 50px 20px;
        text-align: center;
    }
</style>
"""

NAV = """
<nav class="nav">
    <span class="brand">Medi-Bot</span>
    <a href="/">Home</a>
    <a href="/schedule_page">Schedule</a>
    <a href="/patients">Patients</a>
    <a href="/delivery_history">History</a>
</nav>
"""


@app.template_filter("status_badge")
def status_badge(text):
    """Jinja2 filter: wrap a schedule status string in a colour-coded badge."""
    t = (text or "").lower()
    if "correct" in t:
        css = "badge-correct"
    elif "wrong" in t:
        css = "badge-wrong"
    elif "waiting" in t:
        css = "badge-waiting"
    elif "pending" in t:
        css = "badge-pending"
    else:
        css = "badge-default"
    return Markup(f'<span class="badge {css}">{text}</span>')


_HEAD = (
    "<html><head>"
    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
)

# ---------- Page templates ----------

PAGE_HOME = (
    _HEAD + "<title>Medication Robot System</title>" + STYLE + "</head><body>" + NAV +
    """
    <div class="page">
        <h1>Medication Robot System</h1>
        <p class="subtitle">Hospital medication delivery dashboard</p>

        <div class="status-bar">
            {% if arduino_ok %}
            <span class="chip ok"><span class="dot green"></span>Arduino Connected</span>
            {% else %}
            <span class="chip warn"><span class="dot yellow"></span>Arduino Not Connected</span>
            {% endif %}
            <span class="chip ok"><span class="dot green"></span>Database: SQLite</span>
        </div>

        <div class="home-grid">
            <div class="home-card">
                <h2>Schedule Delivery</h2>
                <p>Select a patient, set the delivery date and time, and queue the task for the robot.</p>
                <a class="main-button" href="/schedule_page">Open Scheduler</a>
            </div>
            <div class="home-card">
                <h2>Manage Patients</h2>
                <p>Add, edit, or remove patient details including room number and medication.</p>
                <a class="main-button" href="/patients">Open Patients</a>
            </div>
            <div class="home-card">
                <h2>Delivery History</h2>
                <p>View all scheduled deliveries, RFID verification results, and statuses.</p>
                <a class="main-button" href="/delivery_history">View History</a>
            </div>
        </div>
    </div>
    </body></html>
    """
)

PAGE_SCHEDULE = (
    _HEAD + "<title>Schedule Delivery</title>" + STYLE + "</head><body>" + NAV +
    """
    <div class="page">
        <h1>Schedule Medication Delivery</h1>

        <div class="box">
            <form method="POST" action="/schedule">

                <label>Select Patient:</label>
                <select name="patient_id">
                    {% for p in patients %}
                    <option value="{{ p[0] }}">
                        #{{ p[0] }} — {{ p[1] }} {{ p[2] }} — Room {{ p[3] }} — {{ p[4] }}
                    </option>
                    {% endfor %}
                </select>

                <div id="date_row">
                    <label>Delivery Date:</label>
                    <input type="date" name="delivery_date" id="delivery_date" required>
                </div>

                <label>Delivery Time:</label>
                <input type="time" name="delivery_time" required>

                <label style="margin-top:18px;">Schedule Type:</label>
                <div class="check-row" onclick="document.getElementById('recurring_check').click()">
                    <input type="checkbox" name="recurring" id="recurring_check" value="1">
                    <label for="recurring_check">Repeat every day at this time</label>
                </div>

                <button type="submit">Schedule Delivery</button>
            </form>
        </div>
    </div>

    <script>
    (function () {
        var cb        = document.getElementById('recurring_check');
        var dateRow   = document.getElementById('date_row');
        var dateInput = document.getElementById('delivery_date');

        // Default to today's date
        var today = new Date();
        var yyyy  = today.getFullYear();
        var mm    = String(today.getMonth() + 1).padStart(2, '0');
        var dd    = String(today.getDate()).padStart(2, '0');
        dateInput.value = yyyy + '-' + mm + '-' + dd;

        cb.addEventListener('change', function () {
            if (this.checked) {
                dateRow.style.display = 'none';
                dateInput.removeAttribute('required');
                dateInput.value = '';
            } else {
                dateRow.style.display = 'block';
                dateInput.setAttribute('required', '');
                dateInput.value = yyyy + '-' + mm + '-' + dd;
            }
        });

        // Stop the check-row onclick from double-firing
        cb.addEventListener('click', function (e) { e.stopPropagation(); });
    })();
    </script>
    </body></html>
    """
)

PAGE_SCHEDULE_CONFIRM = (
    _HEAD + "<title>Delivery Scheduled</title>" + STYLE + "</head><body>" + NAV +
    """
    <div class="page">
        <h1>Delivery Scheduled</h1>

        <div class="confirm-card">
            <h2>{{ patient_name }}</h2>
            <p><strong>Room:</strong> {{ room }}</p>
            <p><strong>Medication:</strong> {{ medication }}</p>
            {% if recurring %}
            <p><strong>Schedule:</strong>
                <span class="badge badge-recurring">↻ Daily at {{ delivery_time }}</span>
            </p>
            {% else %}
            <p><strong>Date:</strong> {{ delivery_date }}</p>
            <p><strong>Time:</strong> {{ delivery_time }}</p>
            {% endif %}
            <p><strong>Status:</strong> <span class="badge badge-pending">Pending</span></p>
            <div class="actions">
                <a class="btn" href="/schedule_page">Schedule Another</a>
                <a class="btn" href="/delivery_history">View History</a>
            </div>
        </div>
    </div>
    </body></html>
    """
)

PAGE_PATIENTS = (
    _HEAD + "<title>Manage Patients</title>" + STYLE + "</head><body>" + NAV +
    """
    <div class="page">
        <h1>Manage Patients</h1>

        <div class="box">
            <h2>Add New Patient</h2>
            <form method="POST" action="/add_patient">

                <label>Name:</label>
                <div class="name-row">
                    <div>
                        <input name="first_name" placeholder="First name" required>
                    </div>
                    <div>
                        <input name="last_name"  placeholder="Surname"    required>
                    </div>
                </div>

                <label>Room Number:</label>
                <input name="room"       placeholder="e.g. 101"         required>
                <label>Medication:</label>
                <input name="medication" placeholder="e.g. Paracetamol" required>
                <label>RFID Tag ID:</label>
                <input name="rfid"       placeholder="e.g. PATIENT004"  required>
                <button type="submit">Add Patient</button>
            </form>
        </div>

        <h2>Patient Database</h2>

        {% if patients %}
        <table>
            <thead>
                <tr>
                    <th>Patient ID</th>
                    <th>First Name</th>
                    <th>Surname</th>
                    <th>Room</th>
                    <th>Medication</th>
                    <th>RFID Tag</th>
                    <th>Actions</th>
                </tr>
            </thead>
            <tbody>
                {% for p in patients %}
                <tr>
                    <td>#{{ p[0] }}</td>
                    <td>{{ p[1] }}</td>
                    <td>{{ p[2] }}</td>
                    <td>{{ p[3] }}</td>
                    <td>{{ p[4] }}</td>
                    <td>{{ p[5] }}</td>
                    <td>
                        <a class="darklink" href="/edit_patient/{{ p[0] }}">Edit</a>
                        <a class="danger-link" href="/delete_patient/{{ p[0] }}"
                           onclick="return confirm('Delete this patient? This cannot be undone.')">Delete</a>
                    </td>
                </tr>
                {% endfor %}
            </tbody>
        </table>
        {% else %}
        <div class="empty-state">No patients registered yet — add one above.</div>
        {% endif %}
    </div>
    </body></html>
    """
)

PAGE_EDIT_PATIENT = (
    _HEAD + "<title>Edit Patient</title>" + STYLE + "</head><body>" + NAV +
    """
    <div class="page">
        <h1>Edit Patient</h1>

        <div class="box">
            <form method="POST" action="/update_patient/{{ patient[0] }}">

                <label>Name:</label>
                <div class="name-row">
                    <div>
                        <input name="first_name" value="{{ patient[1] }}" required>
                    </div>
                    <div>
                        <input name="last_name"  value="{{ patient[2] }}" required>
                    </div>
                </div>

                <label>Room:</label>
                <input name="room"       value="{{ patient[3] }}" required>
                <label>Medication:</label>
                <input name="medication" value="{{ patient[4] }}" required>
                <label>RFID Tag:</label>
                <input name="rfid"       value="{{ patient[5] }}" required>
                <button type="submit">Save Changes</button>
            </form>
        </div>

        <a href="/patients">← Back to Patients</a>
    </div>
    </body></html>
    """
)

PAGE_DELIVERY_HISTORY = (
    _HEAD
    + "<meta http-equiv='refresh' content='15'>"
    + "<title>Delivery History</title>" + STYLE + "</head><body>" + NAV +
    """
    <div class="page">
        <h1>Delivery History</h1>
        <p class="subtitle">Refreshes automatically every 15 seconds</p>

        {% if deliveries %}
        <table>
            <thead>
                <tr>
                    <th>Patient</th>
                    <th>Room</th>
                    <th>Medication</th>
                    <th>Date</th>
                    <th>Time</th>
                    <th>Status</th>
                </tr>
            </thead>
            <tbody>
                {% for d in deliveries %}
                <tr>
                    <td>{{ d[0] }}</td>
                    <td>{{ d[1] }}</td>
                    <td>{{ d[2] }}</td>
                    <td>
                        {% if d[5] %}
                        <span class="badge badge-recurring">↻ Daily</span>
                        {% else %}
                        {{ d[3] or '—' }}
                        {% endif %}
                    </td>
                    <td>{{ d[4] }}</td>
                    <td>{{ d[6] | status_badge }}</td>
                </tr>
                {% endfor %}
            </tbody>
        </table>
        {% else %}
        <div class="empty-state">No deliveries scheduled yet.</div>
        {% endif %}
    </div>
    </body></html>
    """
)

# ==============================================================================
# ROUTES
# ==============================================================================

@app.route("/")
def home():
    return render_template_string(PAGE_HOME, arduino_ok=arduino_connected)


@app.route("/schedule_page")
def schedule_page():
    conn = get_db()
    c = conn.cursor()
    c.execute("SELECT id, first_name, last_name, room, medication, rfid FROM patients")
    patients = c.fetchall()
    conn.close()
    return render_template_string(PAGE_SCHEDULE, patients=patients)


@app.route("/schedule", methods=["POST"])
def schedule():
    patient_id    = request.form["patient_id"]
    delivery_time = request.form["delivery_time"]
    recurring     = 1 if request.form.get("recurring") == "1" else 0
    delivery_date = request.form.get("delivery_date") or None
    if recurring:
        delivery_date = None

    conn = get_db()
    c = conn.cursor()
    c.execute(
        "SELECT first_name, last_name, room, medication FROM patients WHERE id = ?",
        (patient_id,),
    )
    first_name, last_name, room, medication = c.fetchone()
    patient_name = f"{first_name} {last_name}".strip()

    c.execute(
        "INSERT INTO schedules"
        " (patient_id, patient_name, room, medication, delivery_date, delivery_time, recurring, status)"
        " VALUES (?,?,?,?,?,?,?,'Pending')",
        (patient_id, patient_name, room, medication, delivery_date, delivery_time, recurring),
    )
    conn.commit()
    conn.close()

    return render_template_string(
        PAGE_SCHEDULE_CONFIRM,
        patient_name=patient_name,
        room=room,
        medication=medication,
        delivery_date=delivery_date,
        delivery_time=delivery_time,
        recurring=recurring,
    )


@app.route("/patients")
def patients():
    conn = get_db()
    c = conn.cursor()
    c.execute("SELECT id, first_name, last_name, room, medication, rfid FROM patients")
    patients_list = c.fetchall()
    conn.close()
    return render_template_string(PAGE_PATIENTS, patients=patients_list)


@app.route("/add_patient", methods=["POST"])
def add_patient():
    first_name = request.form["first_name"]
    last_name  = request.form["last_name"]
    room       = request.form["room"]
    medication = request.form["medication"]
    rfid       = request.form["rfid"]

    conn = get_db()
    c = conn.cursor()
    c.execute(
        "INSERT INTO patients (first_name, last_name, room, medication, rfid) VALUES (?,?,?,?,?)",
        (first_name, last_name, room, medication, rfid),
    )
    conn.commit()
    conn.close()
    return redirect("/patients")


@app.route("/edit_patient/<int:patient_id>")
def edit_patient(patient_id):
    conn = get_db()
    c = conn.cursor()
    c.execute(
        "SELECT id, first_name, last_name, room, medication, rfid FROM patients WHERE id = ?",
        (patient_id,),
    )
    patient = c.fetchone()
    conn.close()
    return render_template_string(PAGE_EDIT_PATIENT, patient=patient)


@app.route("/update_patient/<int:patient_id>", methods=["POST"])
def update_patient(patient_id):
    first_name = request.form["first_name"]
    last_name  = request.form["last_name"]
    room       = request.form["room"]
    medication = request.form["medication"]
    rfid       = request.form["rfid"]

    conn = get_db()
    c = conn.cursor()
    c.execute(
        "UPDATE patients SET first_name=?, last_name=?, room=?, medication=?, rfid=? WHERE id=?",
        (first_name, last_name, room, medication, rfid, patient_id),
    )
    conn.commit()
    conn.close()
    return redirect("/patients")


@app.route("/delete_patient/<int:patient_id>")
def delete_patient(patient_id):
    conn = get_db()
    c = conn.cursor()
    c.execute("DELETE FROM patients WHERE id = ?", (patient_id,))
    conn.commit()
    conn.close()
    return redirect("/patients")


@app.route("/delivery_history")
def delivery_history():
    conn = get_db()
    c = conn.cursor()
    c.execute("""
        SELECT patient_name, room, medication, delivery_date, delivery_time, recurring, status
        FROM   schedules
        ORDER  BY id DESC
    """)
    deliveries = c.fetchall()
    conn.close()
    return render_template_string(PAGE_DELIVERY_HISTORY, deliveries=deliveries)


# ==============================================================================
# STARTUP
# ==============================================================================

if __name__ == "__main__":
    init_db()
    connect_arduino()

    scheduler = threading.Thread(target=scheduler_loop, daemon=True)
    scheduler.start()

    app.run(host=FLASK_HOST, port=FLASK_PORT)
