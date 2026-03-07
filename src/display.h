#ifndef DISPLAY_H
#define DISPLAY_H

#include "config.h"

// Dichiariamo le funzioni che useremo nel main
void inizializzaDisplay();
void disegnaOnda(float* buffer, int timebase, bool inHold);

#endif