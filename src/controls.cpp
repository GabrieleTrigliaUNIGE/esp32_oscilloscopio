#include "config.h"

#ifdef USE_CONTROLS

// Creiamo l'istanza globale del pulsante (se abilitato)
#ifdef USE_BUTTON
  SmartButton btnHold;
#endif

void inizializzaControlli() {
  #ifdef USE_ENCODER
    SmartEncoder::begin(ENCODER_PIN_CLK, ENCODER_PIN_DT, DEFAULT_TIMEBASE, ENCODER_STEP, MIN_TIMEBASE, MAX_TIMEBASE);
  #endif
  
  #ifdef USE_BUTTON
    btnHold.begin(PIN_PULSANTE);
  #endif
}

int leggiTimebase() {
  #ifdef USE_ENCODER
    return SmartEncoder::getValue(); 
  #else
    return DEFAULT_TIMEBASE;
  #endif
}

bool gestisciPulsanteHold(bool statoAttuale, volatile bool &nuovoFramePronto) {
  #ifdef USE_BUTTON
    // Chiediamo al bottone se è stato cliccato (lui gestisce il suo debounce in privato)
    if (btnHold.hasBeenClicked()) {
      statoAttuale = !statoAttuale; // La logica di Hold appartiene all'oscilloscopio
      nuovoFramePronto = true;      // La logica del display appartiene all'oscilloscopio
    }
    return statoAttuale;
  #else
    return statoAttuale;
  #endif
}

#endif // USE_CONTROLS