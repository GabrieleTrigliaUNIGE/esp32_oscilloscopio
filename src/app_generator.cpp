#include "config.h"
#include <driver/dac.h>

extern volatile bool aggiornaSchermoMenu;

volatile int freqGeneratore = 50; 
volatile int tipoOnda = 0; // 0: SPENTO, 1: Sinusoide, 2: Quadra, 3: Triangolare
volatile bool aggiornaSchermoGen = true;
int ultimoValoreEncoderGen = DEFAULT_TIMEBASE; 

// =========================================================
// ⏱️ MOTORE DDS IN BACKGROUND (Hardware Timer)
// =========================================================
hw_timer_t * timerGen = NULL;
volatile uint32_t phaseAccumulator = 0;
volatile uint32_t phaseIncrement = 0;

// Wavetables in RAM (256 punti)
uint8_t waveSine[256];
uint8_t waveSquare[256];
uint8_t waveTriangle[256];

// L'Interrupt ora usa le istruzioni hardware dirette (molto più veloci)
void IRAM_ATTR onTimerGen() {
  if (tipoOnda == 0) return; 
  
  phaseAccumulator += phaseIncrement;
  uint8_t index = phaseAccumulator >> 24; 

  // Usiamo DAC_CHANNEL_1 (che corrisponde fisicamente al PIN 25)
  if (tipoOnda == 1) dac_output_voltage(DAC_CHANNEL_1, waveSine[index]);
  else if (tipoOnda == 2) dac_output_voltage(DAC_CHANNEL_1, waveSquare[index]);
  else if (tipoOnda == 3) dac_output_voltage(DAC_CHANNEL_1, waveTriangle[index]);
}

void aggiornaTimerGeneratore() {
  // Calcolo DDS ricalibrato per un timer a 25 kHz (25000)
  phaseIncrement = (uint32_t)(((uint64_t)freqGeneratore * 4294967296ULL) / 25000);
  if (tipoOnda == 0) dac_output_voltage(DAC_CHANNEL_1, 0); 
}

void setupGeneratoreTimer() {
  // Abilitiamo il DAC a livello hardware
  dac_output_enable(DAC_CHANNEL_1);

  for(int i = 0; i < 256; i++) {
    waveSine[i] = (uint8_t)(127.5 * (1.0 + sin(2.0 * PI * i / 256.0)));
    waveSquare[i] = (i < 128) ? 255 : 0;
    waveTriangle[i] = (i < 128) ? (i * 2) : (255 - ((i - 128) * 2));
  }

  // Timer impostato su 40 microsecondi (25 kHz) invece di 10
  timerGen = timerBegin(0, 80, true); 
  timerAttachInterrupt(timerGen, &onTimerGen, true);
  timerAlarmWrite(timerGen, 40, true); 
  timerAlarmEnable(timerGen);
  
  aggiornaTimerGeneratore();
}

// =========================================================
// ⚡ CORE 1: Logica e Input
// =========================================================
void taskLogicaGeneratore(ButtonEvent eventoTasto, int valoreEncoder) {
  
  // A. Esci (Long Press) - NON spegniamo più il DAC qui!
  if (eventoTasto == BTN_LONG_PRESS) {
    statoAttuale = STATE_MENU;
    aggiornaSchermoMenu = true;
    #ifdef USE_ENCODER
    SmartEncoder::setValue(DEFAULT_TIMEBASE);
    #endif
    return;
  }

  // B. Cambio Tipo Onda (Click) - Passa da SPENTO -> SINE -> SQUARE -> TRI
  if (eventoTasto == BTN_CLICK) {
    tipoOnda++;
    if (tipoOnda > 3) tipoOnda = 0;
    aggiornaTimerGeneratore(); // Applica subito
    aggiornaSchermoGen = true;
  }

  // C. Cambio Frequenza (Rotazione)
  if (valoreEncoder != ultimoValoreEncoderGen) {
    int delta = (valoreEncoder - ultimoValoreEncoderGen) / ENCODER_STEP;
    freqGeneratore += delta; 
    
    if (freqGeneratore < 1) freqGeneratore = 1;
    if (freqGeneratore > 500) freqGeneratore = 500; 
    
    ultimoValoreEncoderGen = valoreEncoder;
    #ifdef USE_ENCODER
    SmartEncoder::setValue(ultimoValoreEncoderGen); 
    #endif
    
    aggiornaTimerGeneratore(); // Applica subito la nuova velocità!
    aggiornaSchermoGen = true;
  }

  // Nessun calcolo matematico qui! Il Core 1 si riposa.
  vTaskDelay(50 / portTICK_PERIOD_MS); 
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
  display.setCursor(7, 5); 
  display.print("- GENERATORE ONDA -");
  display.drawLine(0, 16, 127, 16, SH110X_WHITE);

  // Forma d'onda
  display.setCursor(0, 25);
  display.print("Stato: ");
  if (tipoOnda == 0) display.print("SPENTO (OFF)");
  else if (tipoOnda == 1) display.print("Sinusoide");
  else if (tipoOnda == 2) display.print("Quadra");
  else if (tipoOnda == 3) display.print("Triangolare");

  // Frequenza
  display.setTextSize(2); 
  display.setCursor(0, 40);
  display.print(freqGeneratore);
  display.setTextSize(1);
  display.print(" Hz");

  // Istruzioni
  display.setCursor(0, 56);
  display.print("[Click]Onda [Gira]Hz");

  display.display();
  #endif

  aggiornaSchermoGen = false;
}