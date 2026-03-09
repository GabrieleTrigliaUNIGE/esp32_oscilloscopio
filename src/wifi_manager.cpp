#include "wifi_manager.h"
#include <WiFi.h>
#include <WiFiMulti.h>
#include "arduino_secrets.h"

WiFiMulti wifiMulti;
bool wifiConnesso = false;

bool inizializzaWiFi() {
  WiFi.mode(WIFI_STA);
  
  // Aggiungiamo le reti dal nostro file segreto
  wifiMulti.addAP(SECRET_SSID_1, SECRET_PASS_1);
  wifiMulti.addAP(SECRET_SSID_2, SECRET_PASS_2);
  wifiMulti.addAP(SECRET_SSID_3, SECRET_PASS_3);

  Serial.print("Ricerca reti Wi-Fi note...");
  
  // TIMEOUT DI 10 SECONDI: Risolve il bug del blocco infinito!
  unsigned long startAttemptTime = millis();
  
  while (wifiMulti.run() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n--- CONNESSO! ---");
    Serial.print("SSID: "); Serial.println(WiFi.SSID());
    Serial.print("IP: "); Serial.println(WiFi.localIP());
    wifiConnesso = true;
    return true;
  } else {
    // Se fallisce, spegniamo l'antenna per risparmiare batteria e usciamo puliti
    Serial.println("\n[!] Wi-Fi non trovato. Avvio in Modalità Offline.");
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    wifiConnesso = false;
    return false;
  }
}

bool isWiFiConnesso() {
  return wifiConnesso;
}