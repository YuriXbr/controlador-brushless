#ifndef MOTORCONTROLLER_H
#define MOTORCONTROLLER_H

#include <Arduino.h>

// Novas declarações de funções
void setupMotorController();
void setMotorPWM(int pwmValue);
void motorEmergencyStop();
uint32_t pulseWidthToDuty(float pulse_us);
void rampaDePartidaAteControle(int tempoMs);
void rampaDeDesligamento(int tempoMs);

#endif // MOTORCONTROLLER_H