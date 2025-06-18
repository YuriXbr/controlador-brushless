#include "dump.h"
#include "memoryHandler.h"
#include "configs.h"
#include "debugHandler.h"
#include "flags.h"
#include "wifiHandler.h"

void dumpAll() {
    // Dump all from eeprom, flags, and configs
    // It will read all the bytes and print them to the Serial Monitor
    debugPrint("=========================================");
    debugPrint("Dumping data from EEPROM, flags, and configs...");
    debugPrint("This will take a while, please wait...");
    debugPrint("=========================================");
    debugPrint("Dumping all data from EEPROM, flags, and configs...");
    debugPrint("EEPROM Size: " + String(EEPROM_SIZE) + " bytes");
    for (int i = 0; i < EEPROM_SIZE; i++) {
        byte value = EEPROM.read(i);
        debugPrint("EEPROM[" + String(i) + "] = " + String(value));
    }
    debugPrint("=========================================");
    debugPrint("Flags:");
    debugPrint("EEPROM_INITIALIZED = " + String(EEPROM_INITIALIZED));
    debugPrint("PID_INITIALIZED = " + String(PID_INITIALIZED));
    debugPrint("WiFi_CONNECTED = " + String(WIFI_CONNECTED));
    debugPrint("AP_MODE = " + String(AP_MODE));
    debugPrint("MPU_INITIALIZED = " + String(MPU_INITIALIZED));
    debugPrint("WEBSERVER_INITIALIZED = " + String(WEBSERVER_INITIALIZED));
    debugPrint("WEBSOCKET_INITIALIZED = " + String(WEBSOCKET_INITIALIZED));
    debugPrint("INTEGRATED_HARDWARES_INITIALIZED = " + String(INTEGRATED_HARDWARES_INITIALIZED));
    debugPrint("MOTOR_ERROR = " + String(MOTOR_ERROR));
    debugPrint("EEPROM_ERROR = " + String(EEPROM_ERROR));
    debugPrint("PID_ERROR = " + String(PID_ERROR));
    debugPrint("WIFI_ERROR = " + String(WIFI_ERROR));
    debugPrint("AP_ERROR = " + String(AP_ERROR));
    debugPrint("MPU_ERROR = " + String(MPU_ERROR));
    debugPrint("PWM_ERROR = " + String(PWM_ERROR));
    debugPrint("WEBSERVER_ERROR = " + String(WEBSERVER_ERROR));
    debugPrint("WEBSOCKET_ERROR = " + String(WEBSOCKET_ERROR));
    debugPrint("MOTOR_READY = " + String(MOTOR_READY));
    debugPrint("MOTOR_RUNNING = " + String(MOTOR_RUNNING));
    debugPrint("MOTOR_STOPPED = " + String(MOTOR_STOPPED));
    debugPrint("THROTTLE_ACTIVE = " + String(THROTTLE_ACTIVE));
    debugPrint("PID_ACTIVE = " + String(PID_ACTIVE));
    debugPrint("ESC_INITIALIZED = " + String(ESC_INITIALIZED));
    debugPrint("ESC_ERROR = " + String(ESC_ERROR));
    debugPrint("EMERGENCY_STOP = " + String(EMERGENCY_STOP));
    debugPrint("MANUAL_EMERGENCY_MODE = " + String(MANUAL_EMERGENCY_MODE));
    debugPrint("AUTOMATIC_EMERGENCY_MODE = " + String(AUTOMATIC_EMERGENCY_MODE));
    debugPrint("WEBSOCKET_CONNECTED = " + String(WEBSOCKET_CONNECTED));
    debugPrint("SomeoneIsConnected = " + String(SomeoneIsConnected));
    debugPrint("=========================================");
    debugPrint("Configs:");
    debugPrint("ssid = " + String(ssid));
    debugPrint("password = " + String(password));
    debugPrint("EEPROM_SIZE = " + String(EEPROM_SIZE));
    debugPrint("Kp = " + String(Kp));
    debugPrint("Ki = " + String(Ki));
    debugPrint("Kd = " + String(Kd));
    debugPrint("defaultKp = " + String(defaultKp));
    debugPrint("defaultKi = " + String(defaultKi));
    debugPrint("defaultKd = " + String(defaultKd));
    debugPrint("setpoint = " + String(setpoint));
    debugPrint("defaultSetpoint = " + String(defaultSetpoint));
    debugPrint("escPin = " + String(escPin));
    debugPrint("pwmChannel = " + String(pwmChannel));
    debugPrint("freq = " + String(freq));
    debugPrint("resolution = " + String(resolution));
    debugPrint("debugInterval = " + String(debugInterval));
    debugPrint("DEBUG = " + String(DEBUG));
    debugPrint("=========================================");
    debugPrint("Wifi Information:");
    debugPrint("IP Address: " + getWiFiIP());
    debugPrint("SSID: " + String(ssid));
    debugPrint("Password: " + String(password));
    debugPrint("=========================================");
    debugPrint("Dump completed.");
    debugPrint("=========================================");
    return;
}

void emergencyDump(float currentAngle, int currentPWM, float currentPIDOutput) {
    // This function is called in case of an emergency
    // It will dump all the data from EEPROM, PWM, PID values, MPU values and status
    debugPrint("=========================================");
    debugPrint("Emergency dump initiated.");
    debugPrint("Dumping all data from EEPROM, flags, and configs...");
    debugPrint("This will take a while, please wait...");
    debugPrint("=========================================");
    debugPrint("Dumping all data from EEPROM, flags, and configs...");
    debugPrint("EEPROM Size: " + String(EEPROM_SIZE) + " bytes");
    for (int i = 0; i < EEPROM_SIZE; i++) {
        byte value = EEPROM.read(i);
        debugPrint("EEPROM[" + String(i) + "] = " + String(value));
    }
    debugPrint("=========================================");
    debugPrint("MOTOR_ERROR = " + String(MOTOR_ERROR));
    debugPrint("EEPROM_ERROR = " + String(EEPROM_ERROR));
    debugPrint("PID_ERROR = " + String(PID_ERROR));
    debugPrint("WIFI_ERROR = " + String(WIFI_ERROR));
    debugPrint("AP_ERROR = " + String(AP_ERROR));
    debugPrint("MPU_ERROR = " + String(MPU_ERROR));
    debugPrint("PWM_ERROR = " + String(PWM_ERROR));
    debugPrint("WEBSERVER_ERROR = " + String(WEBSERVER_ERROR));
    debugPrint("WEBSOCKET_ERROR = " + String(WEBSOCKET_ERROR));
    debugPrint("MOTOR_READY = " + String(MOTOR_READY));
    debugPrint("MOTOR_RUNNING = " + String(MOTOR_RUNNING));
    debugPrint("MOTOR_STOPPED = " + String(MOTOR_STOPPED));
    debugPrint("PID_ACTIVE = " + String(PID_ACTIVE));
    debugPrint("ESC_ERROR = " + String(ESC_ERROR));
    debugPrint("EMERGENCY_STOP = " + String(EMERGENCY_STOP));
    debugPrint("MANUAL_EMERGENCY_MODE = " + String(MANUAL_EMERGENCY_MODE));
    debugPrint("AUTOMATIC_EMERGENCY_MODE = " + String(AUTOMATIC_EMERGENCY_MODE));
    debugPrint("=========================================");
    debugPrint("VALUES: ");
    debugPrint("Angle: " + String(currentAngle));
    debugPrint("Current PWM: " + String(currentPWM));
    debugPrint("Current PID Output: " + String(currentPIDOutput));
    debugPrint("Kp: " + String(Kp));
    debugPrint("Ki: " + String(Ki));
    debugPrint("Kd: " + String(Kd));
    debugPrint("Setpoint: " + String(setpoint));
    debugPrint("=========================================");
    debugPrint("Wifi Information:");
    debugPrint("IP Address: " + getWiFiIP());
    debugPrint("SSID: " + String(ssid));
    debugPrint("Password: " + String(password));
    debugPrint("=========================================");
    debugPrint("Emergency dump completed.");


    

}