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
int SmartEncoder::pinSW = -1;

// Inizializzazione variabili per il tasto
unsigned long SmartEncoder::tempoInizioPressione = 0;
bool SmartEncoder::statoPrecedenteSW = HIGH;
bool SmartEncoder::inPressione = false;
bool SmartEncoder::lungaPressioneSegnalata = false;

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

void SmartEncoder::begin(int clk, int dt, int sw, int startValue, int stepVal, int minimum, int maximum) {
  pinCLK = clk; pinDT = dt; pinSW = sw; 
  value = startValue; step = stepVal; minVal = minimum; maxVal = maximum;
  
  pinMode(pinCLK, INPUT_PULLUP);
  pinMode(pinDT, INPUT_PULLUP);
  pinMode(pinSW, INPUT_PULLUP); // Setup del pin del tasto
  
  attachInterrupt(digitalPinToInterrupt(pinCLK), isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pinDT), isr, CHANGE);
}

int SmartEncoder::getValue() { return value; }
void SmartEncoder::addValue(int delta) { setValue(value + delta); }

void SmartEncoder::setValue(int val) {
  value = val;
  if (value < minVal) value = minVal;
  if (value > maxVal) value = maxVal;
}

// Nuova logica Non-Bloccante per il Tasto dell'Encoder
ButtonEvent SmartEncoder::getButtonEvent() {
  bool statoAttuale = digitalRead(pinSW);
  unsigned long tempoAttuale = millis();
  ButtonEvent evento = BTN_NONE;

  // 1. Pressione del tasto (con anti-rimbalzo di 50ms)
  if (statoAttuale == LOW && statoPrecedenteSW == HIGH && (tempoAttuale - tempoInizioPressione > 50)) {
    tempoInizioPressione = tempoAttuale;
    inPressione = true;
    lungaPressioneSegnalata = false;
  }

  // 2. Tasto tenuto premuto (Long Press)
  if (statoAttuale == LOW && inPressione) {
    if (!lungaPressioneSegnalata && (tempoAttuale - tempoInizioPressione > ENCODER_LONG_PRESS_MS)) {
      evento = BTN_LONG_PRESS;
      lungaPressioneSegnalata = true;
    }
  }

  // 3. Rilascio del tasto (Click)
  if (statoAttuale == HIGH && statoPrecedenteSW == LOW) {
    if (inPressione && !lungaPressioneSegnalata) {
      if (tempoAttuale - tempoInizioPressione > 50) { // Anti-rimbalzo al rilascio
        evento = BTN_CLICK;
      }
    }
    inPressione = false;
  }

  statoPrecedenteSW = statoAttuale;
  return evento;
}

// Calcola la percentuale di completamento del Long Press (0-100)
int SmartEncoder::getLongPressProgress() {
  if (!inPressione || lungaPressioneSegnalata) return 0;
  
  unsigned long tempoAttuale = millis();
  if (tempoAttuale - tempoInizioPressione < 50) return 0; // Debounce iniziale
  
  // Mappiamo il tempo trascorso in percentuale
  int progresso = map(tempoAttuale - tempoInizioPressione, 50, ENCODER_LONG_PRESS_MS, 0, 100);
  return constrain(progresso, 0, 100); // Assicuriamoci che non superi il 100%
}

#endif // USE_ENCODER