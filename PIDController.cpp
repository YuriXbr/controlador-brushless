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
    
    // Use actual timing optimized for extreme high frequency
    unsigned long currentTime = micros();
    float dt = (currentTime - lastTime) / 1000000.0; // Convert to seconds
    
    // Clamp dt to reasonable bounds - ultra tight for extreme frequency
    if (dt <= 0 || dt > 0.005) { // If dt is invalid or too large (>5ms)
        dt = PID_SAMPLE_TIME / 1000000.0; // Use configured sample time
    }
    
    lastTime = currentTime;
    float error = setpoint - angle;
    
    // For gravitational unidirecional system - much smaller deadzone for precision
    if (abs(error) < 0.5) { // Reduced from PID_DEADZONE to 0.5 degrees
        error = 0;
        // Keep integral steady in deadzone for hover stability - no decay needed
        // integral remains unchanged to maintain hover power
    }
    
    // Integral calculation optimized for gravitational compensation
    if (error != 0) {
        integral += error * dt;
        
        // Constrain integral to prevent excessive windup but allow gravitational compensation
        integral = constrain(integral, -50.0, 50.0); // Larger range for gravitational systems
    }
    
    // Derivative calculation with filtering
    float derivative = (error - lastError) / dt;
    filteredDerivative = PID_DERIVATIVE_FILTER * derivative + (1.0 - PID_DERIVATIVE_FILTER) * filteredDerivative;
    lastError = error;
    
    // PID output calculation
    float output = Kp * error + Ki * integral + Kd * filteredDerivative;
    
    // Gravitational system output mapping - bias towards sustentation
    // Map output directly to PWM range with gravitational bias
    int baseHoverPWM = 1300; // Base power needed to hover (adjust based on your system)
    int pwmOutput = baseHoverPWM + (int)(output * 15); // Scale factor for fine control
    
    // Apply safety constraints
    pwmOutput = constrain(pwmOutput, 1100, 1900); // Tighter range for stability
    
    // Minimal filtering for immediate response in gravitational system
    lastOutput = 0.9 * pwmOutput + 0.1 * lastOutput; // More responsive
    currentPIDOutput = (int)lastOutput;
    
    // Final safety constraint
    currentPIDOutput = constrain(currentPIDOutput, PID_OUTPUT_MIN, PID_OUTPUT_MAX);
    
    // Debug output - ultra reduced frequency for extreme high-speed operation
    static unsigned long lastDebugTime = 0;
    if (currentTime - lastDebugTime > 500000) { // Debug every 500ms to minimize overhead
        debugPrint("[PID] e=" + String(error, 2) + ", i=" + String(integral, 2) + 
                   ", d=" + String(filteredDerivative, 2) + ", raw=" + String(output, 2) +
                   ", out=" + String(currentPIDOutput) + ", dt=" + String(dt * 1000, 2) + "ms");
        lastDebugTime = currentTime;
    }
    
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
    lastOutput = 1300; // Start at hover power instead of 1000
    lastTime = micros();
    currentPIDOutput = 1300;
    debugPrint("[PID] Controller reset - integral cleared, timing reset, hover bias applied");
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