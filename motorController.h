#ifndef MOTORCONTROLLER_H
#define MOTORCONTROLLER_H

#include <Arduino.h>

// Motor controller functions
void setupMotorController();
void setMotorPWM(int pwmValue);
void motorEmergencyStop();
void startMotor();
void stopMotor();
uint32_t pulseWidthToDuty(float pulse_us);

#endif // MOTORCONTROLLER_H