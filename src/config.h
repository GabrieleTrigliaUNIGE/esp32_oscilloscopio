#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==========================================
// 🎚️ INTERRUTTORI DI COMPILAZIONE (FEATURES)
// ==========================================
#define USE_DISPLAY      // Attiva/Disattiva lo schermo OLED
#define USE_WIFI         // Attiva/Disattiva Wi-Fi e Web Server
#define USE_CONTROLS     // Attiva/Disattiva Pulsanti e Potenziometro
#define USE_PROCESSING   // Attiva/Disattiva i calcoli matematici (VMax, Freq)
#define USE_WEB_SERVER   // Attiva/Disattiva il Web Server (richiede USE_WIFI)

// ==========================================
// 🧲 MOTORE DI ACQUISIZIONE (Scegline UNO SOLO)
// ==========================================
//#define USE_REAL_ADC       // Legge il segnale vero dal Pin 34
#define USE_SIMULATOR      // Genera una finta onda sinusoidale per i test software

// ==========================================
// 📌 PIN E IMPOSTAZIONI GLOBALI
// ==========================================
#define PIN_SEGNALE 34
#define V_REF 3.3
#define PARTITORE_MOLTIPLICATORE 2.0 
#define BUFFER_SIZE 128
#define MIN_TIMEBASE 20
#define MAX_TIMEBASE 100000

#ifdef USE_CONTROLS
  #define PIN_POTENZIOMETRO 35
  #define PIN_PULSANTE 32
#endif

// ==========================================
// 📺 MODULO DISPLAY
// ==========================================
#ifdef USE_DISPLAY
  #include <Wire.h>
  #include <Adafruit_GFX.h>
  #include <Adafruit_SH110X.h>
  
  #define SCREEN_WIDTH 128
  #define SCREEN_HEIGHT 64
  #define OLED_RESET -1
  
  void inizializzaDisplay();
  void disegnaOnda(float* buffer, int timebase, bool inHold, float vMax, float freq); 
#endif

// ==========================================
// 🌐 MODULO WI-FI E WEB
// ==========================================
#ifdef USE_WIFI
  #include "arduino_secrets.h" // Le tue password sicure
  
  bool inizializzaWiFi();
  bool isWiFiConnesso();
  
  #ifdef USE_WEB_SERVER
    #include <WebServer.h>
    #include <WebSocketsServer.h>
  void inizializzaWebServer();
  void gestisciWeb();
  void inviaDatiWeb(float* buffer, int timebase, float vMax, float freq, bool inHold);
  #endif
#endif

// ==========================================
// 🎛️ MODULO CONTROLLI
// ==========================================
#ifdef USE_CONTROLS
  void inizializzaControlli();
  int leggiTimebase(int &ultimoPotRaw);
  bool gestisciPulsanteHold(bool statoAttuale, unsigned long &ultimoTempoPressione, volatile bool &nuovoFramePronto);
#endif

// ==========================================
// 🧮 MODULO PROCESSING E ACQUISIZIONE
// ==========================================
#ifdef USE_PROCESSING
  void calcolaStatistiche(float* buffer, int timebase, float &vMax, float &freq);
#endif

float getVoltage();
void eseguiRollMode(float* bufferAcq, float* bufferDisp, int tb, unsigned long &ultimoCampionamento, volatile bool &nuovoFramePronto);
void eseguiSmartTrigger(float* bufferAcq, float* bufferDisp, int tb, volatile bool &nuovoFramePronto);

#endif // CONFIG_H