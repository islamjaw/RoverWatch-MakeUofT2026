import serial
import firebase_admin
from firebase_admin import credentials, db

# --- FIREBASE SETUP ---
cred = credentials.Certificate("serviceAccountKey.json")
firebase_admin.initialize_app(cred, {'databaseURL': 'https://survivalsense-a48fe-default-rtdb.firebaseio.com'})

# --- SERIAL SETUP ---
# Change 'COM3' to the port your Arduino is plugged into
ser = serial.Serial('COM5', 9600) 

while True:
    line = ser.readline().decode('utf-8').strip()
    if line.startswith("COORD:"):
        # Parse X, Y
        _, coords = line.split(":")
        x, y = coords.split(",")
        
        # Convert Rover CM to "Map Steps"
        lat = 43.6628 + (float(x) * 0.00001)
        lon = -79.3958 + (float(y) * 0.00001)

        # Update Firebase so the Web Map moves
        db.reference('tactical_map/ROVER_LATEST').set({"lat": lat, "lon": lon})
        print(f"Uploaded Position: {lat}, {lon}")