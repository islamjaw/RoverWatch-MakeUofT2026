<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>RoverWatch: SurvivalSense README</title>
    <style>
        body { font-family: 'Inter', -apple-system, sans-serif; line-height: 1.6; color: #333; max-width: 900px; margin: 0 auto; padding: 20px; background-color: #f4f7f6; }
        h1, h2, h3 { color: #2c3e50; border-bottom: 2px solid #00ff00; padding-bottom: 5px; }
        .badge { display: inline-block; padding: 5px 12px; border-radius: 20px; font-size: 0.85rem; font-weight: bold; color: white; margin-right: 5px; }
        .badge-react { background-color: #61dafb; color: #000; }
        .badge-flask { background-color: #444; }
        .badge-firebase { background-color: #f5820d; }
        .badge-gemini { background-color: #8e44ad; }
        code { background: #e8e8e8; padding: 2px 5px; border-radius: 4px; font-family: 'JetBrains Mono', monospace; }
        pre { background: #2d2d2d; color: #ccc; padding: 15px; border-radius: 8px; overflow-x: auto; }
        .section { background: white; padding: 25px; border-radius: 10px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); margin-bottom: 20px; }
        .highlight { color: #cc0000; font-weight: bold; }
    </style>
</head>
<body>

    <h1>🤖 RoverWatch: SurvivalSense</h1>
    <p><strong>Autonomous AI-Powered Tactical Scouting Rover</strong> | <em>MakeUofT 2026</em></p>

    <div style="margin-bottom: 20px;">
        <span class="badge badge-react">React.js</span>
        <span class="badge badge-flask">Flask</span>
        <span class="badge badge-firebase">Firebase</span>
        <span class="badge badge-gemini">Gemini 2.0 Flash</span>
    </div>

    <div class="section">
        <h2>🌠 Inspiration</h2>
        <p>Inspired by the desolate, high-stakes environments of <em>Fallout</em>, RoverWatch was created to navigate areas where civilization has failed. In a world plagued by pollution and unknown hazards, our rover acts as a scout to warn survivors of incoming danger.</p>
    </div>

    <div class="section">
        <h2>🚀 What it does</h2>
        <ul>
            <li><strong>Tactical Navigation:</strong> Uses PID control to drive toward waypoints while autonomously dodging obstacles.</li>
            <li><strong>Environmental Sensing:</strong> Monitors Air Quality (MQ2), Temperature, and Humidity (DHT11).</li>
            <li><strong>AI Vision:</strong> Analyzed by Gemini 2.0 Flash to distinguish between threats (predators, sharp objects) and resources.</li>
            <li><strong>Mission Commander HUD:</strong> A React dashboard with a dynamic 15x9 tactical grid and live AI mission logging.</li>
        </ul>
        
    </div>

    <div class="section">
        <h2>🛠️ How we built it</h2>
        <h3>Hardware Stack</h3>
        <p><strong>Elegoo UNO R3:</strong> Low-level motor logic and inertial tracking via MPU6050 Gyroscope.</p>
        <p><strong>ESP32-CAM:</strong> Vision and sensing bridge, transmitting data via Wi-Fi to our Flask backend.</p>

        <h3>Software Stack</h3>
        <p><strong>Backend:</strong> Flask server coordinating data between ESP32 and Gemini API.</p>
        <p><strong>Database:</strong> Firebase Realtime DB for sub-second HUD synchronization.</p>
        <p><strong>Frontend:</strong> React.js with Vite featuring an immersive "Red-Alert" UI.</p>
    </div>

    <div class="section">
        <h2>⚠️ Challenges we ran into</h2>
        <p>Hardware proved to be unpredictable. We faced broken sensors, finicky jumper wires, and an Arduino UNO Q that stopped being detected mid-development, forcing a 7-hour delay.</p>
    </div>

    <div class="section">
        <h2>📚 What we learned</h2>
        <p>We learned that defining the problem is more critical than the code itself. We gained experience in real-time data synchronization across different software environments and the patience required for hardware debugging.</p>
    </div>

    <div class="section">
        <h2>🛤️ What's next</h2>
        <ul>
            <li><strong>TinyML:</strong> Implementing machine learning directly on the MCU for offline autonomy.</li>
            <li><strong>SLAM Integration:</strong> Advanced 3D terrain reconstruction for higher mapping accuracy.</li>
            <li><strong>Aerial Drone:</strong> Evolving from a ground rover to a drone system.</li>
        </ul>
    </div>

    <div class="section">
        <h2>💻 Installation</h2>
        <pre>
# Run Frontend
cd frontend && npm install && npm run dev

# Run Backend
cd backend && python app.py

# Run Bridge
python bridge.py
        </pre>
    </div>

</body>
</html>
