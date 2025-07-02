#include "motorController.h"
#include <Arduino.h>
#include "debugHandler.h"
#include "flags.h"
#include "configs.h"
#include "PIDController.h"
#include "gyroController.h"

// Non-blocking ramp variables
unsigned long rampStartTime = 0;
int rampStartPWM = 1000;
int rampTargetPWM = 1000;
bool rampActive = false;
bool rampStartup = false;
const int RAMP_DURATION_MS = 2000; // 2 seconds for smooth transition
const int RAMP_STEPS = 50;

void setupMotorController() {
    ledcSetup(pwmChannel, freq, resolution);
    ledcAttachPin(escPin, pwmChannel);
    setMotorPWM(1000);
    debugPrint("[MOTOR] Controller initialized - Channel: " + String(pwmChannel) + 
               ", Freq: " + String(freq) + "Hz, Resolution: " + String(resolution));
    MOTOR_READY = true;
    MOTOR_RUNNING = false;
    MOTOR_STOPPED = true;
}

void setMotorPWM(int pwmValue) {
    // Validate PWM range
    if (pwmValue < 1000 || pwmValue > 2000) {
        debugPrint("[MOTOR] PWM out of range: " + String(pwmValue));
        return;
    }
    
    uint32_t duty = pulseWidthToDuty(pwmValue);
    ledcWrite(pwmChannel, duty);
}

void motorEmergencyStop() {
    debugPrint("[MOTOR] Emergency stop triggered");
    rampActive = false;
    setMotorPWM(1000);
    EMERGENCY_STOP = true;
    MOTOR_RUNNING = false;
    MOTOR_STOPPED = true;
}

uint32_t pulseWidthToDuty(float pulse_us) {
    return (uint32_t)((pulse_us / 20000.0) * ((1 << resolution) - 1));
}

// Simplified motor control without ramps - immediate response for gravitational system
void startMotor() {
    MOTOR_RUNNING = true;
    MOTOR_STOPPED = false;
    debugPrint("[MOTOR] Started - immediate response for gravitational control");
}

void stopMotor() {
    setMotorPWM(1000);
    MOTOR_RUNNING = false;
    MOTOR_STOPPED = true;
    debugPrint("[MOTOR] Stopped - PWM set to 1000");
}