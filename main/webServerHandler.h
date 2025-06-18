#ifndef WEBSERVERHANDLER_H
#define WEBSERVERHANDLER_H

#include <WebServer.h>

void setupWebServer();
void handleClientRequests();
void handleRoot();
void handleSet();
void handleToggleManual();
void handleSetPWM();
void handleManualState();
void handleData();
void checkWebServerInitialized();
String getPage();

#endif // WEBSERVERHANDLER_H