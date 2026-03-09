#include "acquisition.h"

float getVoltage() {
  analogRead(PIN_SEGNALE); 
  int rawValue = analogRead(PIN_SEGNALE);
  return ((rawValue / 4095.0) * V_REF) * PARTITORE_MOLTIPLICATORE; 
}

void eseguiRollMode(float* bufferAcq, float* bufferDisp, int tb, unsigned long &ultimoCampionamento, volatile bool &nuovoFramePronto) {
  unsigned long tempoAttuale = micros();
  if (tempoAttuale - ultimoCampionamento >= tb) {
    ultimoCampionamento = tempoAttuale;
    
    for(int i = 0; i < BUFFER_SIZE - 1; i++) {
      bufferAcq[i] = bufferAcq[i+1];
    }
    bufferAcq[BUFFER_SIZE - 1] = getVoltage();
    
    memcpy(bufferDisp, bufferAcq, BUFFER_SIZE * sizeof(float));
    nuovoFramePronto = true;
  } else {
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void eseguiSmartTrigger(float* bufferAcq, float* bufferDisp, int tb, volatile bool &nuovoFramePronto) {
  unsigned long timeoutTrigger = millis();
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