import os
import time
import os
import time
import firebase_admin
from firebase_admin import credentials, db
from flask import Flask, request, jsonify
from flask_cors import CORS
from PIL import Image
import google.generativeai as genai

# 1. --- FIREBASE SETUP ---
# Ensure serviceAccountKey.json is in your /backend folder
try:
    cred = credentials.Certificate("serviceAccountKey.json")
    firebase_admin.initialize_app(cred, {
        'databaseURL': 'https://survivalsense-a48fe-default-rtdb.firebaseio.com'
    })
    print("✅ Firebase Admin SDK Initialized")
except Exception as e:
    print(f"❌ Firebase Init Error: {e}")

# 2. --- GEMINI SETUP ---
api_key = "AIzaSyB1GLC1bL3F7tWKhwNM1ou0KtJFHNm6Otk"
genai.configure(api_key=api_key)
# Using flash-lite for fastest possible autonomous response
model = genai.GenerativeModel('gemini-2.5-flash-lite')

app = Flask(__name__)
CORS(app)
UPLOAD_FOLDER = 'static/uploads'
os.makedirs(UPLOAD_FOLDER, exist_ok=True)

@app.route('/classify', methods=['POST'])
def classify_image():
    if 'image' not in request.files:
        return jsonify({"error": "No image found in request"}), 400

    file = request.files['image']
    try:
        # Save local copy (Phone HUD will pull from this URL)
        image_path = os.path.join(UPLOAD_FOLDER, "latest.jpg")
        file.save(image_path)
        
        # AI Analysis: Focused on Navigation and Safety
        img = Image.open(image_path)
        prompt = (
            "You are an autonomous car navigator. Analyze this image. "
            "Is there a DANGER or OBSTACLE? If so, tell the car to STOP or TURN. "
            "If clear, say SAFE. Keep under 8 words."
        )
        
        response = model.generate_content([prompt, img])
        result_text = response.text.strip()
        
        # Determine Status Color
        # Logic: Red for threats, Green for clear paths
        threat_keywords = ["DANGER", "OBSTACLE", "BEAR", "STOP", "PERSON", "CHILD", "VEHICLE"]
        color = "#00ff00" # Default: SAFE (Green)
        
        if any(word in result_text.upper() for word in threat_keywords):
            color = "#ff0000" # DANGER (Red)

        # 3. --- PUSH TO FIREBASE ---
        # Update the IP to match your current phone hotspot address
        my_ip = "192.168.123.78" 
        full_image_url = f"http://{my_ip}:5000/static/uploads/latest.jpg?t={time.time()}"
        
        ref = db.reference('sensor_data')
        ref.update({
            "image_url": full_image_url,
            "analysis": result_text.upper(),
            "status_color": color,
            "timestamp": time.time()
        })

        print(f"📡 AI DATA PUSHED: {result_text} | COLOR: {color}")
        return jsonify({"status": "Success", "analysis": result_text})

    except Exception as e:
        print(f"❌ Error during classification: {e}")
        return jsonify({"error": str(e)}), 500

if __name__ == '__main__':
    # host='0.0.0.0' allows devices on your hotspot to see this server
    app.run(host='0.0.0.0', port=5000, debug=False)