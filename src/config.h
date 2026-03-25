#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- MACCHINA A STATI GLOBALE ---
enum AppState {
  STATE_MENU,           // 0: Siamo nel menù principale
  STATE_OSCILLOSCOPE,   // 1: Siamo nell'oscilloscopio
  STATE_GENERATOR       // 2: Siamo nel generatore d'onda
};

enum ButtonEvent {
  BTN_NONE,       // Nessuna azione
  BTN_CLICK,      // Click veloce (Invio)
  BTN_LONG_PRESS  // Pressione prolungata (Indietro)
};

// ==========================================
// 🎚️ INTERRUTTORI DI COMPILAZIONE (FEATURES)
// ==========================================
#define USE_DISPLAY      // Attiva/Disattiva lo schermo OLED
#define USE_WIFI         // Attiva/Disattiva Wi-Fi e Web Server
#define USE_PROCESSING   // Attiva/Disattiva i calcoli matematici (VMax, Freq)
#define USE_WEB_SERVER   // Attiva/Disattiva il Web Server (richiede USE_WIFI)

#define USE_CONTROLS     // Attiva/Disattiva in blocco i controlli fisici
#ifdef USE_CONTROLS
  #define USE_ENCODER    // Attiva la logica dell'Encoder Rotativo KY-040
  #define USE_BUTTON     // Attiva la logica del Pulsante fisico
#endif

// ==========================================
// 🧲 MOTORE DI ACQUISIZIONE (Scegline UNO SOLO)
// ==========================================
// #define USE_REAL_ADC       // Legge il segnale vero dal Pin 34
#define USE_SIMULATOR      // Genera una finta onda sinusoidale per i test software

// ==========================================
// 📌 PIN E IMPOSTAZIONI GLOBALI
// ==========================================
#define PIN_SEGNALE 34
#define V_REF 3.3
#define PARTITORE_MOLTIPLICATORE 2.0 
#define BUFFER_SIZE 128

// Limiti e default della scala dei tempi
#define MIN_TIMEBASE 20
#define MAX_TIMEBASE 100000
#define DEFAULT_TIMEBASE 9000

#ifdef USE_CONTROLS
  #ifdef USE_ENCODER
    #define ENCODER_PIN_CLK 27  // Spostato dal 25 per liberare il DAC
    #define ENCODER_PIN_DT  33  // Spostato dal 26
    #define ENCODER_PIN_SW  14
    #define ENCODER_STEP    50  // Quanto varia il timebase a ogni scatto
  #endif

  #ifdef USE_BUTTON
    #define PIN_PULSANTE 32     // Pin del pulsante fisico per la pausa/selezione
  #endif
#endif

// Pin del DAC per il Generatore di Funzioni
#define PIN_GENERATORE 25

// ==========================================
// 🧩 CLASSI DEI DISPOSITIVI (HARDWARE ABSTRACTION)
// ==========================================
#ifdef USE_ENCODER
class SmartEncoder {
  private:
    static volatile int value;
    static volatile uint8_t statoPrecedente;
    static volatile int contatoreInterno;
    static int step, minVal, maxVal, pinCLK, pinDT, pinSW;
    
    // Variabili per la gestione del tasto integrato
    static unsigned long tempoInizioPressione;
    static bool statoPrecedenteSW;
    static bool inPressione;
    static bool lungaPressioneSegnalata;

    static void IRAM_ATTR isr();
  public:
    static void begin(int clk, int dt, int sw, int startValue, int stepVal, int minimum, int maximum);
    static int getValue();
    static void addValue(int delta);
    static void setValue(int val);
    static ButtonEvent getButtonEvent(); // Cattura Click e Long Press
};
#endif

#ifdef USE_BUTTON
class SmartButton {
  private:
    int pin;
    unsigned long ultimoTempoPressione;
  public:
    SmartButton();
    void begin(int pinButton);
    bool hasBeenClicked(); 
};
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
  void mostraInfoBoot(String versione, String rete, String ip);
#endif

// ==========================================
// 🌐 MODULO WI-FI E WEB
// ==========================================
#ifdef USE_WIFI
  #include "arduino_secrets.h" 
  
  bool inizializzaWiFi();
  bool isWiFiConnesso();
  
  #ifdef USE_WEB_SERVER
    #include <WebServer.h>
    #include <WebSocketsServer.h>
    #include <LittleFS.h>

    void inizializzaWebServer();
    void gestisciWeb();
    void inviaDatiWeb(float* buffer, int timebase, float vMax, float freq, bool inHold);
  #endif
#endif

// ==========================================
// 🎛️ MODULO CONTROLLI GLOBALI
// ==========================================
#ifdef USE_CONTROLS
  void inizializzaControlli();
  int leggiTimebase();
  bool gestisciPulsanteHold(bool statoAttuale, volatile bool &nuovoFramePronto);
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

extern volatile AppState statoAttuale;

#endif // CONFIG_H