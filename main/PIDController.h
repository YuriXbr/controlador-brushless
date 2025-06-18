#ifndef PIDCONTROLLER_H
#define PIDCONTROLLER_H

#include <Arduino.h>

void setupPIDController();
void setPIDParameters(float newKp, float newKi, float newKd);
int computePID(float angle);
void setSetpoint(float newSetpoint);

#endif