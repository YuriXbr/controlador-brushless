#ifndef PIDCONTROLLER_H
#define PIDCONTROLLER_H

#include <Arduino.h>

// PWM mínimo de sustentação para o braço
#define PWM_SUSTENTACAO 1100

void setupPIDController();
void setPIDParameters(float newKp, float newKi, float newKd);
int computePID(float angle);
void setSetpoint(float newSetpoint);
void resetPID();

#endif