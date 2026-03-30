#include "config.h"

// Inizializzazione delle variabili globali condivise
volatile AppState statoAttuale = STATE_MENU;
volatile bool nuovoFramePronto = false;
volatile int timebaseCondiviso = DEFAULT_TIMEBASE;
volatile bool holdAttivo = false;

float bufferAcquisizione[BUFFER_SIZE];
float bufferDisplay[BUFFER_SIZE];      

/// =========================================================
// 📺 TASK CORE 0: Orchestratore Grafico e Web
// =========================================================
void TaskCore0_SchermoWeb(void * pvParameters) {
  for(;;) { 
    #ifdef USE_WIFI
    if (isWiFiConnesso()) { 
      gestisciWeb(); 
      
      // NUOVO: Inviamo lo stato corrente al Browser
      #ifdef USE_WEB_SERVER
      if (statoAttuale == STATE_MENU) {
        extern volatile int voceMenuSelezionata;
        inviaDatiMenuWeb(voceMenuSelezionata);
      } else if (statoAttuale == STATE_GENERATOR) {
        extern volatile int freqGeneratore;
        extern volatile int tipoOnda;
        inviaDatiGeneratoreWeb(freqGeneratore, tipoOnda);
      }
      #endif
    }
    #endif

    // 1. Leggiamo subito se stiamo premendo il tasto
    int progresso = 0;
    #ifdef USE_CONTROLS
    progresso = getProgressoPressione();
    #endif

    // 2. AGGIORNAMENTO GRAFICA (Solo se NON stiamo premendo a lungo!)
    if (progresso == 0) {
      switch (statoAttuale) {
        case STATE_MENU:
          taskGraficaMenu();
          break;
        case STATE_OSCILLOSCOPE:
          taskGraficaOscilloscopio();
          break;
        case STATE_GENERATOR:
          taskGraficaGeneratore();
          break;
      }
    }
    
    // =========================================================
    // 🪄 MAGIA OVERLAY: Animazione Long Press Globale
    // =========================================================
    #if defined(USE_DISPLAY) && defined(USE_CONTROLS)
    static int ultimoProgresso = 0;
    
    // Se il dito è premuto, disegniamo solo la barra sopra l'ultimo frame
    if (progresso > 0 && abs(progresso - ultimoProgresso) > 2) {
      disegnaBarraPressione(progresso);
      display.display(); 
      ultimoProgresso = progresso;
    } 
    // Appena rilasciamo, forziamo il ridisegno completo per far sparire la barra
    else if (progresso == 0 && ultimoProgresso > 0) {
      ultimoProgresso = 0;
      if (statoAttuale == STATE_OSCILLOSCOPE) nuovoFramePronto = true;
      else if (statoAttuale == STATE_MENU) aggiornaSchermoMenu = true;
      else if (statoAttuale == STATE_GENERATOR) aggiornaSchermoGen = true;
    }
    #endif
    // =========================================================

    vTaskDelay(10 / portTICK_PERIOD_MS); 
  }
}

// =========================================================
// ⚡ TASK CORE 1: Orchestratore Input e Logica
// =========================================================
void TaskCore1_Acquisizione(void * pvParameters) {
  unsigned long ultimoCampionamento = 0;
  
  for(;;) {
    #ifdef USE_CONTROLS
    ButtonEvent eventoTasto = leggiEventoEncoder(); 
    int valoreEncoder = leggiTimebase();            
    #else
    ButtonEvent eventoTasto = BTN_NONE;
    int valoreEncoder = DEFAULT_TIMEBASE;
    #endif

    // Smistamento alla logica corretta
    switch (statoAttuale) {
      case STATE_MENU:
        taskLogicaMenu(eventoTasto, valoreEncoder);
        break;
      case STATE_OSCILLOSCOPE:
        taskLogicaOscilloscopio(eventoTasto, valoreEncoder, ultimoCampionamento);
        break;
      case STATE_GENERATOR:
        taskLogicaGeneratore(eventoTasto, valoreEncoder);
        break;
    }
  }
}

// =========================================================
// 🚀 SETUP
// =========================================================
void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  
  #ifdef USE_CONTROLS
  inizializzaControlli();
  #endif

  setupGeneratoreTimer();

  #ifdef USE_DISPLAY
  inizializzaDisplay(); 
  Wire.setClock(400000); 
  #endif

  #ifdef USE_WIFI
  if (inizializzaWiFi()) {
    inizializzaWebServer(); 
    #ifdef USE_DISPLAY
      mostraInfoBoot("1.2.0-MODULAR", WiFi.SSID(), WiFi.localIP().toString());
      delay(3000); 
    #endif
  }
  #endif

  xTaskCreatePinnedToCore(TaskCore0_SchermoWeb, "TaskSchermo", 10000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(TaskCore1_Acquisizione, "TaskAcq", 10000, NULL, 2, NULL, 1);
}

void loop() { vTaskDelete(NULL); }