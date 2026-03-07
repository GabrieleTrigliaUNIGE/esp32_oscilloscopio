#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "config.h"

void inizializzaWiFi();
void gestisciWeb();
void inviaDatiWeb(float* buffer, int timebase);

#endif