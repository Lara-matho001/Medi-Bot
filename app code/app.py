from flask import Flask, request, redirect, render_template_string
import sqlite3
from datetime import datetime
import threading
import time

app = Flask(__name__)

def get_db():
    return sqlite3.connect("patients.db")

def init_db():
    conn = get_db()
    c = conn.cursor()

    c.execute("""
        CREATE TABLE IF NOT EXISTS patients (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT,
            room TEXT,
            medication TEXT
        )
    """)

    c.execute("""
        CREATE TABLE IF NOT EXISTS schedules (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            patient_id INTEGER,
            patient_name TEXT,
            room TEXT,
            medication TEXT,
            delivery_time TEXT,
            status TEXT
        )
    """)

    for column in ["patient_name", "room", "medication"]:
    try:
        c.execute(f"ALTER TABLE schedules ADD COLUMN {column} TEXT")
    except sqlite3.OperationalError:
        pass

    c.execute("SELECT COUNT(*) FROM patients")
    if c.fetchone()[0] == 0:
        c.execute("INSERT INTO patients (name, room, medication) VALUES ('John', '101', 'Paracetamol')")
        c.execute("INSERT INTO patients (name, room, medication) VALUES ('Grace', '102', 'Aspirin')")
        c.execute("INSERT INTO patients (name, room, medication) VALUES ('Tom', '103', 'Ibuprofen')")

    conn.commit()
    conn.close()

import serial
arduino = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
time.sleep(2)

def send_command_to_robot(room):
    arduino.write(b"LED_ON\n")

def scheduler_loop():
    while True:
        now = datetime.now().strftime("%H:%M")

        conn = get_db()
        c = conn.cursor()

        c.execute("""
            SELECT schedules.id, patients.room
            FROM schedules
            JOIN patients ON schedules.patient_id = patients.id
            WHERE schedules.delivery_time = ? AND schedules.status = 'Pending'
        """, (now,))

        jobs = c.fetchall()

        for job_id, room in jobs:
            send_command_to_robot(room)
            c.execute("UPDATE schedules SET status = 'Sent' WHERE id = ?", (job_id,))

        conn.commit()
        conn.close()

        time.sleep(10)

STYLE = """
<style>
    * {
        box-sizing: border-box;
    }

    body {
        font-family: Arial, sans-serif;
        background: linear-gradient(135deg, #081a33, #123c69);
        color: white;
        padding: 60px 90px;
        font-size: 24px;
        margin: 0;
    }

    h1 {
        font-size: 56px;
        margin-bottom: 20px;
    }

    h2 {
        font-size: 36px;
        margin-top: 0;
    }

    .subtitle {
        font-size: 24px;
        color: #d7e8ff;
        margin-bottom: 40px;
    }

    .box {
        background: #f2f6fb;
        color: #111;
        padding: 40px;
        border-radius: 24px;
        max-width: 950px;
        margin-bottom: 35px;
        box-shadow: 0 8px 25px rgba(0,0,0,0.25);
    }

    .home-grid {
        display: flex;
        gap: 30px;
        margin-top: 30px;
        flex-wrap: wrap;
    }

    .home-card {
        background: white;
        color: #111;
        width: 420px;
        min-height: 220px;
        padding: 35px;
        border-radius: 24px;
        box-shadow: 0 8px 25px rgba(0,0,0,0.25);
    }

    .home-card h2 {
        font-size: 34px;
        margin-bottom: 15px;
        color: #0b1f3a;
    }

    .home-card p {
        font-size: 22px;
        color: #444;
        line-height: 1.4;
    }

    .main-button {
        display: block;
        background: #0b1f3a;
        color: white;
        text-align: center;
        padding: 18px;
        border-radius: 14px;
        margin-top: 25px;
        font-size: 24px;
        font-weight: bold;
        text-decoration: none;
    }

    input, select, button {
        width: 100%;
        padding: 22px;
        margin-top: 15px;
        margin-bottom: 20px;
        font-size: 24px;
        border-radius: 14px;
        border: 2px solid #b8c4d1;
    }

    button {
        background: #0b1f3a;
        color: white;
        border: none;
        font-weight: bold;
        cursor: pointer;
    }

    label {
        font-size: 24px;
        font-weight: bold;
    }

    a {
        color: white;
        margin-right: 25px;
        font-size: 24px;
        font-weight: bold;
        text-decoration: none;
    }

    .darklink {
        color: #0b1f3a;
    }

    table {
        background: white;
        color: #111;
        border-collapse: collapse;
        width: 100%;
        max-width: 1200px;
        font-size: 22px;
        border-radius: 18px;
        overflow: hidden;
        box-shadow: 0 8px 25px rgba(0,0,0,0.25);
    }

    th {
        background: #0b1f3a;
        color: white;
        font-size: 24px;
    }

    th, td {
        padding: 22px;
        border: 1px solid #ccc;
        text-align: left;
    }
</style>
"""

@app.route("/")
def home():
    html = """
    <html>
    <head>
        <title>Medication Robot System</title>
        """ + STYLE + """
    </head>
    <body>
        <h1>Medication Robot System</h1>
        <p class="subtitle">Hospital medication delivery dashboard for scheduling and patient management</p>

        <div class="home-grid">

            <div class="home-card">
                <h2>Schedule Delivery</h2>
                <p>Select a patient, set the medication delivery time, and send the task to the robot automatically.</p>
                <a class="main-button" href="/schedule_page">Open Scheduler</a>
            </div>

            <div class="home-card">
                <h2>Manage Patients</h2>
                <p>Add, edit, or remove patient details including room number and medication type.</p>
                <a class="main-button" href="/patients">Open Patients</a>
            </div>

            <div class="home-card">
                <h2>Delivery History</h2>
                <p>View all scheduled deliveries including patient, room, medication, time, and status.</p>
                <a class="main-button" href="/delivery_history">View History</a>
            </div>

        </div>
    </body>
    </html>
    """
    return html

@app.route("/schedule_page")
def schedule_page():
    conn = get_db()
    c = conn.cursor()
    c.execute("SELECT id, name, room, medication FROM patients")
    patients = c.fetchall()
    conn.close()

    html = """
    <html>
    <head>
        <title>Schedule Delivery</title>
        """ + STYLE + """
    </head>
    <body>
        <h1>Schedule Medication Delivery</h1>
        <a href="/">Home</a>
        <a href="/patients">Manage Patients</a>

        <div class="box">
            <form method="POST" action="/schedule">
                <label>Select Patient:</label>
                <select name="patient_id">
                    {% for p in patients %}
                    <option value="{{p[0]}}">
                        {{p[1]}} - Room {{p[2]}} - {{p[3]}}
                    </option>
                    {% endfor %}
                </select>

                <label>Delivery Time:</label>
                <input type="time" name="delivery_time" required>

                <button type="submit">Schedule Delivery</button>
            </form>
        </div>
    </body>
    </html>
    """

    return render_template_string(html, patients=patients)

@app.route("/schedule", methods=["POST"])
def schedule():
    patient_id = request.form["patient_id"]
    delivery_time = request.form["delivery_time"]

    conn = get_db()
    c = conn.cursor()

    c.execute("SELECT name, room, medication FROM patients WHERE id = ?", (patient_id,))
    patient = c.fetchone()

    patient_name = patient[0]
    room = patient[1]
    medication = patient[2]

    c.execute(
        """INSERT INTO schedules 
        (patient_id, patient_name, room, medication, delivery_time, status) 
        VALUES (?, ?, ?, ?, ?, ?)""",
        (patient_id, patient_name, room, medication, delivery_time, "Pending")
    )

    conn.commit()
    conn.close()

    return f"""
    <html>
    <head>{STYLE}</head>
    <body>
        <h2>Delivery scheduled for {patient_name}</h2>
        <p>Room: {room}</p>
        <p>Medication: {medication}</p>
        <p>Time: {delivery_time}</p>
        <a href="/schedule_page">Back to Schedule</a>
        <a href="/delivery_history">Delivery History</a>
        <a href="/">Home</a>
    </body>
    </html>
    """

@app.route("/patients")
def patients():
    conn = get_db()
    c = conn.cursor()
    c.execute("SELECT id, name, room, medication FROM patients")
    patients = c.fetchall()
    conn.close()

    html = """
    <html>
    <head>
        <title>Manage Patients</title>
        """ + STYLE + """
    </head>
    <body>
        <h1>Manage Patients</h1>
        <a href="/">Home</a>
        <a href="/schedule_page">Schedule Delivery</a>

        <div class="box">
            <h2>Add New Patient</h2>
            <form method="POST" action="/add_patient">
                <input name="name" placeholder="Patient name" required>
                <input name="room" placeholder="Room number" required>
                <input name="medication" placeholder="Medication type" required>
                <button type="submit">Add Patient</button>
            </form>
        </div>

        <h2>Patient Database</h2>
        <table>
            <tr>
                <th>Name</th>
                <th>Room</th>
                <th>Medication</th>
                <th>Actions</th>
            </tr>
            {% for p in patients %}
            <tr>
                <td>{{p[1]}}</td>
                <td>{{p[2]}}</td>
                <td>{{p[3]}}</td>
                <td>
                    <a class="darklink" href="/edit_patient/{{p[0]}}">Edit</a>
                    <a class="darklink" href="/delete_patient/{{p[0]}}">Delete</a>
                </td>
            </tr>
            {% endfor %}
        </table>
    </body>
    </html>
    """

    return render_template_string(html, patients=patients)

@app.route("/add_patient", methods=["POST"])
def add_patient():
    name = request.form["name"]
    room = request.form["room"]
    medication = request.form["medication"]

    conn = get_db()
    c = conn.cursor()
    c.execute(
        "INSERT INTO patients (name, room, medication) VALUES (?, ?, ?)",
        (name, room, medication)
    )
    conn.commit()
    conn.close()

    return redirect("/patients")

@app.route("/edit_patient/<int:patient_id>")
def edit_patient(patient_id):
    conn = get_db()
    c = conn.cursor()
    c.execute("SELECT id, name, room, medication FROM patients WHERE id = ?", (patient_id,))
    patient = c.fetchone()
    conn.close()

    html = """
    <html>
    <head>
        <title>Edit Patient</title>
        """ + STYLE + """
    </head>
    <body>
        <h1>Edit Patient</h1>

        <div class="box">
            <form method="POST" action="/update_patient/{{patient[0]}}">
                <label>Name:</label>
                <input name="name" value="{{patient[1]}}" required>

                <label>Room:</label>
                <input name="room" value="{{patient[2]}}" required>

                <label>Medication:</label>
                <input name="medication" value="{{patient[3]}}" required>

                <button type="submit">Update Patient</button>
            </form>
        </div>

        <a href="/patients">Back</a>
    </body>
    </html>
    """

    return render_template_string(html, patient=patient)

@app.route("/update_patient/<int:patient_id>", methods=["POST"])
def update_patient(patient_id):
    name = request.form["name"]
    room = request.form["room"]
    medication = request.form["medication"]

    conn = get_db()
    c = conn.cursor()
    c.execute(
        "UPDATE patients SET name = ?, room = ?, medication = ? WHERE id = ?",
        (name, room, medication, patient_id)
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
        SELECT patient_name, room, medication, delivery_time, status
        FROM schedules
        ORDER BY id DESC
    """)

    deliveries = c.fetchall()
    conn.close()

    html = """
    <html>
    <head>
        <title>Delivery History</title>
        """ + STYLE + """
    </head>
    <body>
        <h1>Delivery History</h1>

        <a href="/">Home</a>
        <a href="/schedule_page">Schedule Delivery</a>
        <a href="/patients">Manage Patients</a>

        <h2>All Scheduled Deliveries</h2>

        <table>
            <tr>
                <th>Patient</th>
                <th>Room</th>
                <th>Pills Dispensed</th>
                <th>Delivery Time</th>
                <th>Status</th>
            </tr>

            {% for d in deliveries %}
            <tr>
                <td>{{d[0]}}</td>
                <td>{{d[1]}}</td>
                <td>{{d[2]}}</td>
                <td>{{d[3]}}</td>
                <td>{{d[4]}}</td>
            </tr>
            {% endfor %}
        </table>
    </body>
    </html>
    """

    return render_template_string(html, deliveries=deliveries)

if __name__ == "__main__":
    init_db()

    thread = threading.Thread(target=scheduler_loop, daemon=True)
    thread.start()

    app.run(host="0.0.0.0", port=5000)