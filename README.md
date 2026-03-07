# ⚡ ESP32 Smart Oscilloscope (Dual-Core & Wi-Fi)

Un oscilloscopio digitale fai-da-te ad alte prestazioni basato su ESP32. Sfrutta il sistema operativo in tempo reale (FreeRTOS) per gestire l'acquisizione dei dati su un core dedicato, garantendo un'interfaccia fluida sia sul display OLED locale sia sul browser web tramite WebSockets.

## ✨ Caratteristiche Principali

* **Architettura Multi-Core (FreeRTOS):** Acquisizione del segnale sul Core 1 (nessun lag) e gestione interfaccia/Wi-Fi sul Core 0.
* **Interfaccia Web in Tempo Reale:** Visualizza l'onda direttamente sul browser del tuo smartphone o PC a frequenze di aggiornamento elevate tramite protocollo WebSocket.
* **Display OLED Locale:** Supporto integrato per display I2C da 1.3" (Chip SH1106) con I2C overcloccato a 400kHz per la massima fluidità.
* **Controllo Hardware del Timebase:** Regola lo zoom temporale (da 20 µs a 100.000 µs) in tempo reale ruotando un potenziometro fisico.
* **Smart Trigger & Roll Mode:** Transizione automatica tra un'onda "congelata" (per segnali veloci) e uno scorrimento continuo stile elettrocardiogramma (per segnali lenti).
* **Funzione HOLD Hardware:** Pulsante fisico con antirimbalzo software per bloccare la lettura e analizzare il segnale con calma.

---

## 🛠️ Componenti Hardware Necessari

* 1x Scheda di sviluppo **ESP32** (es. DevKit V1)
* 1x Display **OLED I2C 1.3"** (Chip SH1106, indirizzo `0x3C`)
* 1x Potenziometro lineare (es. 10kΩ)
* 1x Pulsante (Push-button)
* 2x Resistenze dello stesso valore (es. 10kΩ) per il partitore di tensione
* Breadboard e cavi jumper

---

## 🔌 Schema dei Collegamenti

| Componente | Pin ESP32 | Note Importanti |
| :--- | :--- | :--- |
| **Segnale In** (Centro partitore) | `GPIO 34` | **MAX 3.3V!** Il partitore dimezza il segnale d'ingresso. |
| **Potenziometro** | `GPIO 35` | Collegare gli estremi a `3.3V` e `GND`. |
| **Pulsante HOLD** | `GPIO 32` | Collegare l'altro piedino al `GND` (Pull-Up interno usato). |
| **OLED SDA** | `GPIO 21` | Linea dati I2C. |
| **OLED SCK/SCL** | `GPIO 22` | Linea di clock I2C. |
| **GND in Comune** | `GND` | **Cruciale:** Unire il GND del circuito in esame al GND dell'ESP32. |

> ⚠️ **AVVERTENZA DI SICUREZZA:** L'ADC dell'ESP32 tollera un massimo assoluto di 3.3V. Tensioni superiori danneggeranno irreparabilmente la scheda. Non utilizzare questo strumento per misurare la tensione di rete (230V AC) o segnali di potenza senza un adeguato circuito di isolamento e attenuazione professionale.

---

## 💻 Installazione e Software

Questo progetto è stato sviluppato utilizzando **PlatformIO**. 

1. Clona questa repository:
   ```bash
   git clone [https://github.com/TUO_NOME_UTENTE/NOME_REPO.git](https://github.com/TUO_NOME_UTENTE/NOME_REPO.git)
