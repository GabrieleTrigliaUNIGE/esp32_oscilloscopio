# Changelog

Tutte le modifiche importanti a questo progetto verranno documentate in questo file.
Il formato è basato su [Keep a Changelog](https://keepachangelog.com/it-IT/1.0.0/), e questo progetto aderisce al [Semantic Versioning](https://semver.org/lang/it/).

## [1.2.0] - 2026-03-28
### Aggiunto
- **Generatore di Funzioni Integrato**: Nuova modalità per generare onde sinusoidali, quadre e triangolari tramite il DAC interno (Pin 25). Frequenza regolabile al volo (1-500 Hz).
- **Menù Principale Navigabile**: Introdotta una schermata OLED dedicata per passare agilmente dalla modalità Oscilloscopio alla modalità Generatore.
- **Web Dashboard "State-Aware"**: L'interfaccia web ora riconosce lo stato del dispositivo. Oscura dinamicamente il grafico e mostra eleganti messaggi di stato in sovrimpressione quando si naviga nel menù o si usa il generatore.
- **UI/UX Display**: Aggiunta una *Circular Progress Bar* (animazione radiale) in alto a destra per fornire un feedback visivo istantaneo durante la pressione lunga (Long Press) dell'encoder.

### Modificato
- **Refactoring Dual-Core & Macchina a Stati**: Il codice è stato diviso in moduli isolati (`app_menu`, `app_oscilloscope`, `app_generator`). Il Core 0 gestisce UI e Wi-Fi, il Core 1 l'acquisizione/generazione hardware.
- **Aggiornamento SmartEncoder**: Libreria resa totalmente non-bloccante. Mantiene gli interrupt (`IRAM_ATTR`) per la rotazione, ma ora integra un polling intelligente basato su `millis()` per il pulsante, supportando click rapidi e pressioni lunghe.
- **Nuovo Pinout Hardware**: Layout dei pin riorganizzato per fare spazio al Generatore d'Onda (Encoder spostato dai pin 25/26 ai pin 27/33, aggiunto SW sul pin 14).

### Risolto
- **Bug visivo CSS Web Dashboard**: Risolto il problema dell'overlay scuro che sbordava dal canvas applicando un *Wrapper Container* con `overflow: hidden`.

---

## [1.1.0] - 2026-03-16
### Aggiunto
- **Web UI Bidirezionale**: Aggiunti pulsanti all'interfaccia web per controllare il timebase e mettere in pausa l'oscilloscopio da remoto tramite WebSockets.
- **Inserimento Manuale Timebase**: Possibilità di digitare il valore esatto del timebase direttamente dalla pagina web.
- **Controllo Hold-to-Repeat**: Pressione continua dei pulsanti web (supporto touch e mouse tramite Pointer Events) per scorrere rapidamente i valori.
- **Supporto Encoder KY-040**: Sostituito il vecchio potenziometro analogico con un encoder rotativo digitale per una precisione assoluta.
- **Architettura OOP**: Introdotti i moduli `SmartEncoder` e `SmartButton` per un'astrazione pulita dell'hardware.

### Modificato
- Refactoring del motore di acquisizione: Rimossa la gestione dei rimbalzi e degli stati dal `main.cpp`, ora delegata interamente agli oggetti hardware.
- Sostituita la lettura analogica dei controlli con una Macchina a Stati (State Machine) gestita via Interrupt per l'encoder, eliminando lag e falsi contatti.

### Risolto
- Bug fix sull'interfaccia web: Risolto conflitto sull'ID HTML che impediva il corretto aggiornamento del campo input del timebase.

---

## [1.0.0] - 2026-03-11
### Aggiunto
- Release iniziale.
- Architettura Dual-Core basata su FreeRTOS.
- Supporto per display OLED I2C (SH110X).
- Interfaccia Web di sola lettura tramite WebSockets e LittleFS.
- Lettura hardware tramite potenziometro analogico (GPIO 35) e pulsante fisico (GPIO 32).
- Motore DSP con Schmitt Trigger e Roll Mode.