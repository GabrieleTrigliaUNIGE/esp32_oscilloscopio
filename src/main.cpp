#include "config.h"
#include "display.h"
#include "web_server.h" 

float dataBuffer[BUFFER_SIZE];
unsigned long timebase = 1000;
unsigned long ultimoCampionamento = 0;

// --- VARIABILI PULSANTE CORRETTE ---
bool holdAttivo = false;
unsigned long ultimoTempoPressione = 0;

float getVoltage() {
  analogRead(PIN_SEGNALE); 
  int rawValue = analogRead(PIN_SEGNALE);
  return ((rawValue / 4095.0) * V_REF) * PARTITORE_MOLTIPLICATORE; 
}

int getPotValue() {
  analogRead(PIN_POTENZIOMETRO); 
  return analogRead(PIN_POTENZIOMETRO);
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  
  pinMode(PIN_PULSANTE, INPUT_PULLUP);
  
  inizializzaDisplay(); 
  
  // 🚀 OVERCLOCK I2C: Quadruplica gli FPS dello schermo OLED!
  Wire.setClock(400000); 
  
  inizializzaWiFi(); 
}

void loop() {
  gestisciWeb(); 

  // ==========================================
  // 🔘 LETTURA PULSANTE (Antirimbalzo Infallibile)
  // ==========================================
  if (digitalRead(PIN_PULSANTE) == LOW) {
    if (millis() - ultimoTempoPressione > 250) { 
      holdAttivo = !holdAttivo;           
      ultimoTempoPressione = millis();    
    }
  }

  // ==========================================
  // 📊 ACQUISIZIONE DATI (Solo se NON in Hold)
  // ==========================================
  if (!holdAttivo) {
    timebase = map(getPotValue(), 0, 4095, MIN_TIMEBASE, MAX_TIMEBASE);

    if (timebase > 5000) {
      // 🐢 ROLL MODE (Per segnali lenti)
      unsigned long tempoAttuale = micros();
      if (tempoAttuale - ultimoCampionamento >= timebase) {
        ultimoCampionamento = tempoAttuale;
        for(int i = 0; i < BUFFER_SIZE - 1; i++) {
          dataBuffer[i] = dataBuffer[i+1];
        }
        dataBuffer[BUFFER_SIZE - 1] = getVoltage();
        
        disegnaOnda(dataBuffer, timebase, holdAttivo);
        inviaDatiWeb(dataBuffer, timebase); 
      }
    } else {
      // 🐇 SMART TRIGGER MODE (Per segnali veloci, fluido e sincronizzato)
      unsigned long timeoutTrigger = millis();
      
      // Cerca il segnale scendere sotto i 2.5V (Timeout max 20ms)
      while(getVoltage() > 2.5 && (millis() - timeoutTrigger < 20)) { }
      
      // Cerca il segnale salire sopra i 2.5V (Timeout max 20ms)
      while(getVoltage() < 2.5 && (millis() - timeoutTrigger < 20)) { }

      // Acquisizione veloce dei dati
      for(int i = 0; i < BUFFER_SIZE; i++) {
        unsigned long startTime = micros();
        dataBuffer[i] = getVoltage();
        unsigned long tempoTrascorso = micros() - startTime;
        if (timebase > tempoTrascorso) {
          delayMicroseconds(timebase - tempoTrascorso);
        }
      }
      
      disegnaOnda(dataBuffer, timebase, holdAttivo);
      inviaDatiWeb(dataBuffer, timebase); 
    }
  } else {
    // ⏸️ HOLD MODE: Mantiene vivo l'aggiornamento per chi si connette dal Web
    static unsigned long ultimoInvioHold = 0;
    if (millis() - ultimoInvioHold > 500) { 
      ultimoInvioHold = millis();
      disegnaOnda(dataBuffer, timebase, holdAttivo);
      inviaDatiWeb(dataBuffer, timebase);
    }
  }
}