#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "config.h"

void inizializzaWiFi();
void gestisciWeb();

// Aggiunti i parametri vMax e freq qui!
void inviaDatiWeb(float* buffer, int timebase, float vMax, float freq);

#endif