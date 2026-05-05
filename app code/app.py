from flask import Flask, request

app = Flask(__name__)

deliveries = []

@app.route("/", methods=["GET", "POST"])
def home():
        if request.method == "POST":
                patient = request.form.get("patient")
                room = request.form.get("room")
                time = request.form.get("time")

                deliveries.append({
                    "patient": patient,
                    "room": room,
                    "time": time,
                    "status": "Scheduled"
                })

        html = """
        <html>
        <head>
                <title>Medication Robot App</title>
        </head>
        <body style="font-family: Arial; padding: 20px;">
                <h2>Medication Robot App</h2>

                <form method="POST">
                        <input name="patient" placeholder="Patient name" required><br><br>
                        <input name="room" placeholder="Room number" required><br><br>
                        <input type="time" name="time" required><br><br>
                        <button type="submit">Schedule Delivery</button>
                </form>

                <h3>Delivery Schedule</h3>
        """  

        for d in deliveries:
                html += f"""
                <p>
                <b>Patient:</b> {d['patient']}<br>
                <b>Room:</b> {d['room']}<br>
                <b>Time:</b> {d['time']}<br>
                <b>Status:</b> {d['status']}
                </p>
                <hr>
                """

        html += """
        </body>
        </html>
        """

        return html

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)
