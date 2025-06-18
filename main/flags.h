#ifndef FLAGS_H
#define FLAGS_H

// Initialization flags
extern bool EEPROM_INITIALIZED;
extern bool PID_INITIALIZED;
extern bool WIFI_CONNECTED;
extern bool AP_MODE;
extern bool MPU_INITIALIZED;
extern bool WEBSERVER_INITIALIZED;
extern bool WEBSOCKET_INITIALIZED;
extern bool INTEGRATED_HARDWARES_INITIALIZED;


// Error flags
extern bool MOTOR_ERROR;
extern bool EEPROM_ERROR;
extern bool PID_ERROR;
extern bool WIFI_ERROR;
extern bool AP_ERROR;
extern bool MPU_ERROR;
extern bool PWM_ERROR;
extern bool WEBSERVER_ERROR;
extern bool WEBSOCKET_ERROR;

// Status flags
extern bool MOTOR_READY;
extern bool MOTOR_RUNNING;
extern bool MOTOR_STOPPED;

extern bool THROTTLE_ACTIVE;
extern bool PID_ACTIVE;
extern bool ESC_INITIALIZED;
extern bool ESC_ERROR;

// Emergency flags
extern bool EMERGENCY_STOP;
extern bool MANUAL_EMERGENCY_MODE;
extern bool AUTOMATIC_EMERGENCY_MODE;

//WebSocket flags
extern bool WEBSOCKET_CONNECTED;
extern bool SomeoneIsConnected;




#endif // FLAGS_H