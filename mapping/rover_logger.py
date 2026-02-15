import json
import time
import random

# CONFIGURATION
LOG_FILE = "mission_log.jsonl"

def get_rich_sensor_data(lat, lon):
    """
    Simulates a complex environment with 6 different sensors.
    """
    # 1. Random Walk
    lat += random.choice([-0.0001, 0, 0.0001])
    lon += random.choice([-0.0001, 0, 0.0001])
    
    # 2. Base Environmental Values
    temp = 24.0
    humidity = 60.0
    light = 800 # Bright room
    gas = random.randint(0, 50) # Clean air
    flame = 1 # 1 means SAFE (High), 0 means FIRE (Low) on most sensors
    camera_obj = "none"

    # 3. Simulate "Events" (The interesting stuff)
    event_roll = random.random()
    
    if event_roll < 0.05: 
        # SCENARIO: FIRE DETECTED!
        flame = 0 # Sensor triggers
        temp = 85.0 # Heat spike
        camera_obj = "danger"
        light = 1000 # Fire is bright
        
    elif event_roll < 0.10:
        # SCENARIO: GAS LEAK
        gas = 400
        camera_obj = "danger"
        
    elif event_roll < 0.20:
        # SCENARIO: FOUND RESOURCES (Water/Gold)
        camera_obj = "resource"
        
    elif event_roll < 0.30:
        # SCENARIO: FOUND OBSTACLE
        camera_obj = "object"
        light = 200 # Shadow of the object?
        
    return lat, lon, {
        "temp": round(temp, 1),
        "humidity": round(humidity, 1),
        "light": light,
        "gas": gas,
        "flame": flame,
        "camera": camera_obj
    }

if __name__ == "__main__":
    open(LOG_FILE, 'w').close() # Reset log
    print(f"🤖 Rich Sensor Logger Active.")
    print("Simulating: Gas, Temp, Hum, Light, Flame, AI Camera...")
    
    lat, lon = 43.6629, -79.3957

    try:
        while True:
            lat, lon, sensors = get_rich_sensor_data(lat, lon)
            
            entry = {
                "timestamp": time.time(),
                "lat": lat, "lon": lon,
                "sensors": sensors
            }
            
            with open(LOG_FILE, 'a') as f:
                f.write(json.dumps(entry) + "\n")
                
            print(f"📝 Logged: {sensors['camera'].upper()} | Gas: {sensors['gas']} | Flame: {sensors['flame']}")
            time.sleep(1) 

    except KeyboardInterrupt:
        print("Stopped.")