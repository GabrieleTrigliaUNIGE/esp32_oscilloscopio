#ifndef DISPLAY_H
#define DISPLAY_H

#include "config.h"

void inizializzaDisplay();
// Aggiunti i parametri vMax e freq
void disegnaOnda(float* buffer, int timebase, bool inHold, float vMax, float freq); 

#endif