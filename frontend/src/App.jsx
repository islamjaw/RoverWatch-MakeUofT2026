import { useState, useEffect } from 'react'
import { initializeApp } from 'firebase/app';
import { getDatabase, ref, onValue, update } from 'firebase/database';
import './App.css'

const firebaseConfig = {
  apiKey: "AIzaSyA1OLI7p_E9mPQpKRrU5vG-gOeaV53lhIM",
  authDomain: "survivalsense-a48fe.firebaseapp.com",
  databaseURL: "https://survivalsense-a48fe-default-rtdb.firebaseio.com",
  projectId: "survivalsense-a48fe"
};

const app = initializeApp(firebaseConfig);
const db = getDatabase(app);

function App() {
  const [status, setStatus] = useState({ 
    image_url: null, analysis: "Initializing...", threat_level: 0, 
    status_color: "#00ff00", temp: 0, gas: 0, humidity: 0, battery: 100, timestamp: 0 
  });
  const [mapData, setMapData] = useState({});
  const [selectedCell, setSelectedCell] = useState(null);
  const [isSimulating, setIsSimulating] = useState(false);
  const [audioContext, setAudioContext] = useState(null);

  // Derive UI colors based on threat level (>5) or simulation
  const isHighThreat = status.threat_level > 5 || status.status_color === "red";
  const uiThemeColor = isHighThreat ? "#ff0000" : "#00ff00";
  const uiTint = isHighThreat ? "rgba(40, 0, 0, 0.95)" : "rgba(0, 15, 0, 0.95)";

  useEffect(() => {
    const initAudio = () => { if (!audioContext) setAudioContext(new (window.AudioContext || window.webkitAudioContext)()); };
    document.addEventListener('click', initAudio, { once: true });
    return () => document.removeEventListener('click', initAudio);
  }, [audioContext]);

  const playAlarm = () => {
    if (!audioContext) return;
    const now = audioContext.currentTime;
    [0, 0.2, 0.4].forEach(t => {
      const osc = audioContext.createOscillator();
      const gain = audioContext.createGain();
      osc.connect(gain); gain.connect(audioContext.destination);
      osc.type = 'square'; osc.frequency.value = 800;
      gain.gain.setValueAtTime(0.3, now + t);
      gain.gain.exponentialRampToValueAtTime(0.01, now + t + 0.15);
      osc.start(now + t); osc.stop(now + t + 0.15);
    });
  };

  useEffect(() => {
    onValue(ref(db, 'sensor_data'), (s) => {
      const data = s.val();
      if (data) {
        setStatus(prev => ({ ...prev, ...data }));
        if (data.threat_level > 5) playAlarm(); // Trigger alarm on threat
      }
    });
    onValue(ref(db, 'tactical_map'), (s) => setMapData(s.val() || {}));
  }, [audioContext]);

  const handleToggle = async () => {
    const nextState = !isSimulating;
    setIsSimulating(nextState);
    try {
      if (nextState) {
        playAlarm();
        const updates = {};
        Object.keys(mapData).forEach(k => { if (k !== 'ROVER_LATEST') updates[`tactical_map/${k}/status`] = 2; });
        await update(ref(db), updates);
        await update(ref(db, 'sensor_data'), { status_color: "red", threat_level: 10, analysis: "🚨 SIMULATED EMERGENCY ACTIVE" });
      } else {
        await update(ref(db, 'sensor_data'), { status_color: "#00ff00", threat_level: 0, analysis: "Normal scanning resumed." });
      }
    } catch (err) { console.error(err); }
  };

  // Inside the renderGrid function
  const renderGrid = () => {
    const rover = mapData.ROVER_LATEST || { lat: 43.6628, lon: -79.3958 };
    const items = [];
    
    for (let r = -4; r <= 4; r++) {
      for (let c = -7; c <= 7; c++) {
        const key = `${Math.round(rover.lat / 0.0001) - r}_${Math.round(rover.lon / 0.0001) + c}`;
        const cell = mapData[key];

        // Check if this square is the AI's recommended path
        const isTarget = status.target_square === key;

        items.push(
          <div 
            key={key} 
            className={`grid-box status-${cell?.status || 0} 
              ${r === 0 && c === 0 ? 'rover' : ''} 
              ${isTarget ? 'ai-target' : ''}`} // Add the target class
            onClick={() => setSelectedCell(cell)}
          >
            {r === 0 && c === 0 ? '🤖' : isTarget ? '🎯' : ''}
          </div>
        );
      }
    }
    return items;
  };

  return (
    <div className="hud-container" style={{ "--primary-glow": uiThemeColor, "--danger-tint": uiTint, borderColor: "var(--primary-glow)" }}>
      <div className="main-branding" style={{ background: uiThemeColor }}>
        <h1>RoverWatch: MakeUofT2026</h1>
        <div className="toggle-container">
          <span className="toggle-label">SIM DANGER</span>
          <label className="switch">
            <input type="checkbox" checked={isSimulating} onChange={handleToggle} /><span className="slider round"></span>
          </label>
        </div>
      </div>
      <div className="top-bar">
        <span>Time: {new Date().toLocaleTimeString()}</span>
        <span style={{ color: uiThemeColor, fontWeight: 'bold' }}>Threat: {status.threat_level}/10</span>
        <span>Battery: {status.battery}%</span>
      </div>
      <div className="analysis-strip">
         <span className="label">RoverWatch Analysis</span>
         <div className="value">{status.analysis}</div>
      </div>
      <div className="main-content-layout">
        <div className="left-panel"><div className="live-feed">{status.image_url ? <img src={status.image_url} className="scan-image" alt="Feed" /> : <div className="placeholder">Searching...</div>}</div></div>
        <div className="right-panel"><h3 className="panel-title">Tactical Radar</h3><div className="grid-mesh">{renderGrid()}</div></div>
      </div>
      {selectedCell && (
        <div className="modal" onClick={() => setSelectedCell(null)}>
          <div className="modal-content" onClick={e => e.stopPropagation()}>
            <h4>{selectedCell.isRover ? "🤖 LIVE DATA" : "📍 SECTOR LOG"}</h4>
            <div className="data-row">🌡️ TEMP: {selectedCell.temp}°C | 🌫️ GAS: {selectedCell.gas}</div>
            <p className="ai-note">🧠 AI: {selectedCell.isRover ? status.analysis : selectedCell.analysis}</p>
            <button className="close-btn" onClick={() => setSelectedCell(null)}>CLOSE</button>
          </div>
        </div>
      )}
    </div>
  );
}
export default App;