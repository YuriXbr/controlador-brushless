// ESP32 Motor Control System with PID Controller
// Features: WiFi connectivity, web interface, gyroscope feedback, motor control
// Author: Yuri dos Anjos | Version: 2.0

#include <Wire.h>
#include <EEPROM.h>
#include "memoryHandler.h"
#include "wifiHandler.h"
#include "webServerHandler.h"
#include "gyroController.h"
#include "motorController.h"
#include "PIDController.h"
#include "integratedHardwares.h"
#include "debugHandler.h"
#include "configs.h"
#include "flags.h"
#include "memoryHandler.h"
#include "dump.h"

// Timing control variables
static unsigned long lastDebugSend = 0;
static unsigned long lastPIDUpdate = 0;
static unsigned long lastLoopTime = 0;
static unsigned long lastWebSocketSend = 0;

void setup() {
    Wire.begin();
    Serial.begin(115200);
    
    // Initialize system components
    setupWiFi();
    initMemory();
    setupWebServer();
    Serial.println("Starting Motor Control System...");
    initGyro();
    setupMotorController();
    setupPIDController();
    initIntegratedHardwares();

    dumpAll();
    
    // Check for initialization errors
    if(EEPROM_ERROR || MPU_ERROR || MOTOR_ERROR || PID_ERROR || WIFI_ERROR || WEBSERVER_ERROR || WEBSOCKET_ERROR) {
        debugPrint("[SETUP] Errors found during initialization");
        sendDebugBuffer();
        while (true) {
            delay(1000);
        }
    } else {
        debugPrint("[SETUP] System initialization complete");
        sendDebugBuffer();
    }
    
    // Initialize timing variables
    lastPIDUpdate = micros();
    lastLoopTime = micros();
    lastDebugSend = millis();
    lastWebSocketSend = millis();
}

void loop() {
    unsigned long currentTime = micros();
    
    // Main loop frequency control
    if (currentTime - lastLoopTime >= LOOP_TIME) {
        lastLoopTime = currentTime;
        
        handleMotorButton();
        handleClientRequests();
        
        // Update motor ramps (non-blocking)
        updateMotorRamp();
        
        if (MOTOR_RUNNING) {
            if (manualMode) {
                // Manual mode: bypass PID, use manual PWM
                setMotorPWM(manualPWM);
                currentPWM = manualPWM;
            } else {
                // PID frequency control
                if (currentTime - lastPIDUpdate >= PID_SAMPLE_TIME) {
                    lastPIDUpdate = currentTime;
                    
                    updatePIDTiming();
                    currentAngle = readGyroAngle();
                    currentPWM = computePID(currentAngle);
                    setMotorPWM(currentPWM);
                    sendDebugBuffer();
                }
            }
        } else {
            setMotorPWM(1000);
            currentPWM = 1000;
            Serial.println("[LOOP] Motor stopped, setting PWM to 1000");
        }

        // Periodic debug buffer transmission
        if (millis() - lastDebugSend > debugInterval) {
            sendDebugBuffer();
            lastDebugSend = millis();
        }
        
        // Real-time WebSocket data transmission (higher frequency)
        if (WEBSOCKET_CONNECTED && SomeoneIsConnected && (millis() - lastWebSocketSend > WEBSOCKET_REALTIME_INTERVAL)) {
            sendRealtimeDataFull(currentAngle, currentPWM, setpoint, currentPIDOutput, MOTOR_RUNNING);
            lastWebSocketSend = millis();
        }
        
        // Error handling with emergency stop
        if (MOTOR_ERROR || PID_ERROR || MPU_ERROR) {
            debugPrint("[LOOP] Critical error detected - emergency stop");
            motorEmergencyStop();
            emergencyDump(currentAngle, currentPWM, currentPIDOutput);
            sendDebugBuffer();
            
            // Non-blocking error state - continue processing requests
            static unsigned long lastErrorProcess = 0;
            while (true) {
                unsigned long currentErrorTime = millis();
                if (currentErrorTime - lastErrorProcess >= 1000) {
                    lastErrorProcess = currentErrorTime;
                    handleClientRequests();
                }
            }
        }
    }
}