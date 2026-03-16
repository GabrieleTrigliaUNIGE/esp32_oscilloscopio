```markdown
# Changelog

Tutte le modifiche importanti a questo progetto verranno documentate in questo file.
Il formato è basato su [Keep a Changelog](https://keepachangelog.com/it-IT/1.0.0/), e questo progetto aderisce al [Semantic Versioning](https://semver.org/lang/it/).

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