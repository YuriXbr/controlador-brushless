// Web server handler for ESP32 motor control system
// Provides HTTP API and WebSocket endpoints for system monitoring and control
// Author: Yuri dos Anjos | Version: 2.0 | Date: 2025

#include <WebServer.h>
#include <WebSocketsServer.h>
#include "configs.h"
#include "debugHandler.h"
#include "pageContent.h"
#include "memoryHandler.h"
#include "flags.h"
#include "PIDController.h"
#include "motorController.h"

WebServer server(80);
WebSocketsServer webSocket(81);

// Intelligent cache system
struct ApiCache {
    String data;
    unsigned long lastUpdate;
    bool valid;
} allDataCache = {"", 0, false};

String getPage() {
    return String(PAGE_CONTENT);
}

bool checkWebServerInitialized() {
    if (!WEBSERVER_INITIALIZED) {
        debugPrint("[WEBS] Server not initialized");
        return false;
    }
    return true;
}

bool checkWebServerError() {
    if (WEBSERVER_ERROR) {
        debugPrint("[WEBS] Server in error state");
        return true;
    }
    return false;
}

void invalidateCache() {
    allDataCache.valid = false;
}

// Root page handler
void handleRoot() {
    if (!checkWebServerInitialized()) {
        server.send(500, "text/plain", "Server not initialized");
        return;
    }
    if (checkWebServerError()) {
        server.send(500, "text/plain", "Server error");
        return;
    }
    
    server.sendHeader("Cache-Control", "max-age=3600"); // Cache page for 1 hour
    server.send(200, "text/html", getPage());
}

// PID parameters configuration
void handleSet() {
    if (!checkWebServerInitialized() || checkWebServerError()) {
        server.send(500, "text/plain", "Server error");
        return;
    }
    
    bool paramsChanged = false;
    
    if (server.hasArg("kp")) {
        float newKp = server.arg("kp").toFloat();
        if (newKp >= 0 && newKp <= 10 && newKp != Kp) {
            Kp = newKp;
            writeFloatToEEPROM(0, Kp);
            paramsChanged = true;
        }
    }
    if (server.hasArg("ki")) {
        float newKi = server.arg("ki").toFloat();
        if (newKi >= 0 && newKi <= 5 && newKi != Ki) {
            Ki = newKi;
            writeFloatToEEPROM(4, Ki);
            paramsChanged = true;
        }
    }
    if (server.hasArg("kd")) {
        float newKd = server.arg("kd").toFloat();
        if (newKd >= 0 && newKd <= 10 && newKd != Kd) {
            Kd = newKd;
            writeFloatToEEPROM(8, Kd);
            paramsChanged = true;
        }
    }
    
    if (paramsChanged) {
        setPIDParameters(Kp, Ki, Kd);
        invalidateCache();
        debugPrint("[WEBS] PID updated - Kp=" + String(Kp) + ", Ki=" + String(Ki) + ", Kd=" + String(Kd));
    }
    
    server.send(200, "text/html", getPage());
}

// Manual mode toggle
void handleToggleManual() {
    if (!checkWebServerInitialized() || checkWebServerError()) {
        server.send(500, "text/plain", "Server error");
        return;
    }
    
    bool previousMode = manualMode;
    manualMode = !manualMode;
    
    // Reset PID when switching modes for stability
    if (manualMode != previousMode) {
        resetPID();
        invalidateCache();
        debugPrint("[WEBS] Manual mode: " + String(manualMode ? "ON" : "OFF"));
    }
    
    server.send(200, "text/plain", "ok");
}

// Manual PWM setting
void handleSetPWM() {
    if (!checkWebServerInitialized() || checkWebServerError()) {
        server.send(500, "text/plain", "Server error");
        return;
    }
    
    if (!manualMode) {
        server.send(400, "text/plain", "Manual mode not active");
        return;
    }
    
    if (server.hasArg("value")) {
        int val = server.arg("value").toInt();
        int newPWM = constrain(val, 1000, 2000);
        
        // Validate PWM range more strictly
        if (val < 1000 || val > 2000) {
            server.send(400, "text/plain", "PWM value out of range (1000-2000)");
            return;
        }
        
        if (newPWM != manualPWM) {
            manualPWM = newPWM;
            invalidateCache();
            debugPrint("[WEBS] Manual PWM set to: " + String(manualPWM));
        }
        server.send(200, "text/plain", "PWM set to " + String(manualPWM));
    } else {
        server.send(400, "text/plain", "Missing PWM value parameter");
    }
}

// Manual state query
void handleManualState() {
    if (!checkWebServerInitialized() || checkWebServerError()) {
        server.send(500, "text/plain", "Server error");
        return;
    }
    
    server.sendHeader("Cache-Control", "max-age=5");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", manualMode ? "Ativado" : "Desligado");
}

// DEPRECATED: Legacy data endpoint
void handleData() {
    if (!checkWebServerInitialized() || checkWebServerError()) {
        server.send(500, "application/json", "{\"error\":\"Server error\"}");
        return;
    }
    
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Cache-Control", "max-age=1");
    server.sendHeader("X-Deprecated", "true");
    
    String json = "{\"angle\":" + String(currentAngle, 2) + 
                  ",\"pwm\":" + String(currentPWM) + 
                  ",\"pid\":" + String(currentPIDOutput) + 
                  ",\"timestamp\":" + String(millis()) + "}";
    server.send(200, "application/json", json);
}

// Complete system data with intelligent caching
void handleAllData() {
    if (!checkWebServerInitialized() || checkWebServerError()) {
        server.send(500, "application/json", "{\"error\":\"Server error\"}");
        return;
    }
    
    unsigned long currentTime = millis();
    
    // Update cache if invalid, stale (>100ms), or on first run
    if (!allDataCache.valid || (currentTime - allDataCache.lastUpdate > 100) || allDataCache.data.length() == 0) {
        
        // Clear and pre-allocate memory for better performance
        allDataCache.data = "";
        allDataCache.data.reserve(700);
        allDataCache.data = "{";
        
        // PID data with error handling
        allDataCache.data += "\"pid\":{";
        allDataCache.data += "\"kp\":" + String(isnan(Kp) ? 0.0 : Kp, 3) + ",";
        allDataCache.data += "\"ki\":" + String(isnan(Ki) ? 0.0 : Ki, 3) + ",";
        allDataCache.data += "\"kd\":" + String(isnan(Kd) ? 0.0 : Kd, 3) + ",";
        allDataCache.data += "\"setpoint\":" + String(isnan(setpoint) ? 0.0 : setpoint, 2) + ",";
        allDataCache.data += "\"output\":" + String(currentPIDOutput) + ",";
        allDataCache.data += "\"initialized\":" + String(PID_INITIALIZED ? "true" : "false");
        allDataCache.data += "},";
        
        // Sensor data
        allDataCache.data += "\"sensor\":{";
        allDataCache.data += "\"angle\":" + String(isnan(currentAngle) ? 0.0 : currentAngle, 2) + ",";
        allDataCache.data += "\"error\":" + String(MPU_ERROR ? "true" : "false");
        allDataCache.data += "},";
        
        // Motor data
        allDataCache.data += "\"motor\":{";
        allDataCache.data += "\"pwm\":" + String(currentPWM) + ",";
        allDataCache.data += "\"running\":" + String(MOTOR_RUNNING ? "true" : "false") + ",";
        allDataCache.data += "\"manual_mode\":" + String(manualMode ? "true" : "false") + ",";
        allDataCache.data += "\"manual_pwm\":" + String(manualPWM) + ",";
        allDataCache.data += "\"stopped\":" + String(MOTOR_STOPPED ? "true" : "false");
        allDataCache.data += "},";
        
        // System timing
        allDataCache.data += "\"timing\":{";
        allDataCache.data += "\"loop_freq_hz\":" + String(1000000.0 / LOOP_TIME, 1) + ",";
        allDataCache.data += "\"pid_freq_hz\":" + String(1000000.0 / PID_SAMPLE_TIME, 1) + ",";
        allDataCache.data += "\"debug_interval\":" + String(debugInterval);
        allDataCache.data += "},";
        
        // System status with comprehensive error checking
        allDataCache.data += "\"system\":{";
        allDataCache.data += "\"uptime\":" + String(currentTime) + ",";
        allDataCache.data += "\"wifi_connected\":" + String(!WIFI_ERROR ? "true" : "false") + ",";
        allDataCache.data += "\"ap_mode\":" + String(AP_MODE ? "true" : "false") + ",";
        allDataCache.data += "\"free_heap\":" + String(ESP.getFreeHeap()) + ",";
        allDataCache.data += "\"errors\":{";
        allDataCache.data += "\"eeprom\":" + String(EEPROM_ERROR ? "true" : "false") + ",";
        allDataCache.data += "\"mpu\":" + String(MPU_ERROR ? "true" : "false") + ",";
        allDataCache.data += "\"motor\":" + String(MOTOR_ERROR ? "true" : "false") + ",";
        allDataCache.data += "\"pid\":" + String(PID_ERROR ? "true" : "false") + ",";
        allDataCache.data += "\"wifi\":" + String(WIFI_ERROR ? "true" : "false") + ",";
        allDataCache.data += "\"webserver\":" + String(WEBSERVER_ERROR ? "true" : "false");
        allDataCache.data += "},";
        allDataCache.data += "\"emergency_stop\":" + String(EMERGENCY_STOP ? "true" : "false");
        allDataCache.data += "},";
        
        allDataCache.data += "\"timestamp\":" + String(currentTime) + "}";
        
        allDataCache.lastUpdate = currentTime;
        allDataCache.valid = true;
    }
    
    server.sendHeader("Cache-Control", "max-age=1");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Content-Type", "application/json");
    server.send(200, "application/json", allDataCache.data);
}

// Condensed system status
void handleSystemStatus() {
    if (!checkWebServerInitialized() || checkWebServerError()) {
        server.send(500, "application/json", "{\"error\":\"Server error\"}");
        return;
    }
    
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Cache-Control", "max-age=5");
    
    bool hasError = (EEPROM_ERROR || MPU_ERROR || MOTOR_ERROR || PID_ERROR || WIFI_ERROR || WEBSERVER_ERROR);
    
    String json = "{";
    json += "\"status\":\"" + String(hasError ? "error" : "ok") + "\",";
    json += "\"motor_running\":" + String(MOTOR_RUNNING ? "true" : "false") + ",";
    json += "\"manual_mode\":" + String(manualMode ? "true" : "false") + ",";
    json += "\"angle\":" + String(currentAngle, 2) + ",";
    json += "\"pwm\":" + String(currentPWM) + ",";
    json += "\"free_heap\":" + String(ESP.getFreeHeap()) + ",";
    json += "\"timestamp\":" + String(millis()) + "}";
    
    server.send(200, "application/json", json);
}

// Motor control with smooth ramps
void handleToggleMotor() {
    if (!checkWebServerInitialized() || checkWebServerError()) {
        server.send(500, "text/plain", "Server error");
        return;
    }
    
    // Safety check - don't start motor if there are critical errors
    if (!MOTOR_RUNNING && (MPU_ERROR || PID_ERROR || MOTOR_ERROR)) {
        server.send(400, "text/plain", "Cannot start motor - system errors detected");
        debugPrint("[WEBS] Motor start blocked due to system errors");
        return;
    }
    
    if (MOTOR_RUNNING) {
        stopMotor();
        debugPrint("[WEBS] Stopping motor with smooth ramp");
        server.send(200, "text/plain", "Desligado"); // Motor will be stopped
    } else {
        resetPID();
        startMotor();
        debugPrint("[WEBS] Starting motor with smooth ramp");
        server.send(200, "text/plain", "Ligado"); // Motor will be started
    }
    
    invalidateCache();
}

// Motor state query
void handleMotorState() {
    if (!checkWebServerInitialized() || checkWebServerError()) {
        server.send(500, "text/plain", "Server error");
        return;
    }
    
    server.sendHeader("Cache-Control", "max-age=2");
    server.send(200, "text/plain", MOTOR_RUNNING ? "Ligado" : "Desligado");
}

// Setpoint configuration via GET: /setSetpoint?value=XX
void handleSetSetpoint() {
    if (!checkWebServerInitialized() || checkWebServerError()) {
        server.send(500, "text/plain", "Server error");
        return;
    }
    
    if (server.hasArg("value")) {
        float newSetpoint = server.arg("value").toFloat();
        if (newSetpoint >= -180 && newSetpoint <= 180) {
            setSetpoint(newSetpoint);
            invalidateCache();
            server.send(200, "text/plain", "Setpoint updated to " + String(newSetpoint));
            debugPrint("[WEBS] Setpoint changed: " + String(newSetpoint));
        } else {
            server.send(400, "text/plain", "Invalid setpoint range (-180 to 180)");
        }
    } else {
        server.send(400, "text/plain", "Missing 'value' parameter");
    }
}

// Get current PID values for form initialization
void handleGetPIDValues() {
    if (!checkWebServerInitialized() || checkWebServerError()) {
        server.send(500, "application/json", "{\"error\":\"Server error\"}");
        return;
    }
    
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Cache-Control", "max-age=10");
    
    String json = "{";
    json += "\"kp\":" + String(isnan(Kp) ? 0.0 : Kp, 3) + ",";
    json += "\"ki\":" + String(isnan(Ki) ? 0.0 : Ki, 3) + ",";
    json += "\"kd\":" + String(isnan(Kd) ? 0.0 : Kd, 3) + ",";
    json += "\"setpoint\":" + String(isnan(setpoint) ? 0.0 : setpoint, 2);
    json += "}";
    
    server.send(200, "application/json", json);
}

// PID-specific data endpoint
void handlePIDData() {
    if (!checkWebServerInitialized() || checkWebServerError()) {
        server.send(500, "application/json", "{\"error\":\"Server error\"}");
        return;
    }
    
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Cache-Control", "max-age=2");
    
    String json = "{";
    json += "\"kp\":" + String(Kp, 3) + ",";
    json += "\"ki\":" + String(Ki, 3) + ",";
    json += "\"kd\":" + String(Kd, 3) + ",";
    json += "\"setpoint\":" + String(setpoint, 2) + ",";
    json += "\"output\":" + String(currentPIDOutput) + ",";
    json += "\"angle\":" + String(currentAngle, 2) + ",";
    json += "\"initialized\":" + String(PID_INITIALIZED ? "true" : "false") + ",";
    json += "\"timestamp\":" + String(millis()) + "}";
    
    server.send(200, "application/json", json);
}

// Motor-specific data endpoint
void handleMotorData() {
    if (!checkWebServerInitialized() || checkWebServerError()) {
        server.send(500, "application/json", "{\"error\":\"Server error\"}");
        return;
    }
    
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Cache-Control", "max-age=1");
    
    String json = "{";
    json += "\"pwm\":" + String(currentPWM) + ",";
    json += "\"running\":" + String(MOTOR_RUNNING ? "true" : "false") + ",";
    json += "\"manual_mode\":" + String(manualMode ? "true" : "false") + ",";
    json += "\"manual_pwm\":" + String(manualPWM) + ",";
    json += "\"stopped\":" + String(MOTOR_STOPPED ? "true" : "false") + ",";
    json += "\"timestamp\":" + String(millis()) + "}";
    
    server.send(200, "application/json", json);
}

// Emergency stop endpoint
void handleEmergencyStop() {
    debugPrint("[WEBS] Emergency stop triggered via web interface");
    motorEmergencyStop();
    invalidateCache();
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", "Emergency stop activated");
}

// Reset PID integral term endpoint
void handleResetPIDIntegral() {
    if (!checkWebServerInitialized() || checkWebServerError()) {
        server.send(500, "text/plain", "Server error");
        return;
    }
    
    resetPIDIntegral();
    invalidateCache();
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", "PID integral term reset");
    debugPrint("[WEBS] PID integral reset via web interface");
}

// Complete PID reset endpoint
void handleResetPID() {
    if (!checkWebServerInitialized() || checkWebServerError()) {
        server.send(500, "text/plain", "Server error");
        return;
    }
    
    resetPID();
    invalidateCache();
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", "Complete PID reset performed");
    debugPrint("[WEBS] Complete PID reset via web interface");
}

// Cache optimization function
void optimizeCache() {
    // Clear cache if memory is getting low
    if (ESP.getFreeHeap() < 10000) { // Less than 10KB free
        allDataCache.valid = false;
        allDataCache.data = "";
        debugPrint("[WEBS] Cache cleared due to low memory");
    }
}

// Process client requests and WebSocket messages
void handleClientRequests() {
    if (!checkWebServerInitialized() || checkWebServerError()) {
        return;
    }
    
    // Optimize cache periodically
    static unsigned long lastCacheOptimization = 0;
    if (millis() - lastCacheOptimization > 30000) { // Every 30 seconds
        optimizeCache();
        lastCacheOptimization = millis();
    }
    
    server.handleClient();
    webSocket.loop();
}

// Initialize web server with all routes
void setupWebServer() {
    if(WIFI_ERROR && !AP_MODE) {
        debugPrint("[WEBS] No WiFi connection and not in AP mode");
        WEBSERVER_INITIALIZED = false;
        WEBSOCKET_INITIALIZED = false;
        WEBSERVER_ERROR = true;
        WEBSOCKET_ERROR = true;
        return;
    }

    debugPrint("[WEBS] Starting web server on port 80");
    
    // Configure all routes
    server.on("/", handleRoot);
    server.on("/set", handleSet);
    server.on("/toggleManual", handleToggleManual);
    server.on("/setPWM", handleSetPWM);
    server.on("/manualState", handleManualState);
    server.on("/data", handleData); // DEPRECATED - use /allData
    server.on("/allData", handleAllData);
    server.on("/systemStatus", handleSystemStatus);
    server.on("/pidData", handlePIDData);
    server.on("/getPIDValues", handleGetPIDValues); // Get current PID values
    server.on("/motorData", handleMotorData);
    server.on("/toggleMotor", handleToggleMotor);
    server.on("/motorState", handleMotorState);
    server.on("/setSetpoint", handleSetSetpoint);
    server.on("/emergencyStop", handleEmergencyStop); // Emergency stop endpoint
    server.on("/resetPIDIntegral", handleResetPIDIntegral); // Reset integral term only
    server.on("/resetPID", handleResetPID); // Complete PID reset
    server.on("/resetPIDIntegral", handleResetPIDIntegral); // Reset PID integral term
    server.on("/resetPID", handleResetPID); // Complete PID reset
    
    server.onNotFound([]() {
        server.send(404, "text/plain", "Not Found");
    });
    
    server.begin();
    webSocket.begin();
    
    debugPrint("[WEBS] Web server started on port 80");
    debugPrint("[WEBS] WebSocket started on port 81");
    
    WEBSERVER_INITIALIZED = true;
    WEBSOCKET_INITIALIZED = true;
}