#include "PIDController.h"
#include "memoryHandler.h"
#include "debugHandler.h"
#include "configs.h"
#include "flags.h"

// PID control variables
float integral = 0, lastError = 0;
float filteredDerivative = 0;
float lastOutput = 1300;
unsigned long lastTime = 0;

// Adaptive sustentation learning
float adaptiveSustentationPWM = 1250;
float lastStableAngle = 0;
unsigned long stableStartTime = 0;
bool isStable = false;
const float STABILITY_THRESHOLD = 1.0;
const unsigned long STABILITY_TIME = 2000;
const float LEARNING_RATE = 0.02;

static unsigned long lastDebugTime = 0;

void setupPIDController() {
    debugPrint("[PID] Initializing adaptive controller");
    
    lastTime = micros();
    integral = 0;
    lastError = 0;
    filteredDerivative = 0;
    lastOutput = 1300;
    adaptiveSustentationPWM = 1250;
    lastStableAngle = 0;
    stableStartTime = 0;
    isStable = false;
    
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
    debugPrint("[PID] Adaptive sustentation learning enabled");
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
    
    // Timing optimization for high frequency
    unsigned long currentTime = micros();
    float dt = (currentTime - lastTime) / 1000000.0;
    
    if (dt <= 0 || dt > 0.005) {
        dt = PID_SAMPLE_TIME / 1000000.0;
    }
    
    lastTime = currentTime;
    float error = setpoint - angle;
    
    updateAdaptiveSustentation(angle, error);
    
    // Deadzone for precision hover
    bool inDeadzone = false;
    if (abs(error) < 0.5) {
        inDeadzone = true;
        error = 0;
    }
    
    // Integral with anti-windup
    if (error != 0) {
        integral += error * dt;
        integral = constrain(integral, -50.0, 50.0);
    }
    
    // Derivative with filtering
    float derivative = (error - lastError) / dt;
    filteredDerivative = PID_DERIVATIVE_FILTER * derivative + (1.0 - PID_DERIVATIVE_FILTER) * filteredDerivative;
    lastError = error;
    
    float output = Kp * error + Ki * integral + Kd * filteredDerivative;
    
    int pwmOutput;
    
    if (inDeadzone && abs(output) < 2.0) {
        // Deadzone: use adaptive sustentation with minimal correction
        pwmOutput = (int)adaptiveSustentationPWM + (int)(output * 5);
    } else {
        // Normal PID: adaptive base + full PID correction
        pwmOutput = (int)adaptiveSustentationPWM + (int)(output * 10);
    }
    
    // Safety constraints - allow PID to work but prevent dangerous values
    int minPWM = 1100;
    int maxPWM = 1900;
    
    // Only apply tighter limits when very close to setpoint for safety
    if (inDeadzone) {
        minPWM = max(1150, (int)adaptiveSustentationPWM - 100);
        maxPWM = min(1450, (int)adaptiveSustentationPWM + 100);
    }
    
    pwmOutput = constrain(pwmOutput, minPWM, maxPWM);
    
    // Output filtering
    lastOutput = 0.8 * pwmOutput + 0.2 * lastOutput;
    currentPIDOutput = (int)lastOutput;
    
    // Final safety - only prevent dangerous extremes
    if (currentPIDOutput < 1100) {
        debugPrint("[PID] WARNING: PWM below minimum, forcing to 1100 (was " + String(currentPIDOutput) + ")");
        currentPIDOutput = 1100;
    }
    if (currentPIDOutput > 1900) {
        debugPrint("[PID] WARNING: PWM above maximum, forcing to 1900 (was " + String(currentPIDOutput) + ")");
        currentPIDOutput = 1900;
    }
    
    // Debug output every 500ms
    if (currentTime - lastDebugTime > 500000) {
        String mode = inDeadzone ? "DZ" : "PID";
        float errorMagnitude = abs(setpoint - angle);
        debugPrint("[" + mode + "] e=" + String(error, 2) + ", |e|=" + String(errorMagnitude, 1) +
                   ", i=" + String(integral, 2) + ", d=" + String(filteredDerivative, 2) + 
                   ", pidOut=" + String(output, 2) + ", adaptBase=" + String(adaptiveSustentationPWM, 1) +
                   ", finalPWM=" + String(currentPIDOutput) +
                   ", limits=[" + String(minPWM) + "-" + String(maxPWM) + "]");
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
    lastOutput = adaptiveSustentationPWM;
    lastTime = micros();
    currentPIDOutput = max((int)adaptiveSustentationPWM, 1150);
    isStable = false;
    stableStartTime = 0;
    debugPrint("[PID] Controller reset - using safe learned sustentation PWM: " + String(currentPIDOutput));
}

void resetPIDIntegral() {
    integral = 0;
    debugPrint("[PID] Integral term reset - keeping learned sustentation: " + String(adaptiveSustentationPWM, 1));
}

void resetAdaptiveLearning() {
    adaptiveSustentationPWM = 1250;
    isStable = false;
    stableStartTime = 0;
    lastStableAngle = 0;
    debugPrint("[PID] Adaptive learning reset - starting conservative learning cycle at PWM 1250");
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
    unsigned long expectedInterval = PID_SAMPLE_TIME;
    
    long timingError = (long)actualInterval - (long)expectedInterval;
    float errorPercentage = (float)abs(timingError) / expectedInterval * 100.0;
    
    if (errorPercentage > 15.0) {
        consecutiveTimingErrors++;
        debugPrint("[PID] Timing deviation: " + String(errorPercentage, 1) + "% (" + 
                   String(actualInterval/1000.0, 1) + "ms vs " + 
                   String(expectedInterval/1000.0, 1) + "ms expected)");
        
        if (consecutiveTimingErrors > 5) {
            debugPrint("[PID] WARNING: Consistent timing issues detected. Consider:");
            debugPrint("  - Reducing debug output frequency");
            debugPrint("  - Optimizing loop execution time");
            debugPrint("  - Increasing PID_SAMPLE_TIME if needed");
            consecutiveTimingErrors = 0;
        }
    } else {
        consecutiveTimingErrors = 0;
    }
    
    lastTimingCheck = currentTime;
}

// Adaptive sustentation learning system
void updateAdaptiveSustentation(float angle, float error) {
    unsigned long currentTime = millis();
    
    // Check system stability
    if (abs(error) < STABILITY_THRESHOLD) {
        if (!isStable) {
            isStable = true;
            stableStartTime = currentTime;
            lastStableAngle = angle;
        } else if (currentTime - stableStartTime > STABILITY_TIME) {
            // Learn PWM after stable period
            if (abs(angle - lastStableAngle) < 1.0) {
                float targetPWM = currentPIDOutput;
                
                // Validate learning PWM range - be more permissive
                if (targetPWM >= 1150 && targetPWM <= 1600) {
                    float learningRate = LEARNING_RATE * 0.5;
                    learningRate = constrain(learningRate, 0.005, 0.02);
                    
                    float pwmDifference = abs(targetPWM - adaptiveSustentationPWM);
                    if (pwmDifference < 150) { // More permissive difference
                        adaptiveSustentationPWM = adaptiveSustentationPWM * (1.0 - learningRate) + 
                                                 targetPWM * learningRate;
                        
                        adaptiveSustentationPWM = constrain(adaptiveSustentationPWM, 1150, 1500);
                        
                        // Debug learning progress
                        static unsigned long lastLearnDebug = 0;
                        if (currentTime - lastLearnDebug > 2000) {
                            debugPrint("[LEARN] angle=" + String(angle, 1) + 
                                      ", learned=" + String(adaptiveSustentationPWM, 1) +
                                      ", target=" + String(targetPWM, 1) + 
                                      ", diff=" + String(pwmDifference, 1) +
                                      ", rate=" + String(learningRate, 3));
                            lastLearnDebug = currentTime;
                        }
                    } else {
                        // Reject large PWM changes
                        static unsigned long lastRejectDebug = 0;
                        if (currentTime - lastRejectDebug > 3000) {
                            debugPrint("[LEARN] Rejected learning: target=" + String(targetPWM, 1) +
                                      ", current=" + String(adaptiveSustentationPWM, 1) +
                                      ", diff=" + String(pwmDifference, 1) + " (too large)");
                            lastRejectDebug = currentTime;
                        }
                    }
                } else {
                    // Reject out-of-range PWM
                    static unsigned long lastRangeDebug = 0;
                    if (currentTime - lastRangeDebug > 3000) {
                        debugPrint("[LEARN] Rejected learning: target=" + String(targetPWM, 1) +
                                  " outside range [1150-1600]");
                        lastRangeDebug = currentTime;
                    }
                }
            }
        }
    } else {
        isStable = false;
    }
    
    // Minimal angle compensation for large angles only
    if (!isStable && abs(angle) > 15.0) {
        float angleCompensation = cos(angle * PI / 180.0);
        angleCompensation = max(angleCompensation, 0.85f);
        
        static float baseAdaptivePWM = adaptiveSustentationPWM;
        float compensatedPWM = baseAdaptivePWM * angleCompensation;
        adaptiveSustentationPWM = adaptiveSustentationPWM * 0.99f + compensatedPWM * 0.01f;
        adaptiveSustentationPWM = constrain(adaptiveSustentationPWM, 1150.0f, 1500.0f);
    }
}