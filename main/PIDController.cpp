#include "PIDController.h"
#include "memoryHandler.h"
#include "debugHandler.h"
#include "configs.h"
#include "flags.h"

// PID control variables
float integral = 0, lastError = 0;
float filteredDerivative = 0;
float lastOutput = PWM_SUSTENTACAO;
unsigned long lastTime = 0;
const float FIXED_DT = 0.02; // 20ms fixed period for consistent behavior

void setupPIDController() {
    debugPrint("[PID] Initializing controller");
    
    lastTime = micros();
    integral = 0;
    lastError = 0;
    filteredDerivative = 0;
    lastOutput = PWM_SUSTENTACAO;
    
    if (EEPROM_INITIALIZED) {
        debugPrint("[PID] Loading parameters from EEPROM");
        Kp = readFloatFromEEPROM(0);
        Ki = readFloatFromEEPROM(4);
        Kd = readFloatFromEEPROM(8);
        setpoint = readFloatFromEEPROM(12);
    }

    // Validate and set defaults if necessary
    if (Kp <= 0 || Ki <= 0 || Kd <= 0 || setpoint <= -180 || setpoint >= 180 
        || isnan(Kp) || isnan(Ki) || isnan(Kd) || isnan(setpoint) 
        || Kp > 10 || Ki > 10 || Kd > 10) {
        
        debugPrint("[PID] Invalid parameters detected, using defaults");
        Kp = defaultKp;
        Ki = defaultKi;
        Kd = defaultKd;
        setpoint = defaultSetpoint;

        if (EEPROM_INITIALIZED) {
            writeFloatToEEPROM(0, Kp);
            writeFloatToEEPROM(4, Ki);
            writeFloatToEEPROM(8, Kd);
            writeFloatToEEPROM(12, setpoint);
            debugPrint("[PID] Default parameters saved to EEPROM");
        }
    }
    
    debugPrint("[PID] Configured - Kp=" + String(Kp, 3) + ", Ki=" + String(Ki, 3) + 
               ", Kd=" + String(Kd, 3) + ", Setpoint=" + String(setpoint, 2));
    debugPrint("[PID] Sample time: " + String(PID_SAMPLE_TIME/1000.0, 1) + "ms");
    PID_INITIALIZED = true;
}

void setPIDParameters(float newKp, float newKi, float newKd) {
    if (!PID_INITIALIZED || PID_ERROR) {
        debugPrint("[PID] Cannot update - not initialized or error state");
        return;
    }
    
    // Validate parameters
    if (newKp < 0 || newKi < 0 || newKd < 0 || newKp > 10 || newKi > 10 || newKd > 10 
        || isnan(newKp) || isnan(newKi) || isnan(newKd)) {
        debugPrint("[PID] Invalid parameters rejected");
        return;
    }

    Kp = newKp;
    Ki = newKi;
    Kd = newKd;
    debugPrint("[PID] Parameters updated - Kp=" + String(Kp) + ", Ki=" + String(Ki) + ", Kd=" + String(Kd));
    
    if (EEPROM_INITIALIZED) {
        writeFloatToEEPROM(0, Kp);
        writeFloatToEEPROM(4, Ki);
        writeFloatToEEPROM(8, Kd);
    }
}

int computePID(float angle) {
    if (manualMode) {
        return manualPWM;
    }
    
    if (!PID_INITIALIZED || PID_ERROR) {
        debugPrint("[PID] Not initialized, returning minimum PWM");
        return PID_OUTPUT_MIN;
    }
    
    if (Kp == 0 && Ki == 0 && Kd == 0) {
        debugPrint("[PID] Zero parameters, entering error state");
        PID_ERROR = true;
        PID_INITIALIZED = false;
        return PID_OUTPUT_MIN;
    }
    
    // Use actual timing instead of fixed dt for better accuracy
    unsigned long currentTime = micros();
    float dt = (currentTime - lastTime) / 1000000.0; // Convert to seconds
    
    // Clamp dt to reasonable bounds to prevent instability
    if (dt <= 0 || dt > 0.1) { // If dt is invalid or too large (>100ms)
        dt = PID_SAMPLE_TIME / 1000000.0; // Use configured sample time
    }
    
    lastTime = currentTime;
    float error = setpoint - angle;
    
    // Check for stuck integral (same value for too long when error is not zero)
    static float lastIntegralValue = 0;
    static int stuckIntegralCount = 0;
    static unsigned long lastIntegralCheck = 0;
    
    if (currentTime - lastIntegralCheck > 1000000) { // Check every second
        if (abs(integral - lastIntegralValue) < 0.01 && abs(error) > PID_DEADZONE) {
            stuckIntegralCount++;
            if (stuckIntegralCount > 3) { // If stuck for 3+ seconds
                debugPrint("[PID] WARNING: Integral appears stuck at " + String(integral, 2) + ", forcing reset");
                integral = 0;
                stuckIntegralCount = 0;
            }
        } else {
            stuckIntegralCount = 0;
        }
        lastIntegralValue = integral;
        lastIntegralCheck = currentTime;
    }
    
    // Deadzone implementation with integral reset
    if (abs(error) < PID_DEADZONE) {
        error = 0;
        // Reset integral when in deadzone to prevent windup
        integral = integral * 0.9; // Gradually decay integral instead of instant reset
    }
    
    // Integral calculation with improved anti-windup
    if (error != 0) {
        float newIntegral = integral + error * dt;
        
        // Anti-windup: Check if we're at output limits
        float testOutput = Kp * error + Ki * newIntegral;
        
        // Only update integral if we're not saturating or if error would help reduce saturation
        if (abs(testOutput) < PID_INTEGRAL_MAX || 
            (testOutput > PID_INTEGRAL_MAX && error < 0) ||
            (testOutput < -PID_INTEGRAL_MAX && error > 0)) {
            integral = newIntegral;
        }
        
        // Additional safety constraint
        integral = constrain(integral, -PID_INTEGRAL_MAX / Ki, PID_INTEGRAL_MAX / Ki);
    }
    
    // Derivative calculation with filtering
    float derivative = (error - lastError) / dt;
    filteredDerivative = PID_DERIVATIVE_FILTER * derivative + (1.0 - PID_DERIVATIVE_FILTER) * filteredDerivative;
    lastError = error;
    
    // PID output calculation
    float output = Kp * error + Ki * integral + Kd * filteredDerivative;
    
    // Improved output mapping - use smaller range for better control
    int pwmOutput = map(constrain(output, -30, 30), -30, 30, PWM_SUSTENTACAO, PID_OUTPUT_MAX);
    
    // Apply output constraints before filtering
    pwmOutput = constrain(pwmOutput, PID_OUTPUT_MIN, PID_OUTPUT_MAX);
    
    // Reduce output filtering for more responsive control
    lastOutput = 0.8 * pwmOutput + 0.2 * lastOutput;
    currentPIDOutput = (int)lastOutput;
    
    // Final safety constraint
    currentPIDOutput = constrain(currentPIDOutput, PID_OUTPUT_MIN, PID_OUTPUT_MAX);
    
    debugPrint("[PID] e=" + String(error, 2) + ", i=" + String(integral, 2) + 
               ", d=" + String(filteredDerivative, 2) + ", raw=" + String(output, 2) +
               ", out=" + String(currentPIDOutput) + ", dt=" + String(dt * 1000, 1) + "ms");
    
    return currentPIDOutput;
}

void setSetpoint(float newSetpoint) {
    setpoint = newSetpoint;
    debugPrint("[PID] Setpoint updated: " + String(setpoint));
    
    if (EEPROM_INITIALIZED) {
        writeFloatToEEPROM(12, setpoint);
    }
}

void resetPID() {
    integral = 0;
    lastError = 0;
    filteredDerivative = 0;
    lastOutput = PWM_SUSTENTACAO;
    lastTime = micros();
    currentPIDOutput = PWM_SUSTENTACAO;
    debugPrint("[PID] Controller reset - integral cleared, timing reset");
}

void resetPIDIntegral() {
    integral = 0;
    debugPrint("[PID] Integral term force reset to zero");
}

void updatePIDTiming() {
    static unsigned long lastTimingCheck = 0;
    static int consecutiveTimingErrors = 0;
    unsigned long currentTime = micros();
    
    if (lastTimingCheck == 0) {
        lastTimingCheck = currentTime;
        return;
    }
    
    unsigned long actualInterval = currentTime - lastTimingCheck;
    unsigned long expectedInterval = PID_SAMPLE_TIME; // Use configured sample time
    
    long timingError = (long)actualInterval - (long)expectedInterval;
    float errorPercentage = (float)abs(timingError) / expectedInterval * 100.0;
    
    if (errorPercentage > 15.0) { // Reduced threshold for earlier detection
        consecutiveTimingErrors++;
        debugPrint("[PID] Timing deviation: " + String(errorPercentage, 1) + "% (" + 
                   String(actualInterval/1000.0, 1) + "ms vs " + 
                   String(expectedInterval/1000.0, 1) + "ms expected)");
        
        // If consistent timing issues, suggest actions
        if (consecutiveTimingErrors > 5) {
            debugPrint("[PID] WARNING: Consistent timing issues detected. Consider:");
            debugPrint("  - Reducing debug output frequency");
            debugPrint("  - Optimizing loop execution time");
            debugPrint("  - Increasing PID_SAMPLE_TIME if needed");
            consecutiveTimingErrors = 0; // Reset counter
        }
    } else {
        consecutiveTimingErrors = 0; // Reset on good timing
    }
    
    lastTimingCheck = currentTime;
}