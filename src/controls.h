#ifndef CONTROLS_H
#define CONTROLS_H

#include "config.h"
#include <Arduino.h>

void inizializzaControlli();
int leggiTimebase(int &ultimoPotRaw);
bool gestisciPulsanteHold(bool statoAttuale, unsigned long &ultimoTempoPressione, volatile bool &nuovoFramePronto);

#endif