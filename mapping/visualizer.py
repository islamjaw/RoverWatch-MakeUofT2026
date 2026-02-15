import json
import matplotlib.pyplot as plt
from matplotlib import colors
import time
import os

CLEAN_MAP_FILE = "clean_map.json"
GRID_STEP = 0.0001 

def load_clean_map():
    if not os.path.exists(CLEAN_MAP_FILE): return {}, 43.6629, -79.3957
    try:
        with open(CLEAN_MAP_FILE, 'r') as f:
            data = json.load(f)
            rover = data.pop("ROVER_LATEST", {"lat": 43.6629, "lon": -79.3957})
            return data, rover['lat'], rover['lon']
    except: return {}, 43.6629, -79.3957

def build_grid(clean_data, current_lat, current_lon):
    grid = [[0 for _ in range(20)] for _ in range(20)]
    center_r = round(current_lat / GRID_STEP)
    center_c = round(current_lon / GRID_STEP)

    for row in range(20):
        for col in range(20):
            target_r = center_r + (10 - row)
            target_c = center_c + (col - 10)
            key = f"{target_r}_{target_c}"
            
            if key in clean_data:
                # 1=Safe, 2=Danger, 3=Resource, 4=Object
                grid[row][col] = clean_data[key]['status']
            else:
                grid[row][col] = 0 # Unknown
    return grid

if __name__ == "__main__":
    plt.ion()
    fig, ax = plt.subplots(figsize=(8, 8))
    
    # DEFINING THE NEW PALETTE
    # 0=Grey, 1=Green, 2=Red, 3=Blue, 4=Yellow
    cmap = colors.ListedColormap(['#EEEEEE', '#00CC00', '#FF0000', '#0000FF', '#FFD700'])
    bounds = [0, 1, 2, 3, 4, 5]
    norm = colors.BoundaryNorm(bounds, cmap.N)

    print("📺 Multi-Spectral Visualizer Active...")

    while True:
        map_data, lat, lon = load_clean_map()
        matrix = build_grid(map_data, lat, lon)
        
        ax.clear()
        ax.imshow(matrix, cmap=cmap, norm=norm)
        
        # Grid settings
        ax.set_xticks([]); ax.set_yticks([])
        ax.grid(which='major', color='black', linewidth=0.5)
        
        # Legend
        ax.set_title(f"TACTICAL MAP\nRed=Danger | Blue=Resource | Yellow=Object")
        ax.text(9.7, 9.7, "🤖", fontsize=20)
        
        plt.draw()
        plt.pause(1)