#include "config.h"

// --- IL DOPPIO BUFFER ---
float bufferAcquisizione[BUFFER_SIZE];
float bufferDisplay[BUFFER_SIZE];      

// --- VARIABILI CONDIVISE ---
// Valori di default sicuri nel caso in cui i controlli fisici vengano disattivati
volatile int timebaseCondiviso = 1000;
volatile bool holdAttivo = false;
volatile bool nuovoFramePronto = false;

// =========================================================
// 📺 TASK CORE 0: Gestione Schermo, Wi-Fi e Calcoli
// =========================================================
void TaskCore0_SchermoWeb(void * pvParameters) {
  for(;;) { 
    
    #ifdef USE_WIFI
    if (isWiFiConnesso()) {
      gestisciWeb(); 
    }
    #endif

    if (nuovoFramePronto) {
      // Inizializziamo a zero. Verranno sovrascritti solo se il processing è attivo.
      float vMaxCorrente = 0.0;
      float freqCorrente = 0.0;
      
      #ifdef USE_PROCESSING
      calcolaStatistiche(bufferDisplay, timebaseCondiviso, vMaxCorrente, freqCorrente);
      #endif

      #ifdef USE_DISPLAY
      disegnaOnda(bufferDisplay, timebaseCondiviso, holdAttivo, vMaxCorrente, freqCorrente);
      #endif

      #if defined(USE_WIFI) && defined(USE_WEB_SERVER)
      if (isWiFiConnesso()) {
        inviaDatiWeb(bufferDisplay, timebaseCondiviso, vMaxCorrente, freqCorrente, holdAttivo); 
      }
      #endif
      
      nuovoFramePronto = false; 
    } 
    else if (holdAttivo) {
      static unsigned long ultimoInvioHold = 0;
      if (millis() - ultimoInvioHold > 500) {
        ultimoInvioHold = millis();
        
        float vMaxCorrente = 0.0;
        float freqCorrente = 0.0;
        
        #ifdef USE_PROCESSING
        calcolaStatistiche(bufferDisplay, timebaseCondiviso, vMaxCorrente, freqCorrente);
        #endif
        
        #ifdef USE_DISPLAY
        disegnaOnda(bufferDisplay, timebaseCondiviso, holdAttivo, vMaxCorrente, freqCorrente); 
        #endif

        #if defined(USE_WIFI) && defined(USE_WEB_SERVER)
        if (isWiFiConnesso()) {
          inviaDatiWeb(bufferDisplay, timebaseCondiviso, vMaxCorrente, freqCorrente, holdAttivo);
        }
        #endif
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
  
  #ifdef USE_CONTROLS
  int ultimoPotRaw = 0; 
  unsigned long ultimoTempoPressione = 0;
  #endif

  for(;;) {
    
    #ifdef USE_CONTROLS
    // Se i controlli sono attivati, leggiamo il pulsante
    holdAttivo = gestisciPulsanteHold(holdAttivo, ultimoTempoPressione, nuovoFramePronto);
    #endif

    // La logica di pausa funziona indipendentemente da come è stata attivata
    if (holdAttivo) {
      vTaskDelay(50 / portTICK_PERIOD_MS);
      continue; 
    }

    #ifdef USE_CONTROLS
    // Se i controlli sono attivati, leggiamo la manopola, altrimenti resta a 1000us
    timebaseCondiviso = leggiTimebase(ultimoPotRaw);
    #endif

    // 2. Acquisizione Dati (Sempre attiva, è il cuore dell'oscilloscopio!)
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
  analogReadResolution(12); // Impostiamo la risoluzione a 12 bit (0-4095)
  
  #ifdef USE_CONTROLS
  inizializzaControlli();
  #endif

  #ifdef USE_DISPLAY
  inizializzaDisplay(); 
  Wire.setClock(400000); 
  #endif

  #ifdef USE_WIFI
  if (inizializzaWiFi()) {
    inizializzaWebServer(); 
  }
  #endif

  xTaskCreatePinnedToCore(TaskCore0_SchermoWeb, "TaskSchermo", 10000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(TaskCore1_Acquisizione, "TaskAcquisizione", 10000, NULL, 2, NULL, 1);
}

void loop() {
  vTaskDelete(NULL);
}