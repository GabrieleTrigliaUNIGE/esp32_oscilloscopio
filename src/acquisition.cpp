#include "config.h"
#include <math.h> // Ci serve per la funzione sin()

// =========================================================
// 🎯 MOTORE 1: SIMULATORE (Senza Hardware)
// =========================================================
#ifdef USE_SIMULATOR
float getVoltage() {

  #define PERIODO_SIMULATO 1000000 // 50 Hz -> 20.000 microsecondi di periodo
  unsigned long t = micros();
  // Creiamo un'onda virtuale a 50 Hz (Periodo = 20.000 microsecondi)
  // L'onda oscilla attorno a 1.6V con un'ampiezza di +/- 1.5V (da 0.1V a 3.1V)
  float fase = (t % PERIODO_SIMULATO) / (float)PERIODO_SIMULATO * 2.0 * PI; 
  return 1.6 + 1.5 * sin(fase);
}
#endif

// =========================================================
// 🔌 MOTORE 2: LETTURA REALE (Con Hardware)
// =========================================================
#ifdef USE_REAL_ADC
float getVoltage() {
  analogRead(PIN_SEGNALE); 
  int rawValue = analogRead(PIN_SEGNALE);
  return ((rawValue / 4095.0) * V_REF) * PARTITORE_MOLTIPLICATORE; 
}
#endif

// =========================================================
// LOGICA DI ACQUISIZIONE (Rimane identica per entrambi i motori!)
// =========================================================
void eseguiRollMode(float* bufferAcq, float* bufferDisp, int tb, unsigned long &ultimoCampionamento, volatile bool &nuovoFramePronto) {
  unsigned long tempoAttuale = micros();
  if (tempoAttuale - ultimoCampionamento >= tb) {
    ultimoCampionamento = tempoAttuale;
    
    for(int i = 0; i < BUFFER_SIZE - 1; i++) {
      bufferAcq[i] = bufferAcq[i+1];
    }
    bufferAcq[BUFFER_SIZE - 1] = getVoltage(); // Chiama il simulatore o il pin reale!
    
    memcpy(bufferDisp, bufferAcq, BUFFER_SIZE * sizeof(float));
    nuovoFramePronto = true;
  } else {
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void eseguiSmartTrigger(float* bufferAcq, float* bufferDisp, int tb, volatile bool &nuovoFramePronto) {
  unsigned long timeoutTrigger = millis();
  
  // Il nostro trigger aspetta che l'onda (vera o simulata) passi per 2.5V
  while(getVoltage() > 2.5 && (millis() - timeoutTrigger < 20)) { }
  while(getVoltage() < 2.5 && (millis() - timeoutTrigger < 20)) { }

  for(int i = 0; i < BUFFER_SIZE; i++) {
    unsigned long startTime = micros();
    bufferAcq[i] = getVoltage();
    unsigned long tempoTrascorso = micros() - startTime;
    if (tb > tempoTrascorso) {
      delayMicroseconds(tb - tempoTrascorso);
    }
  }
  
  memcpy(bufferDisp, bufferAcq, BUFFER_SIZE * sizeof(float));
  nuovoFramePronto = true;
  vTaskDelay(1 / portTICK_PERIOD_MS);
}
