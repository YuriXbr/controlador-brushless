#ifndef MOTORCONTROLLER_H
#define MOTORCONTROLLER_H

#include <Arduino.h>

// Novas declarações de funções
void setupMotorController();
void setMotorPWM(int pwmValue);
void motorEmergencyStop();
uint32_t pulseWidthToDuty(float pulse_us);

#endif // MOTORCONTROLLER_H