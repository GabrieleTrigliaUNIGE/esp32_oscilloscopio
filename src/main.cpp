#include "config.h"
#include "display.h"
#include "web_server.h" 

// --- IL DOPPIO BUFFER ---
float bufferAcquisizione[BUFFER_SIZE]; // Usato dal Core 1 (Lettura)
float bufferDisplay[BUFFER_SIZE];      // Usato dal Core 0 (Schermo/Web)

// Variabili condivise tra i due Core (devono essere "volatile" per sicurezza)
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
// 📺 TASK CORE 0: Gestione Schermo e Wi-Fi
// =========================================================
void TaskCore0_SchermoWeb(void * pvParameters) {
  for(;;) { // Ciclo infinito del Task
    gestisciWeb(); // Mantiene vivo il Wi-Fi

    // Se il Core 1 ci dice che c'è una nuova "fotografia" pronta...
    if (nuovoFramePronto) {
      // Disegniamo l'onda usando il buffer copiato
      disegnaOnda(bufferDisplay, timebaseCondiviso, holdAttivo);
      inviaDatiWeb(bufferDisplay, timebaseCondiviso);
      
      nuovoFramePronto = false; // Abbassiamo la bandierina in attesa del prossimo
    }
    
    // FONDAMENTALE: Facciamo riposare il Core 0 per 10 millisecondi.
    // Se non lo facciamo, il cane da guardia (Watchdog) riavvia l'ESP32!
    vTaskDelay(10 / portTICK_PERIOD_MS); 
  }
}

// =========================================================
// ⚡ TASK CORE 1: Acquisizione Dati Pura (Massima Velocità)
// =========================================================
void TaskCore1_Acquisizione(void * pvParameters) {
  unsigned long ultimoCampionamento = 0;

  for(;;) {
    // 1. Lettura Pulsante HOLD
    if (digitalRead(PIN_PULSANTE) == LOW) {
      if (millis() - ultimoTempoPressione > 250) { 
        holdAttivo = !holdAttivo;           
        ultimoTempoPressione = millis();    
      }
    }

    // Se siamo in Pausa, riposiamo per 50ms e saltiamo tutto il resto
    if (holdAttivo) {
      vTaskDelay(50 / portTICK_PERIOD_MS);
      continue; 
    }

    // 2. Lettura Timebase
    int tb = map(getPotValue(), 0, 4095, MIN_TIMEBASE, MAX_TIMEBASE);
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
        
        // Copia veloce nel buffer del display e alza la bandierina!
        memcpy(bufferDisplay, bufferAcquisizione, sizeof(bufferAcquisizione));
        nuovoFramePronto = true;
      } else {
        // Mentre aspetta il segnale lento, fa respirare il processore (1ms)
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
      
      // Copia veloce nel buffer del display e alza la bandierina!
      memcpy(bufferDisplay, bufferAcquisizione, sizeof(bufferAcquisizione));
      nuovoFramePronto = true;
      
      // Riposo minimo tra un'acquisizione fulminea e l'altra
      vTaskDelay(1 / portTICK_PERIOD_MS);
    }
  }
}

// =========================================================
// 🚀 SETUP: Inizializzazione e Lancio dei Task
// =========================================================
void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  pinMode(PIN_PULSANTE, INPUT_PULLUP);
  
  inizializzaDisplay(); 
  Wire.setClock(400000); // Overclock OLED a 400kHz
  inizializzaWiFi(); 

  // Creazione del Task per il Wi-Fi e Display sul CORE 0
  xTaskCreatePinnedToCore(
    TaskCore0_SchermoWeb,   // La funzione da eseguire
    "TaskSchermo",          // Nome di debug
    10000,                  // Dimensione della memoria (Stack)
    NULL,                   // Parametri
    1,                      // Priorità (1 = Normale)
    NULL,                   // Gestore (Handle)
    0                       // ESEGUITO SUL CORE 0
  );

  // Creazione del Task per l'Acquisizione veloce sul CORE 1
  xTaskCreatePinnedToCore(
    TaskCore1_Acquisizione, 
    "TaskAcquisizione",     
    10000,                  
    NULL,                   
    2,                      // Priorità (2 = Più alta del display!)
    NULL,                   
    1                       // ESEGUITO SUL CORE 1
  );
}

void loop() {
  // Il classico "loop" di Arduino qui non serve più a niente!
  // Lo uccidiamo per liberare memoria e lasciare spazio ai nostri Task.
  vTaskDelete(NULL);
}