#ifndef ACQUISITION_H
#define ACQUISITION_H

#include "config.h"
#include <Arduino.h>

float getVoltage();
void eseguiRollMode(float* bufferAcq, float* bufferDisp, int tb, unsigned long &ultimoCampionamento, volatile bool &nuovoFramePronto);
void eseguiSmartTrigger(float* bufferAcq, float* bufferDisp, int tb, volatile bool &nuovoFramePronto);

#endif