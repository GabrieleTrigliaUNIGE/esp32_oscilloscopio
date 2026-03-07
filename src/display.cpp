#include "display.h"

// Creiamo l'oggetto display per il chip SH1106
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void inizializzaDisplay()
{
    // Il comando di avvio è leggermente diverso per questo chip
    if (!display.begin(0x3C, true))
    {
        Serial.println(F("OLED non trovato!"));
        for (;;);
    }
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE); // Colore aggiornato
    display.setTextSize(1);
    display.setCursor(10, 25);
    display.print("Oscilloscopio ESP32");
    display.display();
    delay(1500);
}

void disegnaOnda(float *buffer, int timebase, bool inHold)
{
    display.clearDisplay();

    for (int i = 0; i < SCREEN_WIDTH; i += 4)
    {
        display.drawPixel(i, SCREEN_HEIGHT / 2, SH110X_WHITE);
    }

    for (int i = 0; i < BUFFER_SIZE - 1; i++)
    {
        int y1 = map(buffer[i] * 1000, 0, 5000, SCREEN_HEIGHT - 1, 0);
        int y2 = map(buffer[i + 1] * 1000, 0, 5000, SCREEN_HEIGHT - 1, 0);
        y1 = constrain(y1, 0, SCREEN_HEIGHT - 1);
        y2 = constrain(y2, 0, SCREEN_HEIGHT - 1);
        display.drawLine(i, y1, i + 1, y2, SH110X_WHITE);
    }

    // --- TESTI A SCHERMO ---
    display.setCursor(0, 0);
    display.print(timebase);
    display.print("us");

    // Se l'immagine è bloccata, scrivi "HOLD" in alto a destra!
    if (inHold)
    {
        display.setCursor(100, 0);
        display.print("HOLD");
    }

    display.display();
}