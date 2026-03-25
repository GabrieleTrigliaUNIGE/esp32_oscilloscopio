#include "config.h"

volatile int voceMenuSelezionata = 0;
volatile bool aggiornaSchermoMenu = true;
int ultimoValoreEncoderMenu = DEFAULT_TIMEBASE;

// Gira sul CORE 1 (Input/Logica)
void taskLogicaMenu(ButtonEvent eventoTasto, int valoreEncoder)
{
  // A. Gestione rotazione encoder (Scorrimento)
  if (valoreEncoder > ultimoValoreEncoderMenu)
  {
    voceMenuSelezionata++;
    if (voceMenuSelezionata > 1)
      voceMenuSelezionata = 1;
    aggiornaSchermoMenu = true;
  }
  else if (valoreEncoder < ultimoValoreEncoderMenu)
  {
    voceMenuSelezionata--;
    if (voceMenuSelezionata < 0)
      voceMenuSelezionata = 0;
    aggiornaSchermoMenu = true;
  }
  ultimoValoreEncoderMenu = valoreEncoder;

  // B. Gestione Click (Entra)
  if (eventoTasto == BTN_CLICK)
  {
    if (voceMenuSelezionata == 0)
    {
      statoAttuale = STATE_OSCILLOSCOPE;
      nuovoFramePronto = true; // Forza il primo disegno
    }
    // Qui aggiungeremo l'ingresso al generatore in futuro!
  }

  vTaskDelay(50 / portTICK_PERIOD_MS);
}

// Gira sul CORE 0 (Grafica)
void taskGraficaMenu()
{
  if (!aggiornaSchermoMenu)
    return; // Disegna solo se necessario

#ifdef USE_DISPLAY
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(7, 5);
  display.print("- MENU PRINCIPALE -");
  display.drawLine(0, 16, 127, 16, SH110X_WHITE);

  String voci[2] = {"1. Oscilloscopio", "2. Gen. d'Onda (WIP)"};

  for (int i = 0; i < 2; i++)
  {
    if (i == voceMenuSelezionata)
    {
      display.fillRect(0, 25 + (i * 18), 128, 14, SH110X_WHITE);
      display.setTextColor(SH110X_BLACK, SH110X_WHITE);
    }
    else
    {
      display.setTextColor(SH110X_WHITE, SH110X_BLACK);
    }
    display.setCursor(5, 28 + (i * 18));
    display.print(voci[i]);
  }

  display.display();
#endif

  aggiornaSchermoMenu = false; // Fatto, resettiamo il flag
}