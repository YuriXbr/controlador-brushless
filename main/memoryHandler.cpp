#include <EEPROM.h>
#include "memoryHandler.h"
#include "configs.h"
#include "debugHandler.h"
#include "flags.h"


void initMemory() {
    // Initialize EEPROM with the defined size
    if (EEPROM.begin(EEPROM_SIZE) == false) {
        debugPrint("[EEPROM] Failed to initialize EEPROM");
        EEPROM_INITIALIZED = false;
        return; // Exit if EEPROM initialization fails
    }
    debugPrint("[EEPROM] EEPROM initialized with size: " + String(EEPROM_SIZE));
    EEPROM_INITIALIZED = true; 
}

float readFloatFromEEPROM(int address) {
    // Check if the address is within valid bounds
    if (address < 0 || address + sizeof(float) > EEPROM_SIZE) {
        debugPrint("[EEPROM] Invalid EEPROM address: " + String(address));
        return 0.0f; // Return a default value if address is out of bounds
    }
    // Ensure the EEPROM is initialized and ready
    if (!EEPROM_INITIALIZED) {
        debugPrint("[EEPROM] EEPROM not initialized, cannot read float");
        return 0.0f; // Return a default value if EEPROM is not initialized
    }
    // Read the float value from the specified address
    debugPrint("[EEPROM] Reading float from EEPROM at address " + String(address));
    float value;
    if (EEPROM.get(address, value) == false) {
        debugPrint("[EEPROM] Failed to read float from EEPROM at address " + String(address));
        return 0.0f; // Return a default value if reading fails
    }
    debugPrint("[EEPROM] Read float from EEPROM at address " + String(address) + ": " + String(value));
    return value;
}

void writeFloatToEEPROM(int address, float value) {
    if (address < 0 || address + sizeof(float) > EEPROM_SIZE) {
        debugPrint("Invalid EEPROM address: " + String(address));
        return; // Address out of bounds
    }
    debugPrint("Writing float to EEPROM at address " + String(address) + ": " + String(value));
    // Ensure the EEPROM is initialized and ready
    if (!EEPROM_INITIALIZED) {
        debugPrint("EEPROM not initialized, cannot write float");
        return; // Cannot write if EEPROM is not initialized
    }
    // Write the float value to the specified address
    EEPROM.put(address, value);
    // Commit the changes to EEPROM
    if (!EEPROM.commit()) {
        debugPrint("Failed to commit EEPROM changes");
        return; // Commit failed
    }
}