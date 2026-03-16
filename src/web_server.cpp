#include "config.h"

#ifdef USE_WEB_SERVER

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// Dichiariamo le variabili globali dal main per poterle manipolare
extern volatile bool holdAttivo;
extern volatile bool nuovoFramePronto;

// La funzione che ascolta i comandi in arrivo dal Browser
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    String msg = String((char*)payload);
    
    if (msg == "CMD:TB_UP") {
      #ifdef USE_ENCODER
      SmartEncoder::addValue(ENCODER_STEP);
      nuovoFramePronto = true; 
      #endif
    } 
    else if (msg == "CMD:TB_DOWN") {
      #ifdef USE_ENCODER
      SmartEncoder::addValue(-ENCODER_STEP);
      nuovoFramePronto = true; 
      #endif
    }
    else if (msg == "CMD:HOLD") {
      holdAttivo = !holdAttivo;
      nuovoFramePronto = true; 
    }
    
    else if (msg.startsWith("CMD:TB_SET:")) {
      #ifdef USE_ENCODER
      // Estraiamo il numero tagliando i primi 11 caratteri ("CMD:TB_SET:")
      int newTb = msg.substring(11).toInt(); 
      SmartEncoder::setValue(newTb);
      nuovoFramePronto = true;
      #endif
    }
  }
}

void inizializzaWebServer() {
  if (!LittleFS.begin(true)) { 
    Serial.println("Errore critico: Impossibile montare LittleFS!");
    return;
  }
  
  server.on("/", HTTP_GET, []() {
    File file = LittleFS.open("/index.html", "r");
    if (!file) {
      server.send(500, "text/plain", "Errore: index.html non trovato!");
      return;
    }
    server.streamFile(file, "text/html");
    file.close();
  });

  server.serveStatic("/", LittleFS, "/");
  
  server.begin();
  webSocket.begin();
  
  // Agganciamo la funzione di ascolto al WebSocket
  webSocket.onEvent(webSocketEvent); 
  
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

#endif // USE_WEB_SERVER