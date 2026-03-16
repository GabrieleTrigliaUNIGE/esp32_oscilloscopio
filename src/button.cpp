#include "config.h"

#ifdef USE_BUTTON

// Costruttore: inizializza le variabili a zero
SmartButton::SmartButton() { 
  pin = -1; 
  ultimoTempoPressione = 0; 
}

// Inizializza l'hardware
void SmartButton::begin(int pinButton) { 
  pin = pinButton; 
  pinMode(pin, INPUT_PULLUP); 
}

// Logica "pura": ritorna true solo nell'istante del click
bool SmartButton::hasBeenClicked() {
  if (digitalRead(pin) == LOW) {
    if (millis() - ultimoTempoPressione > 250) { 
      ultimoTempoPressione = millis();    
      return true; 
    }
  }
  return false;
}

#endif // USE_BUTTON