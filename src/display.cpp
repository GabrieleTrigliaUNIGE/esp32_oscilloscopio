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

  // ==========================================
  // 📋 STATUS BAR (Area Superiore 0-11)
  // ==========================================
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE, SH110X_BLACK); 
  
  display.setCursor(0, 1);
  display.print(timebase);
  display.print("us");

  display.setCursor(64, 1);
  display.print("V:");
  display.print(vMax, 1); 
  display.print(" ");
  if (freq > 0) {
    display.print(freq, 0); display.print("Hz");
  } else {
    display.print("--Hz");
  }

  display.drawLine(0, 11, 127, 11, SH110X_WHITE);

  // ==========================================
  // 📏 ASSE Y E TENSIONI (Colonna Sinistra 0-17)
  // ==========================================
  int topGrafico = 12;
  int bottomGrafico = SCREEN_HEIGHT - 1;
  int leftGrafico = 18; 

  display.setCursor(0, topGrafico + 2);
  display.print("5V");
  display.setCursor(0, bottomGrafico - 7);
  display.print("0V");
  display.drawLine(leftGrafico - 2, topGrafico, leftGrafico - 2, bottomGrafico, SH110X_WHITE);

  // ==========================================
  // 🌊 AREA GRAFICO E DISEGNO OTTIMIZZATO
  // ==========================================
  int centroGrafico = topGrafico + ((bottomGrafico - topGrafico) / 2);

  for(int i = leftGrafico; i < SCREEN_WIDTH; i += 4) {
    display.drawPixel(i, centroGrafico, SH110X_WHITE);
  }

  // Disegno dell'onda OTTMIZZATO PER BUFFER GRANDI (Decimazione)
  for (int x = leftGrafico; x < SCREEN_WIDTH - 1; x++) {
    int index1 = map(x, leftGrafico, SCREEN_WIDTH - 1, 0, BUFFER_SIZE - 1);
    int index2 = map(x + 1, leftGrafico, SCREEN_WIDTH - 1, 0, BUFFER_SIZE - 1);
    
    int y1 = map(buffer[index1] * 1000, 0, 5000, bottomGrafico, topGrafico); 
    int y2 = map(buffer[index2] * 1000, 0, 5000, bottomGrafico, topGrafico);
    
    y1 = constrain(y1, topGrafico, bottomGrafico);
    y2 = constrain(y2, topGrafico, bottomGrafico);
    
    display.drawLine(x, y1, x + 1, y2, SH110X_WHITE); 
  }

  // ==========================================
  // ⏸️ OVERLAY PAUSA CENTRALE (Minimalista)
  // ==========================================
  if (inHold) {
    int pauseWidth = 10;
    int pauseHeight = 12;
    int px = leftGrafico + ((SCREEN_WIDTH - leftGrafico) / 2) - (pauseWidth / 2);
    int py = topGrafico + ((bottomGrafico - topGrafico) / 2) - (pauseHeight / 2);

    display.fillRect(px, py, 3, pauseHeight, SH110X_WHITE);
    display.fillRect(px + 7, py, 3, pauseHeight, SH110X_WHITE);
  }

  display.display(); 
}

#endif // USE_DISPLAY