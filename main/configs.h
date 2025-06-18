#ifndef CONFIGS_H
#define CONFIGS_H

// Wi-Fi credentials
extern const char* ssid;
extern const char* password;

// EEPROM size
extern int EEPROM_SIZE;

// PID controller initial values
extern float Kp;
extern float Ki;
extern float Kd;
extern float defaultKp;
extern float defaultKi;
extern float defaultKd;

// Setpoint for PID controller
extern float setpoint;
extern float defaultSetpoint;

// ESC PWM configuration
extern const int escPin;
extern const int pwmChannel;
extern const int freq;
extern const int resolution;
extern bool manualMode;
extern int manualPWM;

extern int currentPIDOutput;
extern int currentPWM;
extern float currentAngle;

// Debugging settings
extern bool DEBUG;
extern unsigned long debugInterval;

#endif // CONFIGS_H
