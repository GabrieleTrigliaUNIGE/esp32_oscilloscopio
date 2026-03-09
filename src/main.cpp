#include "config.h"
#include "display.h"
#include "web_server.h" 
#include "processing.h"
#include "controls.h"
#include "acquisition.h"
#include "wifi_manager.h"

// --- DOPPIO BUFFER ---
float bufferAcquisizione[BUFFER_SIZE]; 
float bufferDisplay[BUFFER_SIZE];      

// --- VARIABILI CONDIVISE (Tra i due Core) ---
volatile int timebaseCondiviso = 1000;
volatile bool holdAttivo = false;
volatile bool nuovoFramePronto = false; 

// =========================================================
// 📺 TASK CORE 0: Gestione Schermo, Wi-Fi e Calcoli
// =========================================================
void TaskCore0_SchermoWeb(void * pvParameters) {
  for(;;) { 
    if (isWiFiConnesso()) {
      gestisciWeb(); // Ascolta i client solo se c'è connessione
    }

    if (nuovoFramePronto) {
      float vMaxCorrente = 0.0;
      float freqCorrente = 0.0;
      calcolaStatistiche(bufferDisplay, timebaseCondiviso, vMaxCorrente, freqCorrente);

      disegnaOnda(bufferDisplay, timebaseCondiviso, holdAttivo, vMaxCorrente, freqCorrente);
      
      // Invia al web solo se siamo online!
      if (isWiFiConnesso()) {
        inviaDatiWeb(bufferDisplay, timebaseCondiviso, vMaxCorrente, freqCorrente); 
      }
      
      nuovoFramePronto = false; 
    } 
    else if (holdAttivo) {
      static unsigned long ultimoInvioHold = 0;
      if (millis() - ultimoInvioHold > 500) {
        ultimoInvioHold = millis();
        
        float vMaxCorrente = 0.0;
        float freqCorrente = 0.0;
        calcolaStatistiche(bufferDisplay, timebaseCondiviso, vMaxCorrente, freqCorrente);
        
        disegnaOnda(bufferDisplay, timebaseCondiviso, holdAttivo, vMaxCorrente, freqCorrente); 
        
        if (isWiFiConnesso()) {
          inviaDatiWeb(bufferDisplay, timebaseCondiviso, vMaxCorrente, freqCorrente);
        }
      }
    }
    vTaskDelay(10 / portTICK_PERIOD_MS); 
  }
}

// =========================================================
// ⚡ TASK CORE 1: Acquisizione Dati Pura
// =========================================================
void TaskCore1_Acquisizione(void * pvParameters) {
  unsigned long ultimoCampionamento = 0;
  int ultimoPotRaw = 0; 
  unsigned long ultimoTempoPressione = 0;

  for(;;) {
    // 1. Controlli (Pulsante e Potenziometro)
    holdAttivo = gestisciPulsanteHold(holdAttivo, ultimoTempoPressione, nuovoFramePronto);

    if (holdAttivo) {
      vTaskDelay(50 / portTICK_PERIOD_MS);
      continue; 
    }

    timebaseCondiviso = leggiTimebase(ultimoPotRaw);

    // 2. Acquisizione Dati (Roll o Trigger)
    if (timebaseCondiviso > 5000) {
      eseguiRollMode(bufferAcquisizione, bufferDisplay, timebaseCondiviso, ultimoCampionamento, nuovoFramePronto);
    } else {
      eseguiSmartTrigger(bufferAcquisizione, bufferDisplay, timebaseCondiviso, nuovoFramePronto);
    }
  }
}

// =========================================================
// 🚀 SETUP
// =========================================================
void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  
  inizializzaControlli();
  inizializzaDisplay(); 
  Wire.setClock(400000); 
  
  // LA NUOVA LOGICA DI AVVIO
  if (inizializzaWiFi()) {
    inizializzaWebServer(); // Si avvia solo se connesso
  }

  xTaskCreatePinnedToCore(TaskCore0_SchermoWeb, "TaskSchermo", 10000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(TaskCore1_Acquisizione, "TaskAcquisizione", 10000, NULL, 2, NULL, 1);
}

void loop() {
  vTaskDelete(NULL);
}