#include "config.h"
#include "display.h"
#include "web_server.h" 

float bufferAcquisizione[BUFFER_SIZE]; 
float bufferDisplay[BUFFER_SIZE];      

volatile int timebaseCondiviso = 1000;
volatile bool holdAttivo = false;
volatile bool nuovoFramePronto = false; 

unsigned long ultimoTempoPressione = 0;

float getVoltage() {
  analogRead(PIN_SEGNALE); 
  int rawValue = analogRead(PIN_SEGNALE);
  return ((rawValue / 4095.0) * V_REF) * PARTITORE_MOLTIPLICATORE; 
}

int getPotValue() {
  analogRead(PIN_POTENZIOMETRO); 
  return analogRead(PIN_POTENZIOMETRO);
}

// =========================================================
// 🧮 MOTORE MATEMATICO (VMax e Frequenza)
// =========================================================
void calcolaStatistiche(float* buffer, int timebase, float &vMax, float &freq) {
  vMax = 0.0;
  float somma = 0.0;

  // Trova il Picco Massimo e la Media del segnale
  for(int i = 0; i < BUFFER_SIZE; i++) {
    if(buffer[i] > vMax) vMax = buffer[i];
    somma += buffer[i];
  }
  float media = somma / BUFFER_SIZE;

  // Calcolo della Frequenza contando i punti di incrocio sulla media
  int incrociSalita = 0;
  int primoIncrocio = -1;
  int ultimoIncrocio = -1;

  for(int i = 1; i < BUFFER_SIZE; i++) {
    // Se il punto precedente era sotto la media e l'attuale è sopra, è una salita!
    if(buffer[i-1] < media && buffer[i] >= media) {
      incrociSalita++;
      if(primoIncrocio == -1) primoIncrocio = i;
      ultimoIncrocio = i;
    }
  }

  // Se abbiamo trovato almeno un'onda completa (2 incroci in salita)
  if(incrociSalita >= 2) {
    float tempoTotaleUs = (ultimoIncrocio - primoIncrocio) * timebase;
    float periodoUs = tempoTotaleUs / (incrociSalita - 1);
    freq = 1000000.0 / periodoUs; // Converte Periodo in Frequenza (Hz)
  } else {
    freq = 0.0; // Segnale piatto o non si ripete abbastanza
  }
}

// =========================================================
// 📺 TASK CORE 0: Gestione Schermo e Wi-Fi
// =========================================================
void TaskCore0_SchermoWeb(void * pvParameters) {
  for(;;) { 
    gestisciWeb(); 

    if (nuovoFramePronto) {
      float vMaxCorrente = 0.0;
      float freqCorrente = 0.0;
      calcolaStatistiche(bufferDisplay, timebaseCondiviso, vMaxCorrente, freqCorrente);

      disegnaOnda(bufferDisplay, timebaseCondiviso, holdAttivo, vMaxCorrente, freqCorrente);
      
      // ECCO LA MODIFICA 1: Aggiungiamo le variabili qui!
      inviaDatiWeb(bufferDisplay, timebaseCondiviso, vMaxCorrente, freqCorrente); 
      
      nuovoFramePronto = false; 
    } 
    else if (holdAttivo) {
      static unsigned long ultimoInvioHold = 0;
      if (millis() - ultimoInvioHold > 500) {
        ultimoInvioHold = millis();
        
        float vMaxCorrente = 0.0;
        float freqCorrente = 0.0;
        calcolaStatistiche(bufferDisplay, timebaseCondiviso, vMaxCorrente, freqCorrente);
        
        disegnaOnda(bufferDisplay, timebaseCondiviso, holdAttivo, vMaxCorrente, freqCorrente); 
        
        // ECCO LA MODIFICA 2: Aggiungiamo le variabili anche qui!
        inviaDatiWeb(bufferDisplay, timebaseCondiviso, vMaxCorrente, freqCorrente);
      }
    }
    
    vTaskDelay(10 / portTICK_PERIOD_MS); 
  }
}

// =========================================================
// ⚡ TASK CORE 1: Acquisizione Dati Pura
// =========================================================
void TaskCore1_Acquisizione(void * pvParameters) {
  unsigned long ultimoCampionamento = 0;
  int ultimoPotRaw = 0; // Memoria per il filtro del potenziometro

  for(;;) {
    // 1. Lettura Pulsante HOLD
    if (digitalRead(PIN_PULSANTE) == LOW) {
      if (millis() - ultimoTempoPressione > 250) { 
        holdAttivo = !holdAttivo;           
        ultimoTempoPressione = millis();    
        
        // FORZA L'AGGIORNAMENTO! Fa comparire o sparire la scritta HOLD
        nuovoFramePronto = true; 
      }
    }

    if (holdAttivo) {
      vTaskDelay(50 / portTICK_PERIOD_MS);
      continue; 
    }

    // 2. Lettura Timebase con FILTRO ANTI-RUMORE (Deadband)
    int potAttuale = getPotValue();
    // Aggiorniamo il valore SOLO se hai girato davvero la manopola (ignoriamo il tremolio)
    if (abs(potAttuale - ultimoPotRaw) > 20) {
      ultimoPotRaw = potAttuale;
    }
    int tb = map(ultimoPotRaw, 0, 4095, MIN_TIMEBASE, MAX_TIMEBASE);
    timebaseCondiviso = tb;

    // 3. Campionamento
    if (tb > 5000) {
      // 🐢 ROLL MODE
      unsigned long tempoAttuale = micros();
      if (tempoAttuale - ultimoCampionamento >= tb) {
        ultimoCampionamento = tempoAttuale;
        
        for(int i = 0; i < BUFFER_SIZE - 1; i++) {
          bufferAcquisizione[i] = bufferAcquisizione[i+1];
        }
        bufferAcquisizione[BUFFER_SIZE - 1] = getVoltage();
        
        memcpy(bufferDisplay, bufferAcquisizione, sizeof(bufferAcquisizione));
        nuovoFramePronto = true;
      } else {
        vTaskDelay(1 / portTICK_PERIOD_MS);
      }
      
    } else {
      // 🐇 SMART TRIGGER MODE 
      unsigned long timeoutTrigger = millis();
      while(getVoltage() > 2.5 && (millis() - timeoutTrigger < 20)) { }
      while(getVoltage() < 2.5 && (millis() - timeoutTrigger < 20)) { }

      for(int i = 0; i < BUFFER_SIZE; i++) {
        unsigned long startTime = micros();
        bufferAcquisizione[i] = getVoltage();
        unsigned long tempoTrascorso = micros() - startTime;
        if (tb > tempoTrascorso) {
          delayMicroseconds(tb - tempoTrascorso);
        }
      }
      
      memcpy(bufferDisplay, bufferAcquisizione, sizeof(bufferAcquisizione));
      nuovoFramePronto = true;
      
      vTaskDelay(1 / portTICK_PERIOD_MS);
    }
  }
}

// =========================================================
// 🚀 SETUP
// =========================================================
void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  pinMode(PIN_PULSANTE, INPUT_PULLUP);
  
  inizializzaDisplay(); 
  Wire.setClock(400000); 
  inizializzaWiFi(); 

  xTaskCreatePinnedToCore(TaskCore0_SchermoWeb, "TaskSchermo", 10000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(TaskCore1_Acquisizione, "TaskAcquisizione", 10000, NULL, 2, NULL, 1);
}

void loop() {
  vTaskDelete(NULL);
}