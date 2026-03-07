#ifndef CONFIG_H
#define CONFIG_H

// --- LIBRERIE GLOBALI ---
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// --- PIN HARDWARE ---
#define PIN_SEGNALE 34
#define PIN_POTENZIOMETRO 35
#define PIN_PULSANTE 32 
// I pin I2C dell'OLED sono quelli di default dell'ESP32 (SDA = 21, SCL = 22)

// --- IMPOSTAZIONI SCHERMO ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// --- IMPOSTAZIONI OSCILLOSCOPIO ---
#define BUFFER_SIZE 128 // Esattamente uguale ai pixel di larghezza dello schermo!
#define TRIGGER_LEVEL 2.5
#define V_REF 3.3
#define PARTITORE_MOLTIPLICATORE 2.0

#define MIN_TIMEBASE 20
#define MAX_TIMEBASE 100000

// --- IMPOSTAZIONI WI-FI ---
// Inserisci qui il nome e la password del tuo modem di casa!
#define WIFI_SSID "Titanic"
#define WIFI_PASSWORD "Giugno1220"

#endif