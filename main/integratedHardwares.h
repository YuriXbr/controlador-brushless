#ifndef INTEGRATED_HARDWARES_H
#define INTEGRATED_HARDWARES_H

#include <Arduino.h>

// Funções para gerenciar hardwares integrados
void initIntegratedHardwares();
bool readButtonState(int buttonPin);
void updateDisplay(const String& message);

#endif // INTEGRATED_HARDWARES_H