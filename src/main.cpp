#include "config.h"

// --- IL DOPPIO BUFFER ---
float bufferAcquisizione[BUFFER_SIZE];
float bufferDisplay[BUFFER_SIZE];      

// --- VARIABILI CONDIVISE ---
// Valori di default sicuri nel caso in cui i controlli fisici vengano disattivati
volatile int timebaseCondiviso = DEFAULT_TIMEBASE;
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
  
  // Addio vecchie variabili "ultimoTempoPressione" e "ultimoPotRaw"! 
  // Ora la memoria è gestita dai nostri oggetti Smart.

  for(;;) {
    
    #ifdef USE_CONTROLS
    // Chiamata super pulita: passiamo solo lo stato attuale e il flag del display
    holdAttivo = gestisciPulsanteHold(holdAttivo, nuovoFramePronto);
    #endif

    // La logica di pausa
    if (holdAttivo) {
      vTaskDelay(50 / portTICK_PERIOD_MS);
      continue; 
    }

    #ifdef USE_CONTROLS
    // Lettura istantanea dall'oggetto Encoder
    timebaseCondiviso = leggiTimebase(); 
    #endif

    // 2. Acquisizione Dati
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

    #ifdef USE_DISPLAY
      // Wi-Fi connesso! Mostriamo la schermata di riepilogo
      String ssidAttuale = WiFi.SSID();
      String ipAttuale = WiFi.localIP().toString();
      
      mostraInfoBoot("1.1.0", ssidAttuale, ipAttuale);
      
      // Congeliamo l'ESP32 per 4 secondi per farti leggere l'IP!
      delay(4000); 
    #endif

  }
  #endif

  xTaskCreatePinnedToCore(TaskCore0_SchermoWeb, "TaskSchermo", 10000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(TaskCore1_Acquisizione, "TaskAcquisizione", 10000, NULL, 2, NULL, 1);
}

void loop() {
  vTaskDelete(NULL);
}