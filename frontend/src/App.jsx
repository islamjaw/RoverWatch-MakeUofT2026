import { useState, useEffect } from 'react'
import './App.css'
import { initializeApp } from 'firebase/app';
import { getDatabase, ref, onValue } from 'firebase/database';

// Replace with the config provided by your friend
const firebaseConfig = {
  apiKey: "AIzaSyA1OLI7p_E9mPQpKRrU5vG-gOeaV53lhIM",
  authDomain: "survivalsense-a48fe.firebaseapp.com",
  databaseURL: "https://survivalsense-a48fe-default-rtdb.firebaseio.com",
  projectId: "survivalsense-a48fe",
  storageBucket: "survivalsense-a48fe.firebasestorage.app",
  messagingSenderId: "80781712015",
  appId: "1:80781712015:web:ac6708f703295693ea6efa"
};

const app = initializeApp(firebaseConfig);
const db = getDatabase(app);

function App() {
  const [status, setStatus] = useState({
    image_url: null,
    analysis: "SYSTEM ONLINE - WAITING FOR DATA...",
    status_color: "white",
    timestamp: 0
  })

  const [systemActive, setSystemActive] = useState(false)

  // --- SCI-FI ALARM SOUND ---
const playAlarm = () => {
    try {
      const audioCtx = new (window.AudioContext || window.webkitAudioContext)();
      
      // We will create two pulses
      const createPulse = (startTime, frequency) => {
        const oscillator = audioCtx.createOscillator();
        const gainNode = audioCtx.createGain();

        oscillator.connect(gainNode);
        gainNode.connect(audioCtx.destination);

        // 'square' sounds like a retro alarm, 'sawtooth' sounds like a modern siren
        oscillator.type = 'square'; 
        
        // Frequency Sweep (High to Low)
        // oscillator.frequency.setValueAtTime(frequency, startTime);
        // oscillator.frequency.exponentialRampToValueAtTime(frequency / 2, startTime + 0.3);
        // High-low alternating siren
        oscillator.frequency.setValueAtTime(900, audioCtx.currentTime);
        oscillator.frequency.setValueAtTime(600, audioCtx.currentTime + 1);
        // Play for 0.5 seconds total

        // Volume Envelope (Fade out)
        gainNode.gain.setValueAtTime(0.3, startTime);
        gainNode.gain.exponentialRampToValueAtTime(0.01, startTime + 0.3);

        oscillator.start(startTime);
        oscillator.stop(startTime + 0.3);
      };

      // Play a double pulse (BEEP-BEEP)
      createPulse(audioCtx.currentTime, 880);      // First beep (Note A5)
      createPulse(audioCtx.currentTime + 0.4, 880); // Second beep 0.4s later
      
    } catch (e) {
      console.error("Audio failed", e);
    }
  };

  // --- POLLING LOGIC ---
  // useEffect(() => {
  //   const interval = setInterval(() => {
  //     // Use your Laptop's IP
  //     fetch('http://100.67.151.18:5000/api/status')
  //       .then(response => response.json())
  //       .then(data => {
  //         if (data.timestamp > status.timestamp) {
  //           setStatus(data);
            
  //           // Trigger Alarm if system is active and color is red
  //           if (systemActive && (data.status_color === "#ff0000" || data.status_color === "red")) {
  //             playAlarm();
  //           }
  //         }
  //       })
  //       .catch(error => console.error("Error connecting to Flask:", error))
  //   }, 1000)

  //   return () => clearInterval(interval)
  // }, [status.timestamp, systemActive])
  // --- REAL-TIME DATABASE LISTENER ---
  useEffect(() => {
    // Reference the specific path where your sensor data lives (e.g., 'sensor_data')
    const statusRef = ref(db, 'sensor_data');

    // onValue attaches a listener that fires immediately and then on every change
    const unsubscribe = onValue(statusRef, (snapshot) => {
      const data = snapshot.val();
      if (data) {
        // Update local state with the cloud data
        setStatus(data);
        
        // Trigger Alarm if system is active and color is red
        if (systemActive && (data.status_color === "#ff0000" || data.status_color === "red")) {
          playAlarm();
        }
      }
    });

    // Cleanup: Remove the listener when the component unmounts
    return () => unsubscribe();
  }, [systemActive]); // Only re-run if systemActive changes

  return (
    <div className="hud-container" style={{ borderColor: status.status_color }}>
      {!systemActive && (
        <button className="start-btn" onClick={() => setSystemActive(true)}>
          INITIALIZE AUDIO SYSTEM
        </button>
      )}

      <h1 className="glitch-text">SURVIVAL HEADBAND: MAKEUofT</h1>
      <h2>Abrar, Arnob, Jawwad, Nisarg</h2>
      
      <div className="live-feed">
        {status.image_url ? (
          <img 
            key={status.timestamp} // Forces image refresh
            src={status.image_url} 
            alt="Live Feed" 
            className="scan-image"
          />
        ) : (
          <div className="placeholder">NO SIGNAL</div>
        )}
      </div>

      <div 
        className="analysis-box"
        style={{ color: status.status_color, borderColor: status.status_color }}
      >
        {status.analysis.toUpperCase()}
      </div>

      <p className="system-status">
        ● {systemActive ? "AUDIO READY" : "AUDIO MUTED"} | CONNECTED
      </p>
    </div>
  )
}

export default App