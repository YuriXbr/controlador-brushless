#include "debugHandler.h"
#include <vector>
#include <WebSocketsServer.h>
#include "configs.h"
#include "flags.h"

extern WebSocketsServer webSocket;

// Optimized debug buffer with size limits
std::vector<String> debugBuffer;
unsigned long lastDebugSend = 0;
int repeatedCount1 = 1, repeatedCount2 = 1;
String lastMsg1 = "", lastMsg2 = "";

void debugPrint(String msg) {
    if (!DEBUG) return;
    
    // Truncate message if too long
    if (msg.length() > DEBUG_MESSAGE_MAX_LENGTH) {
        msg = msg.substring(0, DEBUG_MESSAGE_MAX_LENGTH - 3) + "...";
    }
    
    // Handle message deduplication
    if (!debugBuffer.empty() && (msg == lastMsg1 || msg == ("[" + String(repeatedCount1) + "] " + lastMsg1))) {
        repeatedCount1++;
        debugBuffer.pop_back();
        debugBuffer.push_back("[" + String(repeatedCount1) + "] " + lastMsg1);
    }
    else if (debugBuffer.size() > 1 && (msg == lastMsg2 || msg == ("[" + String(repeatedCount2) + "] " + lastMsg2))) {
        repeatedCount2++;
        debugBuffer.pop_back();
        debugBuffer.pop_back();
        debugBuffer.push_back("[" + String(repeatedCount2) + "] " + lastMsg2);
        lastMsg1 = lastMsg2;
        repeatedCount1 = repeatedCount2;
        lastMsg2 = "";
        repeatedCount2 = 1;
    } else {
        lastMsg2 = lastMsg1;
        repeatedCount2 = repeatedCount1;
        lastMsg1 = msg;
        repeatedCount1 = 1;
        debugBuffer.push_back(msg);
    }
    
    //Serial.println(msg);
    
    // Efficient buffer size management
    if (debugBuffer.size() > DEBUG_BUFFER_MAX_SIZE) {
        // Remove oldest 10 messages for better performance
        int removeCount = min(10, (int)debugBuffer.size() - DEBUG_BUFFER_MAX_SIZE + 10);
        debugBuffer.erase(debugBuffer.begin(), debugBuffer.begin() + removeCount);
    }
}

void sendDebugBuffer() {
    if (millis() - lastDebugSend >= debugInterval && !debugBuffer.empty()) {
        String combined;
        combined.reserve(debugBuffer.size() * 50); // Pre-allocate memory
        
        for (const auto& line : debugBuffer) {
            combined += line + "\n";
        }
        
        webSocket.broadcastTXT(combined);
        clearDebugBuffer();
        lastDebugSend = millis();
    }
}

void clearDebugBuffer() {
    debugBuffer.clear();
    debugBuffer.shrink_to_fit(); // Release unused memory
    lastMsg1 = "";
    lastMsg2 = "";
    repeatedCount1 = 1;
    repeatedCount2 = 1;
}

size_t getDebugBufferSize() {
    return debugBuffer.size();
}

// Send real-time data via WebSocket in JSON format
void sendRealtimeData(float angle, int pwm) {
    if (!WEBSOCKET_INITIALIZED || !SomeoneIsConnected) return;
    
    // Create JSON data
    String jsonData = "{";
    jsonData += "\"angle\":" + String(angle, 2) + ",";
    jsonData += "\"pwm\":" + String(pwm) + ",";
    jsonData += "\"timestamp\":" + String(millis());
    jsonData += "}";
    
    webSocket.broadcastTXT(jsonData);
}

// Send real-time data with full system status
void sendRealtimeDataFull(float angle, int pwm, float setpoint, float pidOutput, bool motorRunning) {
    if (!WEBSOCKET_INITIALIZED || !SomeoneIsConnected) return;
    
    String jsonData = "{";
    jsonData += "\"angle\":" + String(angle, 2) + ",";
    jsonData += "\"pwm\":" + String(pwm) + ",";
    jsonData += "\"setpoint\":" + String(setpoint, 2) + ",";
    jsonData += "\"pid_output\":" + String(pidOutput, 2) + ",";
    jsonData += "\"motor_running\":" + String(motorRunning ? "true" : "false") + ",";
    jsonData += "\"timestamp\":" + String(millis());
    jsonData += "}";
    
    webSocket.broadcastTXT(jsonData);
}