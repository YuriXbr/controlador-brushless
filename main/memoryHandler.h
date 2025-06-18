#ifndef MEMORYHANDLER_H
#define MEMORYHANDLER_H

#include <EEPROM.h>

// Funções para ler e escrever valores de ponto flutuante na EEPROM
void initMemory();
float readFloatFromEEPROM(int address);
void writeFloatToEEPROM(int address, float value);

#endif // MEMORYHANDLER_H