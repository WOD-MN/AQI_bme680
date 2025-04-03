# 🌡️✨ **Air Quality Monitoring System** ✨🌫️  

*A smart IoT project using **ESP32 & BME680/BME688** to monitor indoor air quality (IAQ) in real-time!*  
![Uploading Screenshot 2025-04-03 at 10.00.18 PM.png…]()

🚀 **Deploy your own air quality station** that measures:  
- **Temperature** 🌡️  
- **Humidity** 💧  
- **Air Pressure** 🌀  
- **IAQ (Indoor Air Quality)** 🌬️  
- **CO₂ Levels** ☁️  
- **VOC (Volatile Organic Compounds)** 🧪  

📊 **Data is sent to a server** for logging and visualization!  

---

## 📌 **Table of Contents**  
1. [Features](#-features)  
2. [Hardware Requirements](#-hardware-requirements)  
3. [Software Setup](#-software-setup)  
4. [Calibration Guide](#-calibration-guide)  
5. [Server Setup](#-server-setup)  
6. [Troubleshooting](#-troubleshooting)  
7. [Future Enhancements](#-future-enhancements)  
8. [Contributing](#-contributing)  

---

## 🎯 **Features**  
✅ **Real-time sensor readings** (updated every 30s)  
✅ **Wi-Fi connectivity** (auto-reconnect if dropped)  
✅ **Data smoothing** (5-point moving average for accuracy)  
✅ **Calibration tracking** (auto-calibrates over time)  
✅ **JSON API integration** (sends data to a web server)  
✅ **LED status indicator** (visual feedback)  

📈 **Perfect for:**  
- Smart homes 🏠  
- Offices 🏢  
- Schools 🏫  
- Health monitoring 🩺  

---

## 🔧 **Hardware Requirements**  
| Component | Quantity | Notes |
|-----------|----------|-------|
| **ESP32 Dev Board** | 1 | Wi-Fi & Bluetooth enabled |
| **BME680/BME688 Sensor** | 1 | IAQ, Temp, Humidity, Pressure, VOC |
| **Breadboard & Jumper Wires** | As needed | For connections |
| **Micro-USB Cable** | 1 | Power & Programming |
| **LED (Optional)** | 1 | Visual status indicator |

🔌 **Wiring Guide**  
| ESP32 Pin | BME680 Pin |
|-----------|-----------|
| **3.3V**  | VCC       |
| **GND**   | GND       |
| **GPIO 21 (SDA)** | SDA       |
| **GPIO 22 (SCL)** | SCL       |

---

## 💻 **Software Setup**  
### **1. Install Required Libraries**  
📚 **Libraries Needed:**  
- **BSEC Library** (for BME680/BME688)  
- **WiFi & HTTPClient** (for ESP32 Wi-Fi)  
- **ArduinoJSON** (for JSON payloads)  

🔹 **Install via Arduino Library Manager:**  
1. Open **Arduino IDE** → **Sketch** → **Include Library** → **Manage Libraries**  
2. Search & Install:  
   - `BSEC by Bosch Sensortec`  
   - `ArduinoJSON by Benoit Blanchon`  

### **2. Upload the Code**  
📥 **Steps:**  
1. Clone/download this repository.  
2. Open `air_quality_monitor.ino` in Arduino IDE.  
3. Update Wi-Fi credentials in the code:  
   ```cpp
   const char *ssid = "YOUR_WIFI_SSID";
   const char *password = "YOUR_WIFI_PASSWORD";
   ```
4. Set your server URL:  
   ```cpp
   const String serverURL = "http://YOUR_SERVER_IP:PORT/data";
   ```
5. **Upload to ESP32!**  

---

## 🔄 **Calibration Guide**  
🔄 **The BME680/BME688 needs calibration for accurate IAQ readings.**  

📅 **Calibration Time:**  
- **Initial Calibration:** ~5 days (120 hours)  
- **After power cycles:** ~1-2 hours (re-stabilization)  

📊 **How to Check Calibration Status?**  
- The ESP32 will print calibration logs via **Serial Monitor**.  
- `iaqAccuracy` values:  
  - **0** → Uncalibrated (wait)  
  - **1** → Low accuracy (~3-4 days left)  
  - **2** → Medium accuracy (~24h left)  
  - **3** → Fully calibrated! ✅  

💡 **Tips for Faster Calibration:**  
- Keep the sensor in a **well-ventilated** area.  
- Avoid extreme temperature changes.  
- Let it run **continuously** (no frequent power-offs).  

---

## 🌐 **Server Setup**  
📡 **To receive & store sensor data:**  

### **Option 1: Local Python Server (Fast & Easy)**  
```python
from flask import Flask, request, jsonify

app = Flask(__name__)

@app.route('/data', methods=['POST'])
def handle_data():
    data = request.json
    print("Received:", data)
    return jsonify({"status": "success"})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5001)
```
▶️ **Run:** `python server.py`  

### **Option 2: Database Integration (MySQL, Firebase, InfluxDB)**  
🔹 **Example Firebase (Firestore) Code:**  
```javascript
const functions = require('firebase-functions');
const admin = require('firebase-admin');
admin.initializeApp();

exports.logSensorData = functions.https.onRequest((req, res) => {
    const data = req.body;
    admin.firestore().collection('sensor_data').add(data);
    res.status(200).send("Data logged!");
});
```

---

## 🛠 **Troubleshooting**  
🔴 **Problem:** Sensor not detected  
✅ **Fix:** Check I2C connections (SDA/SCL)  

🔴 **Problem:** Wi-Fi not connecting  
✅ **Fix:** Verify SSID/password, check router settings  

🔴 **Problem:** Data not sending to server  
✅ **Fix:** Ensure server is running & URL is correct  

🔴 **Problem:** `iaqAccuracy` stuck at 0  
✅ **Fix:** Wait longer (calibration takes days!)  

---

## 🚀 **Future Enhancements**  
📅 **Planned Features:**  
- **OLED Display** (real-time readings) 📺  
- **MQTT Support** (for IoT platforms) 🌍  
- **Battery Power** (portable mode) 🔋  
- **Mobile App Alerts** (if air quality drops) 📱  

---

## 🤝 **Contributing**  
💡 **Want to improve this project?**  
1. **Fork the repo**  
2. **Create a new branch** (`git checkout -b feature/new-feature`)  
3. **Commit changes** (`git commit -m "Added cool feature"`)  
4. **Push to branch** (`git push origin feature/new-feature`)  
5. **Open a Pull Request**  

---

## 📜 **License**  
**MIT License** - Free to use, modify & distribute!  

---

## 🌟 **Like this project? Give it a ⭐ on GitHub!**  
🔗 **[GitHub Repo Link](#)**  

---

### 🎉 **Happy Building! Let’s make the air cleaner!** 🎉  
💨 **Breathe easy with real-time air quality insights!** 💨
