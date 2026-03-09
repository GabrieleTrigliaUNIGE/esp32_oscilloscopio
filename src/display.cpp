#include "config.h"

#ifdef USE_DISPLAY

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void inizializzaDisplay() {
  if(!display.begin(0x3C, true)) { 
    Serial.println(F("OLED non trovato!"));
    for(;;); 
  }
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);
  display.setCursor(10, 25);
  display.print("Oscilloscopio ESP32");
  display.display();
  delay(1500); 
}

void disegnaOnda(float* buffer, int timebase, bool inHold, float vMax, float freq) {
  display.clearDisplay(); 

  // Linea centrale di riferimento
  for(int i=0; i<SCREEN_WIDTH; i+=4) {
    display.drawPixel(i, SCREEN_HEIGHT/2, SH110X_WHITE);
  }

  // Disegno dell'onda
  for (int i = 0; i < BUFFER_SIZE - 1; i++) {
    int y1 = map(buffer[i] * 1000, 0, 5000, SCREEN_HEIGHT - 1, 0); 
    int y2 = map(buffer[i+1] * 1000, 0, 5000, SCREEN_HEIGHT - 1, 0);
    y1 = constrain(y1, 0, SCREEN_HEIGHT - 1);
    y2 = constrain(y2, 0, SCREEN_HEIGHT - 1);
    display.drawLine(i, y1, i + 1, y2, SH110X_WHITE); 
  }

  // ==========================================
  // 📊 TESTI E STATISTICHE A SCHERMO
  // ==========================================
  
  // 1. Base dei tempi (In alto a sinistra)
  display.setCursor(0, 0);
  display.print(timebase);
  display.print("us");

  // 2. Scritta HOLD (In alto a destra)
  if (inHold) {
    display.setCursor(104, 0);
    display.print("HOLD");
  }

  // 3. VMax (In basso a sinistra)
  display.setCursor(0, 56);
  display.print("Vmax:"); 
  display.print(vMax, 1); // 1 decimale di precisione
  display.print("V");

  // 4. Frequenza (In basso a destra)
  display.setCursor(76, 56);
  if (freq > 0) {
    display.print(freq, 0); // 0 decimali (numero intero)
    display.print("Hz");
  } else {
    display.print("--Hz"); // Se il segnale è piatto o troppo lento
  }

  display.display(); 
}

#endif