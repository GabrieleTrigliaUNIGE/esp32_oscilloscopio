#include "config.h"

#ifdef USE_CONTROLS

// Creiamo un'istanza del nostro pulsante intelligente
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
    return DEFAULT_TIMEBASE; // Valore di fallback se l'encoder è disattivato
  #endif
}

bool gestisciPulsanteHold(bool statoAttuale, volatile bool &nuovoFramePronto) {
  #ifdef USE_BUTTON
    // L'oggetto update ora ci dice solo se è stato PREMUTO (true) o no (false)
    if (btnHold.update(nuovoFramePronto)) {
      statoAttuale = !statoAttuale; // Invertiamo lo stato globale
    }
    return statoAttuale;
  #else
    return statoAttuale; // Se il bottone è disabilitato, lo stato non cambia
  #endif
}

#endif // USE_CONTROLS