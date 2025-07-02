#include <WiFi.h>
#include "wifiHandler.h"
#include "debugHandler.h"
#include "configs.h"
#include "flags.h"

// Function to set up WiFi connection
void setupWiFi() {
    debugPrint("[WIFI] Iniciando conexão WiFi...");
    // Check if WiFi credentials are set
    if (ssid == nullptr || password == nullptr) {
        debugPrint("[WIFI] Credenciais WiFi não definidas. Verifique o arquivo configs.h.");
        WIFI_CONNECTED = false;
        WIFI_ERROR = true;
        return;
    }
    // Start WiFi connection
    debugPrint("[WIFI] Conectando ao WiFi SSID: " + String(ssid));
    WiFi.begin(ssid, password);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 5) {
        delay(1000);
        debugPrint("[WIFI] Conectando ao WiFi...");
        attempts++;
        WIFI_CONNECTED = false;
    }

    if (WiFi.status() != WL_CONNECTED) {
        WIFI_CONNECTED = false;
        WIFI_ERROR = true;
        debugPrint("[WIFI] Falha ao conectar. Iniciando AP...");
        WiFi.softAP("ESP32-PID");
        IPAddress IP = WiFi.softAPIP();
        AP_MODE = true;
        debugPrint("[WIFI] AP iniciado com SSID: ESP32-PID");
        debugPrint("[WIFI] IP do AP: " + IP.toString());
    } else {
        debugPrint("[WIFI] Conectado ao WiFi");
        WIFI_CONNECTED = true;
        WIFI_ERROR = false;
        AP_MODE = false;
        debugPrint("[WIFI] SSID: " + String(ssid));
        debugPrint("[WIFI] IP do WiFi: " + WiFi.localIP().toString());
    }
}

void disconnectWiFi() {
    if (WIFI_CONNECTED) {
        WiFi.disconnect();
        WIFI_CONNECTED = false;
        debugPrint("[WIFI] WiFi desconectado");
    } else {
        debugPrint("[WIFI] WiFi já está desconectado");
    }
}

String getWiFiIP() {
    if (WIFI_CONNECTED) {
        IPAddress localIP = WiFi.localIP();
        debugPrint("[WIFI] IP Local: " + localIP.toString());
        // Return the local IP address as a string
        return localIP.toString();
    } else {
        debugPrint("[WIFI] WiFi não conectado, não é possível obter o IP");
    }
}
String getWiFiSSID() {
    if (WIFI_CONNECTED) {
        debugPrint("[WIFI] SSID: " + String(ssid));
        return String(ssid);
    } else {
        debugPrint("[WIFI] WiFi não conectado, não é possível obter o SSID");
    }
}