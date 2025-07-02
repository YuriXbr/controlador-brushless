#ifndef PIDCONTROLLER_H
#define PIDCONTROLLER_H

#include <Arduino.h>

// PID configuration constants
#define PWM_SUSTENTACAO 1100
#define PID_OUTPUT_MIN 1000
#define PID_OUTPUT_MAX 2000
#define PID_INTEGRAL_MAX 300.0      // Reduced from 500 to prevent excessive windup
#define PID_DEADZONE 0.5
#define PID_DERIVATIVE_FILTER 0.15  // Slightly increased filtering
#define PID_OUTPUT_FILTER 0.2

// PID controller functions
void setupPIDController();
void setPIDParameters(float newKp, float newKi, float newKd);
int computePID(float angle);
void setSetpoint(float newSetpoint);
void resetPID();
void resetPIDIntegral(); // Force reset integral term
void updatePIDTiming();

#endif