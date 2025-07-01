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

void startMotorRamp() {
    if (!rampActive) {
        rampStartTime = millis();
        rampStartPWM = 1000;
        rampTargetPWM = manualMode ? manualPWM : computePID(currentAngle);
        rampActive = true;
        rampStartup = true;
        debugPrint("[MOTOR] Starting ramp to " + String(rampTargetPWM));
    }
}

void stopMotorRamp() {
    if (!rampActive) {
        rampStartTime = millis();
        rampStartPWM = manualMode ? manualPWM : computePID(currentAngle);
        rampTargetPWM = 1000;
        rampActive = true;
        rampStartup = false;
        debugPrint("[MOTOR] Starting stop ramp from " + String(rampStartPWM));
    }
}

void updateMotorRamp() {
    if (!rampActive) return;
    
    unsigned long elapsed = millis() - rampStartTime;
    
    if (elapsed >= RAMP_DURATION_MS) {
        // Ramp complete
        setMotorPWM(rampTargetPWM);
        rampActive = false;
        
        if (rampStartup) {
            MOTOR_RUNNING = true;
            MOTOR_STOPPED = false;
        } else {
            MOTOR_RUNNING = false;
            MOTOR_STOPPED = true;
        }
        
        debugPrint("[MOTOR] Ramp complete - PWM: " + String(rampTargetPWM));
        return;
    }
    
    // Calculate current PWM value using smooth curve
    float progress = (float)elapsed / RAMP_DURATION_MS;
    progress = progress * progress * (3.0 - 2.0 * progress); // Smooth step function
    
    int currentPWM = rampStartPWM + (int)((rampTargetPWM - rampStartPWM) * progress);
    
    // Update target for startup ramp if in PID mode
    if (rampStartup && !manualMode) {
        rampTargetPWM = computePID(currentAngle);
    }
    
    setMotorPWM(currentPWM);
}