#include "config.h"

#ifdef USE_WEB_SERVER

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

void inizializzaWebServer() {
  if (!LittleFS.begin(true)) { 
    Serial.println("Errore critico: Impossibile montare LittleFS!");
    return;
  }
  
  // 1. REGOLA ESPLICITA PER LA ROOT (Risolve il "Not found: /")
  server.on("/", HTTP_GET, []() {
    File file = LittleFS.open("/index.html", "r");
    if (!file) {
      server.send(500, "text/plain", "Errore: index.html non trovato!");
      return;
    }
    server.streamFile(file, "text/html");
    file.close();
  });

  // 2. REGOLA MAGICA PER TUTTO IL RESTO (style.css, script.js, ecc.)
  server.serveStatic("/", LittleFS, "/");
  
  server.begin();
  webSocket.begin();
  Serial.println("Web Server avviato. Modalità Ibrida attiva.");
}

void gestisciWeb() {
  server.handleClient();
  webSocket.loop();
}

void inviaDatiWeb(float* buffer, int timebase, float vMax, float freq, bool inHold) {
  static unsigned long ultimoInvio = 0;
  if (millis() - ultimoInvio > 50) { 
    ultimoInvio = millis();
    
    String payload = String(timebase) + ";" + String(vMax, 2) + ";" + String(freq, 0) + ";" + String(inHold ? "1" : "0") + ";";
    
    for (int i = 0; i < BUFFER_SIZE; i++) {
      payload += String(buffer[i], 2); 
      if (i < BUFFER_SIZE - 1) payload += ",";
    }
    webSocket.broadcastTXT(payload); 
  }
}

#endif // USE_WIFI