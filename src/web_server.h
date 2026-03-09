#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "config.h"

void inizializzaWebServer();
void gestisciWeb();
void inviaDatiWeb(float* buffer, int timebase, float vMax, float freq);

#endif