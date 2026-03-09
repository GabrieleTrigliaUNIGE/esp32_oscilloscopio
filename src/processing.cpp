#include "processing.h"

void calcolaStatistiche(float* buffer, int timebase, float &vMax, float &freq) {
  vMax = 0.0;
  float somma = 0.0;

  for(int i = 0; i < BUFFER_SIZE; i++) {
    if(buffer[i] > vMax) vMax = buffer[i];
    somma += buffer[i];
  }
  float media = somma / BUFFER_SIZE;

  int incrociSalita = 0;
  int primoIncrocio = -1;
  int ultimoIncrocio = -1;

  for(int i = 1; i < BUFFER_SIZE; i++) {
    if(buffer[i-1] < media && buffer[i] >= media) {
      incrociSalita++;
      if(primoIncrocio == -1) primoIncrocio = i;
      ultimoIncrocio = i;
    }
  }

  if(incrociSalita >= 2) {
    float tempoTotaleUs = (ultimoIncrocio - primoIncrocio) * timebase;
    float periodoUs = tempoTotaleUs / (incrociSalita - 1);
    freq = 1000000.0 / periodoUs; 
  } else {
    freq = 0.0; 
  }
}