#include "config.h"

#ifdef USE_PROCESSING

void calcolaStatistiche(float* buffer, int timebase, float &vMax, float &freq) {
  vMax = 0.0;
  float vMin = 5.0; // Ricerca del minimo
  float somma = 0.0;

  for(int i = 0; i < BUFFER_SIZE; i++) {
    if(buffer[i] > vMax) vMax = buffer[i];
    if(buffer[i] < vMin) vMin = buffer[i];
    somma += buffer[i];
  }
  
  float media = somma / BUFFER_SIZE;
  float ampiezza = vMax - vMin;

  // 1. CONTROLLO DI QUALITÀ: Se l'onda è quasi piatta, rifiutiamo il calcolo
  if (ampiezza < 0.2) { 
    freq = 0.0;
    return; 
  }

  // 2. ISTERESI (Filtro Anti-Rumore): Fascia morta del 15%
  float fasciaMorta = ampiezza * 0.15; 
  float sogliaAlta = media + fasciaMorta;
  float sogliaBassa = media - fasciaMorta;

  int incrociSalita = 0;
  int primoIncrocio = -1;
  int ultimoIncrocio = -1;
  bool sottoSoglia = false;

  for(int i = 0; i < BUFFER_SIZE; i++) {
    if(buffer[i] < sogliaBassa) {
      sottoSoglia = true; // "Armato" per la salita
    }
    else if(buffer[i] > sogliaAlta && sottoSoglia) {
      incrociSalita++;
      if(primoIncrocio == -1) primoIncrocio = i;
      ultimoIncrocio = i;
      sottoSoglia = false; // "Disarmato"
    }
  }

  // Calcolo finale se abbiamo almeno un'onda pulita
  if(incrociSalita >= 2) {
    float tempoTotaleUs = (ultimoIncrocio - primoIncrocio) * timebase;
    float periodoUs = tempoTotaleUs / (incrociSalita - 1);
    freq = 1000000.0 / periodoUs; 
  } else {
    freq = 0.0; // Segnale troppo lento per questa finestra
  }
}

#endif // USE_PROCESSING
