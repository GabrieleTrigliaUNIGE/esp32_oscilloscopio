#include "controls.h"

void inizializzaControlli() {
  pinMode(PIN_PULSANTE, INPUT_PULLUP);
}

int leggiTimebase(int &ultimoPotRaw) {
  analogRead(PIN_POTENZIOMETRO); // Lettura a vuoto
  int potAttuale = analogRead(PIN_POTENZIOMETRO);
  
  // Filtro anti-rumore
  if (abs(potAttuale - ultimoPotRaw) > 20) {
    ultimoPotRaw = potAttuale;
  }
  return map(ultimoPotRaw, 0, 4095, MIN_TIMEBASE, MAX_TIMEBASE);
}

bool gestisciPulsanteHold(bool statoAttuale, unsigned long &ultimoTempoPressione, volatile bool &nuovoFramePronto) {
  if (digitalRead(PIN_PULSANTE) == LOW) {
    if (millis() - ultimoTempoPressione > 250) { 
      statoAttuale = !statoAttuale;           
      ultimoTempoPressione = millis();    
      nuovoFramePronto = true; // Forza l'aggiornamento a schermo
    }
  }
  return statoAttuale;
}