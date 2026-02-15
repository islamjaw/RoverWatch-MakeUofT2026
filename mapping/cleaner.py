import json
import time
import os

RAW_LOG_FILE = "mission_log.jsonl"
CLEAN_MAP_FILE = "clean_map.json"
GRID_STEP = 0.0001 

def determine_status(sensors):
    """
    Decides the map color based on sensor hierarchy.
    Returns: 1=Safe, 2=Danger, 3=Resource, 4=Object
    """
    # PRIORITY 1: SAFETY (Red)
    if sensors['flame'] == 0: return 2 # Fire!
    if sensors['gas'] > 200: return 2  # Gas!
    if sensors['camera'] == "danger": return 2
    
    # PRIORITY 2: VALUE (Blue)
    if sensors['camera'] == "resource": return 3
    if sensors['camera'] == "food": return 3

    # PRIORITY 3: OBSTACLE (Yellow)
    if sensors['camera'] == "object": return 4
    
    # PRIORITY 4: DEFAULT (Green)
    return 1

def clean_data():
    if not os.path.exists(RAW_LOG_FILE): return

    collapsed_map = {}
    
    try:
        with open(RAW_LOG_FILE, 'r') as f:
            for line in f:
                try:
                    entry = json.loads(line)
                    
                    # Snap to Grid
                    lat_idx = round(entry['lat'] / GRID_STEP)
                    lon_idx = round(entry['lon'] / GRID_STEP)
                    grid_key = f"{lat_idx}_{lon_idx}"
                    
                    # Get Status Code
                    status_code = determine_status(entry['sensors'])
                    
                    # Store Logic: DANGER overrides everything else
                    # If this cell was ever marked DANGER (2), keep it DANGER.
                    existing = collapsed_map.get(grid_key)
                    if existing and existing['status'] == 2:
                        continue # Don't overwrite danger with safety
                        
                    collapsed_map[grid_key] = {
                        "status": status_code,
                        "lat": entry['lat'],
                        "lon": entry['lon'],
                        # Store extra details for the hover tooltip later
                        "details": f"T:{entry['sensors']['temp']}C | Cam:{entry['sensors']['camera']}"
                    }
                    
                    collapsed_map["ROVER_LATEST"] = {"lat": entry['lat'], "lon": entry['lon']}
                    
                except: continue
        
        with open(CLEAN_MAP_FILE, 'w') as f:
            json.dump(collapsed_map, f, indent=2)
            
        print(f"🧹 Map Updated: {len(collapsed_map)} cells.")

    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    print("🧹 Multi-Sensor Cleaner Active.")
    while True:
        clean_data()
        time.sleep(2)