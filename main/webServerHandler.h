#ifndef WEBSERVERHANDLER_H
#define WEBSERVERHANDLER_H

#include <WebServer.h>

// Web server setup and control functions
void setupWebServer();
void handleClientRequests();
void invalidateCache();

// HTTP endpoint handlers
void handleRoot();
void handleSet();
void handleToggleManual();
void handleSetPWM();
void handleManualState();
void handleData(); // DEPRECATED: Use handleAllData() for better performance
void handleAllData();
void handleSystemStatus();
void handlePIDData();
void handleGetPIDValues();
void handleMotorData();
void handleToggleMotor();
void handleMotorState();
void handleSetSetpoint();
void handleEmergencyStop();

// Utility functions
bool checkWebServerInitialized();
bool checkWebServerError();
void optimizeCache();
String getPage();

#endif // WEBSERVERHANDLER_H