#include "configs.h"

// Wi-Fi credentials
const char* ssid = "VIVOFIBRA-4C58";
const char* password = "84EED35225";

// EEPROM size
int EEPROM_SIZE = 64; // Define the size of the EEPROM

// PID controller initial values
float Kp = 0.0;
float Ki = 0.0;
float Kd = 0.0;
float defaultKp = 0.5;
float defaultKi = 0.6;
float defaultKd = 0.55;

// Setpoint
float setpoint = 0.0;
float defaultSetpoint = 0.0;

// ESC PWM configuration
const int escPin = 18;
const int pwmChannel = 0;
const int freq = 50;
const int resolution = 16;
bool manualMode = false;
int manualPWM = 1000;

int currentPIDOutput = 0;
int currentPWM = 1000;
float currentAngle = 0.0;

// Debug - reduzido para não impactar performance
bool DEBUG = 1;
unsigned long debugInterval = 2000; // Aumentado para 2 segundos para reduzir overhead

// PID timing settings (em microssegundos para máxima frequência)
const unsigned long PID_SAMPLE_TIME = 2000;  // 2ms = 500Hz - frequência máxima para PID
const unsigned long LOOP_TIME = 1000;        // 1ms = 1kHz - frequência máxima do loop principal

// WebSocket real-time data settings
const unsigned long WEBSOCKET_REALTIME_INTERVAL = 50; // 50ms = 20Hz para dados em tempo real
