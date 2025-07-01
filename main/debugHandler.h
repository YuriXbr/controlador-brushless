#ifndef DEBUGHANDLER_H
#define DEBUGHANDLER_H

#include <Arduino.h>
#include <vector>

// Debug buffer configuration
#define DEBUG_BUFFER_MAX_SIZE 50
#define DEBUG_MESSAGE_MAX_LENGTH 200

// Debug functions
void debugPrint(String msg);
void sendDebugBuffer();
void clearDebugBuffer();
size_t getDebugBufferSize();

// Real-time WebSocket data functions
void sendRealtimeData(float angle, int pwm);
void sendRealtimeDataFull(float angle, int pwm, float setpoint, float pidOutput, bool motorRunning);

#endif // DEBUGHANDLER_H