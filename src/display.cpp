#include "config.h"

#ifdef USE_DISPLAY

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void mostraInfoBoot(String versione, String rete, String ip) {
  display.clearDisplay();
  
  // Titolo e riga di separazione
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(8, 5);
  display.print("ESP32 OSCILLOSCOPE");
  display.drawLine(0, 16, 127, 16, SH110X_WHITE);

  // Informazioni di sistema
  display.setCursor(0, 25);
  display.print("Ver:  "); 
  display.print(versione);

  display.setCursor(0, 40);
  display.print("Rete: "); 
  display.print(rete);

  display.setCursor(0, 55);
  display.print("IP:   "); 
  display.print(ip);

  display.display();
}

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

// Disegna un piccolo arco di cerchio per completare l'animazione
// start e end sono angoli in radianti.
void disegnaSegmentoArco(int x, int y, int r, float start, float end, int color) {
  // Passaggio di 0.05 radianti (circa 3 gradi) per avere una linea fluida
  for (float i = start; i < end; i += 0.05) {
    int px1 = x + r * cos(i);
    int py1 = y + r * sin(i);
    int px2 = x + r * cos(i + 0.05);
    int py2 = y + r * sin(i + 0.05);
    display.drawLine(px1, py1, px2, py2, color);
  }
}

// --- 📱 LA TUA NUOVA FUNZIONE ---
// Sostituisce la barra lineare con un cerchio progressivo
void disegnaBarraPressione(int progresso) {
  if (progresso <= 0) return;

  // Parametri del cerchio (Top Right)
  int centerX = 120; 
  int centerY = 8;  
  int radius = 5;   

  // 1. Pulizia dell'area (un rettangolo che contiene il cerchio)
  // Questo serve a non flickerare o coprire il titolo
  display.fillRect(centerX - radius - 1, centerY - radius - 1, (radius + 1) * 2, (radius + 1) * 2, SH110X_BLACK);

  // 2. Disegniamo lo "tracciato" (un cerchio vuoto bianco in background)
  display.drawCircle(centerX, centerY, radius, SH110X_WHITE);

  // 3. Disegniamo l'arco che si completa in base al progresso
  // In C++ gli angoli partono da "Est" (0). Noi vogliamo partire da "Nord" (-PI/2).
  // Un cerchio completo è 2*PI radianti.
  float startAngle = -PI / 2.0; 
  float currentAngle = startAngle + (progresso * (2.0 * PI / 100.0));

  // Disegniamo l'arco del progresso sul bordo esterno
  disegnaSegmentoArco(centerX, centerY, radius, startAngle, currentAngle, SH110X_WHITE);
  // Un altro arco più piccolo internamente per renderlo più "spesso" visivamente
  disegnaSegmentoArco(centerX, centerY, radius - 1, startAngle, currentAngle, SH110X_WHITE);
}

#endif // USE_DISPLAY