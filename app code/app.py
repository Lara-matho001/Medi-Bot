from flask import Flask, request, render_template_string
import sqlite3
from datetime import datetime
import threading
import time

app = Flask(__name__)

def init_db():
    conn = sqlite3.connect("patients.db")
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

    # Premade database examples
    c.execute("SELECT COUNT(*) FROM patients")
    if c.fetchone()[0] == 0:
        c.execute("INSERT INTO patients (name, room, medication) VALUES ('John', '101', 'Paracetamol')")
        c.execute("INSERT INTO patients (name, room, medication) VALUES ('Rose', '102', 'Aspirin')")
        c.execute("INSERT INTO patients (name, room, medication) VALUES ('Tom', '103', 'Ibuprofen')")

    conn.commit()
    conn.close()

def send_command_to_robot(room):
    # Later this can be replaced with Arduino serial command
    print(f"COMMAND SENT TO ARDUINO: GO_TO_ROOM_{room}")

def scheduler_loop():
    while True:
        now = datetime.now().strftime("%H:%M")

        conn = sqlite3.connect("patients.db")
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

@app.route("/")
def home():
    conn = sqlite3.connect("patients.db")
    c = conn.cursor()
    c.execute("SELECT id, name, room, medication FROM patients")
    patients = c.fetchall()
    conn.close()

    html = """
    <html>
    <head>
        <title>Medication Robot Scheduler</title>
        <style>
            body { font-family: Arial; background:#0b1f3a; color:white; padding:30px; }
            .box { background:#d9d9d9; color:#111; padding:20px; border-radius:12px; max-width:450px; }
            select, input, button { width:100%; padding:10px; margin-top:10px; }
            button { background:#0b1f3a; color:white; border:none; border-radius:8px; }
        </style>
    </head>
    <body>
        <h1>Medication Robot Scheduler</h1>

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

    conn = sqlite3.connect("patients.db")
    c = conn.cursor()
    c.execute(
        "INSERT INTO schedules (patient_id, delivery_time, status) VALUES (?, ?, ?)",
        (patient_id, delivery_time, "Pending")
    )
    conn.commit()
    conn.close()

    return f"Delivery scheduled for {delivery_time}. <br><br><a href='/'>Back</a>"

if __name__ == "__main__":
    init_db()

    thread = threading.Thread(target=scheduler_loop, daemon=True)
    thread.start()

    app.run(host="0.0.0.0", port=5000)