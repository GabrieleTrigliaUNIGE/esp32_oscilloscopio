#include "web_server.h"
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

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
      // Il nuovo formato è: "timebase;vMax;freq;val1,val2..."
      var data = event.data.split(';');
      
      // Aggiorna i testi HTML
      document.getElementById('tb').innerHTML = data[0];
      document.getElementById('vmax').innerHTML = parseFloat(data[1]).toFixed(1);
      
      var freqValue = parseInt(data[2]);
      document.getElementById('freq').innerHTML = (freqValue > 0) ? freqValue : "--";

      var points = data[3].split(',');
      
      ctx.clearRect(0, 0, canvas.width, canvas.height);
      
      // Griglia centrale
      ctx.strokeStyle = '#333';
      ctx.lineWidth = 1;
      ctx.beginPath();
      ctx.moveTo(0, canvas.height / 2);
      ctx.lineTo(canvas.width, canvas.height / 2);
      ctx.stroke();

      // Disegna l'onda
      ctx.beginPath();
      ctx.strokeStyle = '#00ff00';
      ctx.lineWidth = 3;
      var xStep = canvas.width / (points.length - 1);
      
      for(var i = 0; i < points.length; i++) {
        var v = parseFloat(points[i]);
        var y = canvas.height - (v / 5.0 * canvas.height); 
        var x = i * xStep;
        if(i === 0) ctx.moveTo(x, y);
        else ctx.lineTo(x, y);
      }
      ctx.stroke();
    }
    window.addEventListener('load', initWebSocket);
  </script>
</body>
</html>
)rawliteral";

void inizializzaWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connessione al Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n--- CONNESSO! ---");
  Serial.print("Indirizzo IP Web: ");
  Serial.println(WiFi.localIP());

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

void inviaDatiWeb(float* buffer, int timebase, float vMax, float freq) {
  static unsigned long ultimoInvio = 0;
  if (millis() - ultimoInvio > 50) { 
    ultimoInvio = millis();
    
    // Assembliamo la nuova mega-stringa con tutti i dati!
    String payload = String(timebase) + ";" + String(vMax, 2) + ";" + String(freq, 0) + ";";
    
    for (int i = 0; i < BUFFER_SIZE; i++) {
      payload += String(buffer[i], 2); 
      if (i < BUFFER_SIZE - 1) payload += ",";
    }
    webSocket.broadcastTXT(payload); 
  }
}