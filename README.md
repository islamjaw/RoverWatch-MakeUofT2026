# RoverWatch: MakeUofT2026
Autonomous AI-Powered Tactical Scouting Rover | Built for MakeUofT 2026

🌠 Inspiration
When we heard the theme for D-Day, our minds immediately went to the worst: desolate areas with little civilization, plagued by pollution, radiation, and unseen hazards. Inspired by the world of Fallout, we wanted to create something like a Mars Rover—a scout capable of exploring unseen areas and warning its operators of incoming danger before they encounter it.

🚀 What it does
RoverWatch is a tactical scouting system that provides real-time environmental analysis:

Navigation: The rover navigates to specific coordinates, using a gyroscope for heading and an ultrasonic sensor to autonomously avoid obstructions.

Environmental Monitoring: Continuously tracks air quality, temperature, and humidity.

AI Strategy: Uses the Gemini 2.0 Flash API to analyze live camera feeds, distinguish between threats and resources, and recommend the "next best move".

Tactical HUD: A web dashboard that maps sensor data and AI analysis onto a grid for easy visualization.

🛠️ How we built it
Hardware
Elegoo UNO R3: Drives the motors and handles low-level logic, including a PID control loop for straight driving and autonomous obstacle avoidance.

ESP32-CAM: Captures images and environmental data, acting as the wireless bridge to the backend.

Sensors: MPU6050 (Gyroscope), HC-SR04 (Ultrasonic), DHT11 (Temp/Humidity), and MQ2 (Air Quality).

Software Stack
Frontend: Built with React.js and Vite.

Backend: A Flask server (Python) that manages data flow between the hardware and AI.

Database: Firebase Realtime Database for live synchronization between the rover and the dashboard.

AI Intelligence: Gemini 2.0 Flash API provides spatial awareness and threat detection.

⚠️ Challenges we ran into
Hardware Reliability: We faced significant hurdles with finicky jumper wires and broken sensors.

Hardware Failure: Our original Arduino UNO Q stopped being detected mid-development, forcing us to pivot and start over 7 hours late.

Full-Stack Coordination: Simultaneously managing data flow between the ESP32, Flask, Firebase, and the React frontend was a complex synchronization task.

🏆 Accomplishments we're proud of
Vision Integration: Successfully getting a rover with "vision" that can distinguish threats using AI.

Full-Stack Mastery: Building a complete end-to-end system from microcontrollers to a web-based HUD.

Resilience: Achieving our full project scope despite losing 7 hours to critical hardware debugging.

📚 What we learned
We learned the hard way that hardware is the most unpredictable part of development. Jumper wires and MCUs can fail without warning, making it essential to double-check equipment. We also realized that clearly defining the problem before coding is just as important as the code itself.

🛤️ What's next for RoverWatch
Aerial Transformation: Transitioning from a ground rover to a drone-based system.

Offline Autonomy: Implementing TinyML to run machine learning models directly on the MCU, eliminating the need for Wi-Fi in disaster zones.

SLAM Mapping: Integrating Simultaneous Localization and Mapping (SLAM) for professional-grade 3D terrain reconstruction.
