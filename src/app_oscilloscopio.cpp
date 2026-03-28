#include "config.h"

// Questo flag serve a dire al menu di ridisegnarsi quando usciamo dall'oscilloscopio
extern volatile bool aggiornaSchermoMenu; 

// Gira sul CORE 1 (Acquisizione/Input)
void taskLogicaOscilloscopio(ButtonEvent eventoTasto, int valoreEncoder, unsigned long &ultimoCampionamento) {
  // A. Esci e torna al menu (Long Press)
  if (eventoTasto == BTN_LONG_PRESS) {
    statoAttuale = STATE_MENU;
    aggiornaSchermoMenu = true; 
    
    // Un piccolo trucco per evitare che l'encoder, appena tornati al menu, 
    // registri uno "scatto" involontario:
    #ifdef USE_ENCODER
    SmartEncoder::setValue(DEFAULT_TIMEBASE);
    #endif
    return; 
  }

  // B. Logica normale dell'Oscilloscopio
  #ifdef USE_CONTROLS
  holdAttivo = gestisciPulsanteHold(holdAttivo, nuovoFramePronto);
  #endif

  if (holdAttivo) {
    vTaskDelay(50 / portTICK_PERIOD_MS);
    return; 
  }

  timebaseCondiviso = valoreEncoder; 

  if (timebaseCondiviso > 5000) {
    eseguiRollMode(bufferAcquisizione, bufferDisplay, timebaseCondiviso, ultimoCampionamento, nuovoFramePronto);
  } else {
    eseguiSmartTrigger(bufferAcquisizione, bufferDisplay, timebaseCondiviso, nuovoFramePronto);
  }
}

// Gira sul CORE 0 (Grafica)
void taskGraficaOscilloscopio() {
  if (nuovoFramePronto) {
    float vMaxCorrente = 0.0; float freqCorrente = 0.0;
    
    #ifdef USE_PROCESSING
      calcolaStatistiche(bufferDisplay, timebaseCondiviso, vMaxCorrente, freqCorrente);
    #endif

    #ifdef USE_DISPLAY
    disegnaOnda(bufferDisplay, timebaseCondiviso, holdAttivo, vMaxCorrente, freqCorrente);
    #endif

    // Gestione dei WebSockets centralizzata qui
    #if defined(USE_WIFI) && defined(USE_WEB_SERVER)
    if (isWiFiConnesso()) {
      inviaDatiWeb(bufferDisplay, timebaseCondiviso, vMaxCorrente, freqCorrente, holdAttivo); 
    }
    #endif
    
    nuovoFramePronto = false; 
  } else if (holdAttivo) {
    static unsigned long ultimoInvioHold = 0;
    if (millis() - ultimoInvioHold > 500) {
      ultimoInvioHold = millis();
      #ifdef USE_DISPLAY
      disegnaOnda(bufferDisplay, timebaseCondiviso, holdAttivo, 0.0, 0.0); 
      #endif
    }
  }
}