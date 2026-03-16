#include "config.h"

#ifdef USE_CONTROLS

volatile int timebaseAttuale = DEFAULT_TIMEBASE;
volatile uint8_t statoPrecedente = 0;
volatile int contatoreInterno = 0; // Il nostro "divisore" segreto!

// La nostra Macchina a Stati iper-reattiva
void IRAM_ATTR encoderISR() {
  uint8_t CLK_stato = digitalRead(ENCODER_PIN_CLK);
  uint8_t DT_stato = digitalRead(ENCODER_PIN_DT);
  
  uint8_t statoCorrente = (CLK_stato << 1) | DT_stato;
  uint8_t movimento = (statoPrecedente << 2) | statoCorrente;
  statoPrecedente = statoCorrente;

  // 1. Validiamo la direzione e aggiorniamo il contatore fantasma
  if(movimento == 0b1101 || movimento == 0b0100 || movimento == 0b0010 || movimento == 0b1011) {
    contatoreInterno++; 
  } 
  else if(movimento == 0b1110 || movimento == 0b0111 || movimento == 0b0001 || movimento == 0b1000) {
    contatoreInterno--; 
  }
  
  // 2. La Magia: convertiamo gli impulsi elettrici in scatti fisici
  // Visto che il tuo encoder fa 2 impulsi per click, scattiamo solo a +2 o -2
  if (contatoreInterno >= 2) {
    timebaseAttuale += ENCODER_STEP;
    contatoreInterno = 0; // Resettiamo per il prossimo click
  } 
  else if (contatoreInterno <= -2) {
    timebaseAttuale -= ENCODER_STEP;
    contatoreInterno = 0; // Resettiamo per il prossimo click
  }
  
  // Manteniamo i valori dentro i limiti imposti dal config.h
  if (timebaseAttuale < MIN_TIMEBASE) timebaseAttuale = MIN_TIMEBASE;
  if (timebaseAttuale > MAX_TIMEBASE) timebaseAttuale = MAX_TIMEBASE;
}

void inizializzaControlli() {
  pinMode(ENCODER_PIN_CLK, INPUT_PULLUP);
  pinMode(ENCODER_PIN_DT, INPUT_PULLUP);
  pinMode(PIN_PULSANTE, INPUT_PULLUP); 
  
  // Agganciamo la nostra letale Macchina a Stati
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_CLK), encoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_DT),  encoderISR, CHANGE);
}

int leggiTimebase() {
  return timebaseAttuale; 
}

bool gestisciPulsanteHold(bool statoAttuale, unsigned long &ultimoTempoPressione, volatile bool &nuovoFramePronto) {
  if (digitalRead(PIN_PULSANTE) == LOW) {
    if (millis() - ultimoTempoPressione > 250) { 
      statoAttuale = !statoAttuale;           
      ultimoTempoPressione = millis();    
      nuovoFramePronto = true; 
    }
  }
  return statoAttuale;
}

#endif