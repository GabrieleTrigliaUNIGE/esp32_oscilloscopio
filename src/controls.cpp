#include "config.h"

#ifdef USE_CONTROLS

#ifdef USE_BUTTON
  SmartButton btnHold;
#endif

void inizializzaControlli() {
  #ifdef USE_ENCODER
    SmartEncoder::begin(ENCODER_PIN_CLK, ENCODER_PIN_DT, ENCODER_PIN_SW, DEFAULT_TIMEBASE, ENCODER_STEP, MIN_TIMEBASE, MAX_TIMEBASE);
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

ButtonEvent leggiEventoEncoder() {
  #ifdef USE_ENCODER
    return SmartEncoder::getButtonEvent();
  #else
    return BTN_NONE;
  #endif
}

bool gestisciPulsanteHold(bool statoAttuale, volatile bool &nuovoFramePronto) {
  #ifdef USE_BUTTON
    if (btnHold.hasBeenClicked() == BTN_CLICK) {
      statoAttuale = !statoAttuale; 
      nuovoFramePronto = true;      
    }
    return statoAttuale;
  #else
    return statoAttuale;
  #endif
}

int getProgressoPressione() {
  #ifdef USE_ENCODER
    return SmartEncoder::getLongPressProgress();
  #else
    return 0;
  #endif
}

#endif // USE_CONTROLS