import express from "express";
const router = express.Router();
import { SerialPort } from 'serialport';

const port = new SerialPort({
  path: '/dev/ttyAMA0',
  baudRate: 115200,
});

let cmd_interval = null;

// Hilfsfunktion: Hex zu RGB wandeln
function hexToRgb(hex) {
  // Entferne %23 oder # am Anfang
  const cleanHex = hex.replace('%23', '').replace('#', '');
  const result = /^([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(cleanHex);
  return result ? {
    r: parseInt(result[1], 16),
    g: parseInt(result[2], 16),
    b: parseInt(result[3], 16)
  } : null;
}

function sendCommand(cmd, res) {
  // WICHTIG: Alten Intervall sofort löschen, um Platz für neuen Befehl zu machen
  if (cmd_interval) {
    clearInterval(cmd_interval);
    cmd_interval = null;
  }
  
  const fullCmd = cmd + "\n";
  console.log(`Sende an Arduino: ${cmd}`);
  port.write(fullCmd);
  
  // Wir antworten dem Browser SOFORT, damit die Seite nicht hängt
  if (res) res.status(200).send({ status: "sent", command: cmd });

  // Optional: Einmalige Wiederholung nach 100ms, falls der erste Versuch im "Undervoltage" unterging
  setTimeout(() => {
    port.write(fullCmd);
  }, 100);
}

// Feedback vom Arduino (nur zum Loggen)
port.on('data', (data) => {
  console.log(`Arduino-Antwort: ${data.toString().trim()}`);
});

router.post("*", (req, res) => {
  const data = req.body;
  console.log("Web-Befehl erhalten:", data);

  if (!data.cmd) return res.status(400).send("Kein cmd vorhanden");

  // 1. FARBRAD (Erkennt %23 als Ersatz für #)
  if (data.cmd.startsWith('%23') || data.cmd.startsWith('#')) {
    const rgb = hexToRgb(data.cmd);
    if (rgb) {
      sendCommand(`c:${rgb.r},${rgb.g},${rgb.b}`, res);
    } else {
      res.status(400).send("Ungültiger Hex-Code");
    }
    return;
  }

  // 2. MODI (%01, %02, etc.)
  if (data.cmd.startsWith('%')) {
    const rawId = data.cmd.replace('%', '');
    
    // Mapping-Tabelle
    const mapping = {
      "01": "s",    // Off
      "02": "m:2",  // Rainbow
      "03": "m:2",  // Christmas -> Rainbow
      "04": "m:3",  // Color Pulse -> Breathe
      "05": "m:4"   // Ambiance -> Running
    };

    const targetCmd = mapping[rawId] || `m:${parseInt(rawId, 10)}`;
    sendCommand(targetCmd, res);
    return;
  }

  res.status(200).send("Daten empfangen");
});

export default router;
