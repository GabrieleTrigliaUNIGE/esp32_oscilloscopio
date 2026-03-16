#include "config.h"

#ifdef USE_ENCODER

// Inizializzazione della memoria per le variabili statiche
volatile int SmartEncoder::value = DEFAULT_TIMEBASE;
volatile uint8_t SmartEncoder::statoPrecedente = 0;
volatile int SmartEncoder::contatoreInterno = 0;
int SmartEncoder::step = ENCODER_STEP;
int SmartEncoder::minVal = MIN_TIMEBASE;
int SmartEncoder::maxVal = MAX_TIMEBASE;
int SmartEncoder::pinCLK = -1;
int SmartEncoder::pinDT = -1;

void IRAM_ATTR SmartEncoder::isr() {
  uint8_t CLK_stato = digitalRead(pinCLK);
  uint8_t DT_stato = digitalRead(pinDT);
  
  uint8_t statoCorrente = (CLK_stato << 1) | DT_stato;
  uint8_t movimento = (statoPrecedente << 2) | statoCorrente;
  statoPrecedente = statoCorrente;

  if(movimento == 0b1101 || movimento == 0b0100 || movimento == 0b0010 || movimento == 0b1011) {
    contatoreInterno++; 
  } else if(movimento == 0b1110 || movimento == 0b0111 || movimento == 0b0001 || movimento == 0b1000) {
    contatoreInterno--; 
  }
  
  if (contatoreInterno >= 2) {
    value += step;
    contatoreInterno = 0;
  } else if (contatoreInterno <= -2) {
    value -= step;
    contatoreInterno = 0;
  }
  
  if (value < minVal) value = minVal;
  if (value > maxVal) value = maxVal;
}

void SmartEncoder::begin(int clk, int dt, int startValue, int stepVal, int minimum, int maximum) {
  pinCLK = clk; pinDT = dt; value = startValue; step = stepVal; minVal = minimum; maxVal = maximum;
  pinMode(pinCLK, INPUT_PULLUP);
  pinMode(pinDT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pinCLK), isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pinDT), isr, CHANGE);
}

int SmartEncoder::getValue() { 
  return value; 
}

void SmartEncoder::addValue(int delta) {
  value += delta;
  if (value < minVal) value = minVal;
  if (value > maxVal) value = maxVal;
}

#endif // USE_ENCODER