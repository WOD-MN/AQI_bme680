from flask import Flask, request, jsonify, render_template, send_file
from io import BytesIO, StringIO
import pandas as pd
from datetime import datetime
import json
import os

app = Flask(__name__)

# In-memory data storage
sensor_data = []
DATA_FILE = 'sensor_data.json'

def load_data():
    global sensor_data
    if os.path.exists(DATA_FILE):
        with open(DATA_FILE, 'r') as f:
            try:
                sensor_data = json.load(f)
            except json.JSONDecodeError:
                sensor_data = [] # Handle empty or malformed JSON

def save_data():
    with open(DATA_FILE, 'w') as f:
        json.dump(sensor_data, f, indent=4)

# Load existing data on startup
load_data()

@app.route('/data', methods=['POST'])
def receive_data():
    try:
        # Get JSON data from Arduino
        data = request.get_json()
        
        if not data:
            return jsonify({"status": "error", "message": "No data received"}), 400
            
        # Add timestamp to the data
        data['timestamp'] = datetime.now().isoformat()
        
        # Add to storage
        sensor_data.append(data)
        
        # Keep only the last 1000 entries to prevent memory issues
        if len(sensor_data) > 1000:
            sensor_data.pop(0)
            
        save_data()
        return jsonify({"status": "success"}), 200
        
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/api/history')
def get_history():
    return jsonify(sensor_data)

@app.route('/api/latest')
def get_latest_data():
    if sensor_data:
        return jsonify(sensor_data[-1])
    return jsonify({})

@app.route('/api/export/<file_format>')
def export_data(file_format):
    try:
        df = pd.DataFrame(sensor_data)
        
        if df.empty:
            return jsonify({"status": "error", "message": "No data to export"}), 404

        # Ensure timestamp is first column
        if 'timestamp' in df.columns:
            cols = ['timestamp'] + [col for col in df.columns if col != 'timestamp']
            df = df[cols]

        if file_format == 'csv':
            output = StringIO()
            df.to_csv(output, index=False)
            output.seek(0)
            return send_file(
                BytesIO(output.getvalue().encode('utf-8')),
                as_attachment=True,
                download_name=f"sensor_data_{datetime.now().strftime('%Y%m%d_%H%M%S')}.csv",
                mimetype='text/csv'
            )
        elif file_format == 'excel':
            output = BytesIO()
            with pd.ExcelWriter(output, engine='xlsxwriter') as writer:
                df.to_excel(writer, sheet_name='Sensor Data', index=False)
                for column in df:
                    column_width = max(df[column].astype(str).map(len).max(), len(column))
                    col_idx = df.columns.get_loc(column)
                    writer.sheets['Sensor Data'].set_column(col_idx, col_idx, column_width)
            output.seek(0)
            return send_file(
                output,
                as_attachment=True,
                download_name=f"sensor_data_{datetime.now().strftime('%Y%m%d_%H%M%S')}.xlsx",
                mimetype='application/vnd.openxmlformats-officedocument.spreadsheetml.sheet'
            )
        else:
            return jsonify({"status": "error", "message": "Unsupported file format"}), 400

    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/')
def dashboard():
    return render_template('dashboard.html')

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5001, debug=True)
