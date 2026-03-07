#include "web_server.h"
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// L'interfaccia grafica HTML/JavaScript salvata in memoria
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 Oscilloscopio</title>
  <style>
    body { font-family: Arial; text-align: center; background-color: #121212; color: white; margin-top: 20px;}
    canvas { background-color: #000; border: 2px solid #333; box-shadow: 0px 0px 15px #00ff00; max-width: 95%; border-radius: 5px; }
    h2 { color: #00ff00; margin-bottom: 5px; }
    .info { font-size: 14px; color: #aaa; margin-bottom: 15px; }
  </style>
</head>
<body>
  <h2>Oscilloscopio Wi-Fi</h2>
  <div class="info">Timebase: <span id="tb">1000</span> &micro;s</div>
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
      // Riceviamo i dati dall'ESP32 nel formato "timebase;val1,val2,val3..."
      var data = event.data.split(';');
      document.getElementById('tb').innerHTML = data[0];
      var points = data[1].split(',');
      
      ctx.clearRect(0, 0, canvas.width, canvas.height);
      
      // Disegna la linea di mezzo del Trigger (2.5V)
      ctx.strokeStyle = '#333';
      ctx.lineWidth = 1;
      ctx.beginPath();
      ctx.moveTo(0, canvas.height / 2);
      ctx.lineTo(canvas.width, canvas.height / 2);
      ctx.stroke();

      // Disegna l'onda
      ctx.beginPath();
      ctx.strokeStyle = '#00ff00'; // Verde fosforo
      ctx.lineWidth = 3;
      
      var xStep = canvas.width / (points.length - 1);
      
      for(var i = 0; i < points.length; i++) {
        var v = parseFloat(points[i]);
        var y = canvas.height - (v / 5.0 * canvas.height); // Mappa da 0-5V all'altezza del canvas
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

void inizializzaWiFi()
{
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connessione al Wi-Fi");

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\n--- CONNESSO! ---");
    Serial.print("Indirizzo IP Oscilloscopio: ");
    Serial.println(WiFi.localIP()); // <-- QUESTO È L'INDIRIZZO DA APRIRE NEL BROWSER!

    server.on("/", HTTP_GET, []()
              { server.send_P(200, "text/html", index_html); });

    server.begin();
    webSocket.begin();
}

void gestisciWeb()
{
    server.handleClient();
    webSocket.loop();
}

void inviaDatiWeb(float *buffer, int timebase)
{
    // Inviamo i dati al web al massimo 20 volte al secondo per non bloccare l'ESP32
    static unsigned long ultimoInvio = 0;
    if (millis() - ultimoInvio > 50)
    {
        ultimoInvio = millis();

        // Prepariamo la stringa: "timebase;punto1,punto2,punto3..."
        String payload = String(timebase) + ";";
        for (int i = 0; i < BUFFER_SIZE; i++)
        {
            payload += String(buffer[i], 2);
            if (i < BUFFER_SIZE - 1)
                payload += ",";
        }
        webSocket.broadcastTXT(payload); // Spara i dati a chiunque sia connesso alla pagina!
    }
}