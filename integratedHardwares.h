#ifndef INTEGRATED_HARDWARES_H
#define INTEGRATED_HARDWARES_H

#include <Arduino.h>

void initIntegratedHardwares();
bool readButtonState(int buttonPin);
void updateDisplay(const String& message);

#define MOTOR_BUTTON_PIN  4 
void handleMotorButton();

#endif