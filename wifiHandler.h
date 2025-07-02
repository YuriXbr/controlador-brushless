#ifndef WIFIHANDLER_H
#define WIFIHANDLER_H

#include <WiFi.h>

void setupWiFi();
void startAccessPoint(const char* apName);
String getWiFiIP();
void disconnectWiFi();
bool isWiFiConnected();

#endif // WIFIHANDLER_H