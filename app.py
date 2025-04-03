from flask import Flask, request, jsonify, render_template
import json

app = Flask(__name__)
sensor_data = {}  # Store the latest sensor data

@app.route('/')
def index():
    return render_template('index.html', data=sensor_data)

@app.route('/data', methods=['POST'])
def receive_data():
    global sensor_data
    try:
        sensor_data = request.get_json()
        print("Received data:", sensor_data)
        return jsonify({"status": "success", "message": "Data received"}), 200
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 400

@app.route('/data', methods=['GET'])
def get_data():
    return jsonify(sensor_data)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5001, debug=True)
