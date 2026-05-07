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
            delivery_time TEXT,
            status TEXT
        )
    """)

    c.execute("SELECT COUNT(*) FROM patients")
    if c.fetchone()[0] == 0:
        c.execute("INSERT INTO patients (name, room, medication) VALUES ('John', '101', 'Paracetamol')")
        c.execute("INSERT INTO patients (name, room, medication) VALUES ('Tom', '102', 'Aspirin')")
        c.execute("INSERT INTO patients (name, room, medication) VALUES ('Rose', '103', 'Ibuprofen')")

    conn.commit()
    conn.close()

def send_command_to_robot(room):
    print(f"COMMAND SENT TO ARDUINO: GO_TO_ROOM_{room}")

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
    body {
        font-family: Arial;
        background: #0b1f3a;
        color: white;
        padding: 30px;
    }
    .box {
        background: #d9d9d9;
        color: #111;
        padding: 20px;
        border-radius: 12px;
        max-width: 550px;
        margin-bottom: 20px;
    }
    input, select, button {
        width: 100%;
        padding: 10px;
        margin-top: 10px;
        box-sizing: border-box;
    }
    button {
        background: #0b1f3a;
        color: white;
        border: none;
        border-radius: 8px;
        cursor: pointer;
    }
    a {
        color: #ffffff;
        margin-right: 15px;
    }
    .darklink {
        color: #0b1f3a;
    }
    table {
        background: #d9d9d9;
        color: #111;
        border-collapse: collapse;
        width: 100%;
        max-width: 750px;
    }
    th, td {
        padding: 10px;
        border: 1px solid #999;
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

        <div class="box">
            <h2>Home</h2>
            <a class="darklink" href="/schedule_page">Schedule Delivery</a><br><br>
            <a class="darklink" href="/patients">Manage Patients</a>
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
    c.execute(
        "INSERT INTO schedules (patient_id, delivery_time, status) VALUES (?, ?, ?)",
        (patient_id, delivery_time, "Pending")
    )
    conn.commit()
    conn.close()

    return f"""
    <html>
    <head>{STYLE}</head>
    <body>
        <h2>Delivery scheduled for {delivery_time}</h2>
        <a href="/schedule_page">Back to Schedule</a>
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

if __name__ == "__main__":
    init_db()

    thread = threading.Thread(target=scheduler_loop, daemon=True)
    thread.start()

    app.run(host="0.0.0.0", port=5000)