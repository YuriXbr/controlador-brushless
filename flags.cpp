#include "flags.h"

// Initialization flags
bool EEPROM_INITIALIZED = false; // Flag to check if EEPROM is initialized
bool PID_INITIALIZED = false; // Flag to check if PID controller is initialized
bool WIFI_CONNECTED = false; // Flag to check if WiFi is connected
bool AP_MODE = false; // Flag to check if the device is in Access Point mode
bool MPU_INITIALIZED = false; // Flag to check if MPU6050 is initialized
bool WEBSERVER_INITIALIZED = false; // Flag to check if the web server is initialized
bool WEBSOCKET_INITIALIZED = false; // Flag to check if WebSocket is initialized
bool INTEGRATED_HARDWARES_INITIALIZED = false; // Flag to check if integrated hardwares are initialized


// Error flags
bool MOTOR_ERROR = false; // Flag to indicate an error with the motor
bool EEPROM_ERROR = false; // Flag to indicate an error with EEPROM
bool PID_ERROR = false; // Flag to indicate an error with PID controller
bool WIFI_ERROR = false; // Flag to indicate an error with WiFi connection
bool AP_ERROR = false; // Flag to indicate an error with Access Point mode
bool MPU_ERROR = false; // Flag to indicate an error with MPU6050 initialization
bool PWM_ERROR = false; // Flag to indicate an error with PWM configuration
bool WEBSERVER_ERROR = false; // Flag to indicate an error with the web server
bool WEBSOCKET_ERROR = false; // Flag to indicate an error with WebSocket connection

// Status flags
bool MOTOR_READY = false; // Flag to indicate if the motor is ready
bool MOTOR_RUNNING = false; // Flag to indicate if the motor is currently running
bool MOTOR_STOPPED = false; // Flag to indicate if the motor is stopped

bool THROTTLE_ACTIVE = false; // Flag to indicate if the throttle is active
bool PID_ACTIVE = false; // Flag to indicate if the PID controller is active
bool ESC_INITIALIZED = false; // Flag to check if the ESC is initialized
bool ESC_ERROR = false; // Flag to indicate an error with the ESC

// Emergency flags
bool EMERGENCY_STOP = false; // Flag to indicate if an emergency stop is triggered
bool MANUAL_EMERGENCY_MODE = false; // Flag to indicate if the device is in emergency mode
bool AUTOMATIC_EMERGENCY_MODE = false; // Flag to indicate if the device is in automatic emergency mode

//WebSocket flags
bool WEBSOCKET_CONNECTED = false; // Flag to indicate if WebSocket is connected
bool SomeoneIsConnected = false; // Flag to indicate if someone is connected to the WebSocket