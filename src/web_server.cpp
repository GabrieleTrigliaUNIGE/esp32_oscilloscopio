#include "config.h"

#ifdef USE_WEB_SERVER

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 Oscilloscopio</title>
  <style>
    body { font-family: 'Segoe UI', Arial, sans-serif; text-align: center; background-color: #121212; color: white; margin-top: 20px;}
    canvas { background-color: #000; border: 2px solid #333; box-shadow: 0px 0px 15px #00ff00; max-width: 95%; border-radius: 5px; }
    h2 { color: #00ff00; margin-bottom: 5px; text-transform: uppercase; letter-spacing: 2px;}
    .dashboard { display: flex; justify-content: center; gap: 20px; margin-bottom: 15px; font-size: 16px; color: #ccc; }
    .val { color: #fff; font-weight: bold; font-size: 18px; }
  </style>
</head>
<body>
  <h2>Oscilloscopio Wi-Fi</h2>
  
  <div class="dashboard">
    <div>Timebase: <span id="tb" class="val">1000</span> &micro;s</div>
    <div>VMax: <span id="vmax" class="val">0.0</span> V</div>
    <div>Freq: <span id="freq" class="val">0</span> Hz</div>
  </div>

  <canvas id="oscope" width="800" height="400"></canvas>
  <script>
    var canvas = document.getElementById('oscope');
    var ctx = canvas.getContext('2d');
    var gateway = `ws://${window.location.hostname}:81/`;
    var websocket;

    function initWebSocket() {
      websocket = new WebSocket(gateway);
      websocket.onmessage = onMessage;
    }

    function onMessage(event) {
      // NUOVO FORMATO: timebase;vMax;freq;inHold;val1,val2...
      var data = event.data.split(';');
      
      document.getElementById('tb').innerHTML = data[0];
      document.getElementById('vmax').innerHTML = parseFloat(data[1]).toFixed(1);
      
      var freqValue = parseInt(data[2]);
      document.getElementById('freq').innerHTML = (freqValue > 0) ? freqValue : "--";

      var inHold = (data[3] === "1"); // Leggiamo se è in pausa
      var points = data[4].split(',');
      
      ctx.clearRect(0, 0, canvas.width, canvas.height);
      
      var leftMargin = 40;
      var graphWidth = canvas.width - leftMargin;

      // ==========================================
      // ASSE Y E TESTI
      // ==========================================
      ctx.fillStyle = '#ccc';
      ctx.font = '16px Segoe UI';
      ctx.fillText('5V', 10, 20);
      ctx.fillText('0V', 10, canvas.height - 10);

      // Linea asse Y verticale
      ctx.strokeStyle = '#ccc';
      ctx.lineWidth = 2;
      ctx.beginPath();
      ctx.moveTo(leftMargin, 0);
      ctx.lineTo(leftMargin, canvas.height);
      ctx.stroke();

      // Linea tratteggiata centrale (2.5V)
      ctx.strokeStyle = '#333';
      ctx.lineWidth = 1;
      ctx.setLineDash([10, 10]); // Tratteggio
      ctx.beginPath();
      ctx.moveTo(leftMargin, canvas.height / 2);
      ctx.lineTo(canvas.width, canvas.height / 2);
      ctx.stroke();
      ctx.setLineDash([]); // Reset tratteggio

      // ==========================================
      // DISEGNO ONDA
      // ==========================================
      ctx.beginPath();
      ctx.strokeStyle = '#00ff00';
      ctx.lineWidth = 3;
      var xStep = graphWidth / (points.length - 1);
      
      for(var i = 0; i < points.length; i++) {
        var v = parseFloat(points[i]);
        var y = canvas.height - (v / 5.0 * canvas.height); 
        var x = leftMargin + (i * xStep);
        if(i === 0) ctx.moveTo(x, y);
        else ctx.lineTo(x, y);
      }
      ctx.stroke();

      // ==========================================
      // OVERLAY PAUSA MINIMALISTA
      // ==========================================
      if (inHold) {
        var px = leftMargin + (graphWidth / 2) - 10;
        var py = (canvas.height / 2) - 20;
        ctx.fillStyle = '#fff';
        // Disegna due barrette centrali trasparenti (senza sfondo)
        ctx.fillRect(px, py, 6, 40);
        ctx.fillRect(px + 14, py, 6, 40);
      }
    }
    window.addEventListener('load', initWebSocket);
  </script>
</body>
</html>
)rawliteral";

void inizializzaWebServer() {
  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html", index_html);
  });
  server.begin();
  webSocket.begin();
}

void gestisciWeb() {
  server.handleClient();
  webSocket.loop();
}

// Aggiunto "bool inHold" alla funzione
void inviaDatiWeb(float* buffer, int timebase, float vMax, float freq, bool inHold) {
  static unsigned long ultimoInvio = 0;
  if (millis() - ultimoInvio > 50) { 
    ultimoInvio = millis();
    
    // Assembliamo la stringa inserendo anche lo stato della pausa (1 o 0)
    String payload = String(timebase) + ";" + String(vMax, 2) + ";" + String(freq, 0) + ";" + String(inHold ? "1" : "0") + ";";
    
    for (int i = 0; i < BUFFER_SIZE; i++) {
      payload += String(buffer[i], 2); 
      if (i < BUFFER_SIZE - 1) payload += ",";
    }
    webSocket.broadcastTXT(payload); 
  }
}

#endif // USE_WEB_SERVER