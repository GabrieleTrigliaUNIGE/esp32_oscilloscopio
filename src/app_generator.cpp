#include "config.h"

extern volatile bool aggiornaSchermoMenu;

volatile int freqGeneratore = 50; // Partiamo da 50 Hz
volatile int tipoOnda = 0;        // 0: Sinusoide, 1: Quadra, 2: Triangolare
volatile bool aggiornaSchermoGen = true;
int ultimoValoreEncoderGen = DEFAULT_TIMEBASE; 

// =========================================================
// ⚡ CORE 1: Logica e Generazione Hardware (DAC)
// =========================================================
void taskLogicaGeneratore(ButtonEvent eventoTasto, int valoreEncoder) {
  
  // A. Esci e torna al menu (Long Press)
  if (eventoTasto == BTN_LONG_PRESS) {
    statoAttuale = STATE_MENU;
    aggiornaSchermoMenu = true;
    dacWrite(PIN_GENERATORE, 0); // Spegniamo l'uscita analogica prima di uscire!
    
    #ifdef USE_ENCODER
    SmartEncoder::setValue(DEFAULT_TIMEBASE);
    #endif
    return;
  }

  // B. Cambio Tipo Onda (Click)
  if (eventoTasto == BTN_CLICK) {
    tipoOnda++;
    if (tipoOnda > 2) tipoOnda = 0;
    aggiornaSchermoGen = true;
  }

  // C. Cambio Frequenza (Rotazione)
  if (valoreEncoder != ultimoValoreEncoderGen) {
    // Calcoliamo di quanti "scatti" fisici hai girato l'encoder
    int delta = (valoreEncoder - ultimoValoreEncoderGen) / ENCODER_STEP;
    freqGeneratore += delta; // Aumenta/Diminuisci di 5 Hz a scatto
    
    // Limiti di sicurezza
    if (freqGeneratore < 1) freqGeneratore = 1;
    if (freqGeneratore > 500) freqGeneratore = 500; // Oltre i 500Hz via software perde precisione
    
    ultimoValoreEncoderGen = valoreEncoder;
    #ifdef USE_ENCODER
    SmartEncoder::setValue(ultimoValoreEncoderGen); // Sincronizza
    #endif
    
    aggiornaSchermoGen = true;
  }

  // D. 🧮 MOTORE DI GENERAZIONE ONDA (Scrittura sul PIN 25)
  unsigned long t = micros();
  float periodo = 1000000.0 / freqGeneratore;
  float fase = fmod(t, periodo) / periodo; // Restituisce un valore da 0.0 a 1.0 lungo un ciclo

  uint8_t dacValue = 0; // Il DAC dell'ESP32 è a 8 bit (0-255)

  if (tipoOnda == 0) { // SINE WAVE
    dacValue = (uint8_t)(127.5 * (1.0 + sin(fase * 2.0 * PI)));
  } 
  else if (tipoOnda == 1) { // SQUARE WAVE
    dacValue = (fase < 0.5) ? 255 : 0;
  } 
  else if (tipoOnda == 2) { // TRIANGLE WAVE
    if (fase < 0.5) {
      dacValue = (uint8_t)(fase * 2.0 * 255.0); // Salita
    } else {
      dacValue = (uint8_t)((1.0 - fase) * 2.0 * 255.0); // Discesa
    }
  }

  // Scrive la tensione reale sul Pin 25!
  dacWrite(PIN_GENERATORE, dacValue);

  // per non far crashare FreeRTOS 
  delayMicroseconds(50); 
}

// =========================================================
// 📺 CORE 0: Interfaccia Grafica
// =========================================================
void taskGraficaGeneratore() {
  if (!aggiornaSchermoGen) return;

  #ifdef USE_DISPLAY
  display.clearDisplay();
  
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(7, 5); // Centrato come piace a te!
  display.print("- GENERATORE ONDA -");
  display.drawLine(0, 16, 127, 16, SH110X_WHITE);

  // Forma d'onda
  display.setCursor(0, 25);
  display.print("Forma: ");
  if (tipoOnda == 0) display.print("Sinusoide");
  else if (tipoOnda == 1) display.print("Quadra");
  else if (tipoOnda == 2) display.print("Triangolare");

  // Frequenza
  display.setTextSize(2); // Facciamo la frequenza bella grande!
  display.setCursor(0, 40);
  display.print(freqGeneratore);
  display.setTextSize(1);
  display.print(" Hz");

  // Istruzioni in basso a destra
  display.setCursor(0, 56);
  display.print("[Click]Onda [Gira]Hz");

  display.display();
  #endif

  aggiornaSchermoGen = false;
}