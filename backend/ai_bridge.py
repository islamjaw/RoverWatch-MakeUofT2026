import firebase_admin
from firebase_admin import credentials, db
import google.generativeai as genai
from PIL import Image
from io import BytesIO

# --- FIREBASE & GEMINI SETUP ---
cred = credentials.Certificate("backend/serviceAccountKey.json")
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://survivalsense-a48fe-default-rtdb.firebaseio.com/'
})

genai.configure(api_key="YOUR_GEMINI_API_KEY")
model = genai.GenerativeModel('gemini-1.5-flash')

def hex_to_gemini(event):
    data = event.data
    if not data: return

    try:
        # Get the latest entry from the 'imageData' node
        latest_key = max(data.keys())
        hex_data = data[latest_key].get('hex')
        
        if hex_data:
            print(f"New Image Received ({latest_key}). Converting HEX...")
            
            # Convert Hex String back to raw bytes
            img_bytes = bytes.fromhex(hex_data)
            img = Image.open(BytesIO(img_bytes))
            
            # Send to Gemini
            response = model.generate_content([
                "Analyze this 96x96 rover feed. Is there a survival threat? Answer in 3 words.",
                img
            ])
            
            result = response.text.strip()
            print(f"Gemini Result: {result}")

            # Update results for your phone to see
            db.reference('/mission_control').update({
                "ai_result": result,
                "timestamp": latest_key
            })

    except Exception as e:
        print(f"Conversion Error: {e}")

print("Brain Active: Waiting for HEX images...")
db.reference('/imageData').listen(hex_to_gemini)