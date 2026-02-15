import os, time, json, firebase_admin, random
from firebase_admin import credentials, db
from flask import Flask, request, jsonify, send_from_directory
from flask_cors import CORS
from PIL import Image
from google import genai

# --- CONFIGURATION ---
MAP_FILE = "../mapping/clean_map.json"
LOG_FILE = "../mapping/mission_log.jsonl"
GRID_STEP = 0.0001
UPLOAD_FOLDER = 'static/uploads'
os.makedirs(UPLOAD_FOLDER, exist_ok=True)

# Firebase Initialization
cred = credentials.Certificate("serviceAccountKey.json")
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://survivalsense-a48fe-default-rtdb.firebaseio.com'
})

# Gemini AI Initialization
client = genai.Client(api_key="AIzaSyBvnrYRBdHl4deW8qeNFpMDpkE0iDfyTns")
app = Flask(__name__)
CORS(app)

@app.route('/classify', methods=['POST'])
def classify_image():
    print("\n" + "="*60)
    print("🚀 INCOMING IMAGE FROM ESP32-CAM")
    print("="*60)
    
    # 1. Capture ALL real sensor data from ESP32
    lat = float(request.form.get('lat', 43.6628))
    lon = float(request.form.get('lon', -79.3957))
    temp = float(request.form.get('temp', 0.0))
    hum = float(request.form.get('humidity', 0.0))
    gas = int(request.form.get('gas', 0))
    batt = int(request.form.get('battery', 100))
    
    print(f"📍 Position: ({lat:.6f}, {lon:.6f})")
    print(f"🌡️  Temp: {temp}°C | 💧 Humidity: {hum}% | 🌫️  Gas: {gas}")
    
    # Save the captured image
    file = request.files['image']
    timestamp = int(time.time())
    image_filename = f"scan_{timestamp}.jpg"
    image_path = os.path.join(UPLOAD_FOLDER, image_filename)
    file.save(image_path)
    print(f"✅ Image saved: {image_filename}")
    
# 2. REAL AI ANALYSIS OF THE ACTUAL IMAGE
    print("🤖 Analyzing image with Gemini AI...")
    
    try:
        # Overhauled prompt for complex visual hazard detection
        prompt = f"""You are the Tactical Mission Commander for a survival rover.
        
        SENSOR DATA:
        - Ambient Temp: {temp}°C
        - Gas/Air Quality: {gas} ppm
        - Humidity: {hum}%

        CRITICAL MISSION: Analyze the visual feed for survival-critical hazards. 
        PRIORITY HIERARCHY:
        1. PHYSICAL DANGER: Sharp objects (glass, rusted metal), unstable terrain, or traps.
        2. BIOLOGICAL THREATS: Large animals/predators, poisonous plants, or spoiled/toxic food.
        3. ENVIRONMENTAL: Sensor-based threats like toxic gas or extreme heat.

        Respond ONLY with a valid JSON object in this format:
        {{
          "ANALYSIS": "Immediate report of physical/bio hazards first, then sensor interpretation.",
          "THREAT": "Score 0-10 based on immediate physical danger to the rover.",
          "OBJECT_TYPE": "danger (if threat > 5), resource (if food/water found), obstacle (if blocked), safe",
          "MOVE": "NORTH/SOUTH/EAST/WEST/STAY",
          "ACTION": "EVACUATE (if life-threatening), AVOID (if sharp/bio hazard), COLLECT (if resource), INVESTIGATE"
        }}"""

        response = client.models.generate_content(
            model='gemini-2.0-flash', 
            contents=[prompt, Image.open(image_path)]
        )
        
        res_text = response.text.strip()
        print(f"🤖 Raw Gemini response:\n{res_text}")
        
        # Extract JSON from markdown backticks if present
        json_str = res_text
        if "```json" in res_text:
            json_str = res_text.split("```json")[1].split("```")[0]
        elif "```" in res_text:
            json_str = res_text.split("```")[1].split("```")[0]
        elif "{" in res_text:
            json_str = res_text[res_text.find("{"):res_text.rfind("}")+1]
        
        json_data = json.loads(json_str)
        
        # Extract analysis components
        display_analysis = json_data.get("ANALYSIS", res_text)
        threat_level = int(json_data.get("THREAT", 0))
        object_type = json_data.get("OBJECT_TYPE", "safe")
        recommended_move = json_data.get("MOVE", "STAY")
        recommended_action = json_data.get("ACTION", "INVESTIGATE")
        
        print(f"✅ Analysis: {display_analysis}")
        print(f"⚠️  Threat Level: {threat_level}/10")
        # print(f"🎯 Object Type: {object_type}")
        print(f"🧭 Recommended Move: {recommended_move}")
        
    except Exception as e:
        print(f"❌ Gemini API error: {e}")
        display_analysis = "Error analyzing image - using sensor data only"
        threat_level = 5 if gas > 200 else 0
        object_type = "danger" if gas > 200 else "safe"
        recommended_move = "STAY"
        recommended_action = "INVESTIGATE"
    
    # Determine UI color and grid status based on AI analysis
    ui_color = "#ff0000" if threat_level > 7 else "#ffaa00" if threat_level > 4 else "#00ff00"
    
    # Map object type to status code
    status_map = {
        "safe": 1,      # Green
        "danger": 2,    # Red
        "resource": 3,  # Blue
        "obstacle": 4   # Yellow (if you add it)
    }
    rover_status = status_map.get(object_type, 1)
    
    # 3. UPDATE ROVER'S CURRENT POSITION (CENTER OF GRID) WITH REAL DATA
    rover_grid_key = f"{round(lat / GRID_STEP)}_{round(lon / GRID_STEP)}"
    
    db.reference(f'tactical_map/{rover_grid_key}').set({
        "status": rover_status,  # Based on Gemini's analysis
        "lat": lat, 
        "lon": lon,
        "temp": temp,
        "gas": gas,
        "humidity": hum,
        "analysis": display_analysis,  # Gemini's actual analysis
        "timestamp": timestamp,
        "is_rover": True  # Mark as current rover position
    })
    
    print(f"✅ Rover position updated: {rover_grid_key} (Status: {rover_status})")
    
    # 4. GENERATE FAKE SURROUNDING GRID (for demo purposes)
    print("🗺️  Generating tactical grid...")
    
    for r in range(-3, 4):
        for c in range(-3, 4):
            # Skip center (rover's actual position - already updated above)
            if r == 0 and c == 0: 
                continue
            
            sample_lat = lat + (r * GRID_STEP)
            sample_lon = lon + (c * GRID_STEP)
            grid_key = f"{round(sample_lat / GRID_STEP)}_{round(sample_lon / GRID_STEP)}"
            
            # Randomized variety: 60% Safe (1), 30% Danger (2), 10% Resource (3)
            rand_val = random.random()
            fake_status = 1 if rand_val < 0.6 else 2 if rand_val < 0.9 else 3
            
            db.reference(f'tactical_map/{grid_key}').update({
                "status": fake_status,
                "lat": sample_lat, 
                "lon": sample_lon,
                "temp": round(temp + random.uniform(-2, 2), 1),
                "gas": max(0, gas + random.randint(-10, 30)),
                "humidity": round(hum + random.uniform(-5, 5), 1),
                "analysis": f"Simulated scan: {'Clear' if fake_status == 1 else 'Hazard' if fake_status == 2 else 'Resource detected'}",
                "timestamp": timestamp - random.randint(10, 300)  # Older scans
            })
    
    print(f"✅ Generated 48 surrounding cells (7x7 grid)")
    
    # Update rover's latest position marker
    db.reference('tactical_map/ROVER_LATEST').set({"lat": lat, "lon": lon})
    
    # 5. UPDATE MAIN HUD WITH REAL GEMINI ANALYSIS
    db.reference('sensor_data').set({
        "image_url": f"http://192.168.123.78:5000/static/uploads/{image_filename}",
        "analysis": display_analysis,  # Real Gemini analysis
        "threat_level": threat_level,  # Real threat assessment
        "status_color": ui_color,
        "object_type": object_type,
        "recommended_move": recommended_move,
        "recommended_action": recommended_action,
        "temp": temp,
        "gas": gas,
        "humidity": hum,
        "battery": batt,
        "timestamp": timestamp
    })
    
    print("="*60)
    print("✨ ANALYSIS COMPLETE")
    print(f"📊 Threat: {threat_level}/10 | Object: {object_type}")
    print(f"🧭 Recommendation: {recommended_action} - Move {recommended_move}")
    print("="*60 + "\n")
    
    return jsonify({
        "status": "success",
        "analysis": display_analysis,
        "threat_level": threat_level,
        "object_type": object_type
    })

@app.route('/simulate_danger', methods=['POST'])
def simulate_danger():
    """Toggle for demo - simulates extreme danger scenario"""
    timestamp = int(time.time())
    
    print("\n🚨 DANGER SIMULATION ACTIVATED")
    
    db.reference('sensor_data').update({
        "analysis": "🚨 CRITICAL EMERGENCY: Multiple environmental hazards detected across all sectors! Toxic gas levels critical. Immediate evacuation required!",
        "threat_level": 10,
        "status_color": "red",
        "object_type": "danger",
        "recommended_move": "NORTH",
        "recommended_action": "EVACUATE",
        "timestamp": timestamp
    })
    
    return jsonify({"status": "danger_simulated"})

@app.route('/static/uploads/<filename>')
def serve_image(filename):
    return send_from_directory(UPLOAD_FOLDER, filename)

@app.route('/api/status', methods=['GET'])
def get_status():
    """Get current sensor data"""
    ref = db.reference('sensor_data')
    data = ref.get()
    return jsonify(data if data else {"status": "no_data"})

@app.route('/api/map', methods=['GET'])
def get_map():
    """Get tactical map data"""
    ref = db.reference('tactical_map')
    data = ref.get()
    return jsonify(data if data else {})

@app.route('/', methods=['GET'])
def health_check():
    return jsonify({
        "status": "online",
        "service": "RoverWatch AI Mission Commander",
        "endpoints": [
            "/classify - POST image for analysis",
            "/simulate_danger - POST to toggle danger mode",
            "/api/status - GET sensor data",
            "/api/map - GET tactical map",
            "/static/uploads/<filename> - GET image"
        ]
    })

if __name__ == '__main__':
    print("\n" + "="*60)
    print("🧠 ROVERWATCH AI MISSION COMMANDER")
    print("="*60)
    print("🔥 Firebase: Connected")
    print("🤖 Gemini AI: Ready")
    print("🗺️  Tactical Grid: Active")
    print("📡 Listening on: http://0.0.0.0:5000")
    print("="*60 + "\n")
    
    app.run(host='0.0.0.0', port=5000, debug=True)