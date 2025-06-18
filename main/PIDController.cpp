#include "PIDController.h"
#include "memoryHandler.h"
#include "debugHandler.h"
#include "configs.h"
#include "flags.h"

float integral = 0, lastError = 0;
unsigned long lastTime = 0;


void setupPIDController() {
    debugPrint("[PID] Configurando PID Controller...");
    if (EEPROM_INITIALIZED) {
        debugPrint("[PID] EEPROM inicializada, lendo parâmetros PID da memória");
        // Lê os parâmetros PID da EEPROM
        Kp = readFloatFromEEPROM(0);
        Ki = readFloatFromEEPROM(4);
        Kd = readFloatFromEEPROM(8);
        setpoint = readFloatFromEEPROM(12);
    }

    if (Kp <= 0 || Ki <= 0 || Kd <= 0 || setpoint <= -180 || setpoint >= 180 
        || isnan(Kp) || isnan(Ki) || isnan(Kd) || isnan(setpoint) 
        || Kp > 10 || Ki > 10 || Kd > 10) {
        debugPrint("[PID] Parâmetros PID não definidos na memória ou invalidos, usando valores padrão");
        Kp = defaultKp;
        Ki = defaultKi;
        Kd = defaultKd;
        setpoint = defaultSetpoint;

        debugPrint("[PID] PID Controller configurado: Kp=" + String(Kp) + ", Ki=" + String(Ki) + ", Kd=" + String(Kd) + ", Setpoint=" + String(setpoint));
        PID_INITIALIZED = true; // Marca PID como inicializado

        if (EEPROM_INITIALIZED) {
        // Salva os valores padrão na EEPROM
        writeFloatToEEPROM(0, Kp);
        writeFloatToEEPROM(4, Ki);
        writeFloatToEEPROM(8, Kd);
        writeFloatToEEPROM(12, setpoint);
        debugPrint("[PID] Parâmetros PID salvos na memória: Kp=" + String(Kp) + ", Ki=" + String(Ki) + ", Kd=" + String(Kd) + ", Setpoint=" + String(setpoint));

        } else {
            debugPrint("[PID] EEPROM não inicializada, não é possível salvar os parâmetros PID na memória");
            
        }
    }
    debugPrint("[PID] PID Controller configurado: Kp=" + String(Kp) + ", Ki=" + String(Ki) + ", Kd=" + String(Kd) + ", Setpoint=" + String(setpoint));
    PID_INITIALIZED = true; // Marca PID como inicializado
}

void setPIDParameters(float newKp, float newKi, float newKd) {
    if (!PID_INITIALIZED || PID_ERROR) {
        debugPrint("[PID] PID não inicializado ou em erro, não é possível atualizar parâmetros");
        return;
    }
    if (newKp < 0 || newKi < 0 || newKd < 0  
        || newKp > 10 || newKi > 10 || newKd > 10 
        || isnan(newKp) || isnan(newKi) || isnan(newKd)) {
        debugPrint("[PID] Parâmetros PID inválidos, não é possível atualizar");
        return;
    }

    Kp = newKp;
    Ki = newKi;
    Kd = newKd;
    debugPrint("[PID] Parâmetros PID atualizados: Kp=" + String(Kp) + ", Ki=" + String(Ki) + ", Kd=" + String(Kd));
    // Verifica se a EEPROM está inicializada antes de atualizar os parâmetros
    if (!EEPROM_INITIALIZED) {
        debugPrint("[PID] EEPROM não inicializada, não é possível salvar novos parâmetros na memória");
        return;
    }
    // Salva os novos parâmetros na EEPROM
    writeFloatToEEPROM(0, Kp);
    writeFloatToEEPROM(4, Ki);
    writeFloatToEEPROM(8, Kd);
}

int computePID(float angle) {
    if (manualMode) {
        debugPrint("[PID] Modo manual ativado, retornando PWM manual: " + String(manualPWM));
        return manualPWM;
    }
    if (!PID_INITIALIZED || PID_ERROR) {
        debugPrint("[PID] PID não inicializado, retornando PWM mínimo");
        return 1000; // Retorna PWM mínimo se PID não estiver inicializado
    }
    if (Kp == 0 && Ki == 0 && Kd == 0) {
        debugPrint("[PID] Parâmetros PID não definidos, retornando PWM mínimo");
        PID_ERROR = true; // Define a flag de erro para PID
        PID_INITIALIZED = false; // Marca PID como não inicializado
        return 1000; // Retorna PWM mínimo se os parâmetros PID não estiverem definidos
    }
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0;
    if (dt <= 0) dt = 0.001;

    lastTime = now;

    float error = setpoint - angle;
    if (abs(error) < 0.5) error = 0;

    integral += error * dt;
    integral = constrain(integral, -100, 100);

    float derivative = (error - lastError) / dt;
    lastError = error;

    float output = Kp * error + Ki * integral + Kd * derivative;
    currentPIDOutput = constrain(map(output, -50, 50, 1000, 2000), 1000, 2000);

    debugPrint("[PID] Cálculo PID: erro=" + String(error) + ", integral=" + String(integral) + ", derivativo=" + String(derivative) + ", saída=" + String(output) + ", PWM=" + String(currentPIDOutput));
    return currentPIDOutput;
}

void setSetpoint(float newSetpoint) {
    setpoint = newSetpoint;
    debugPrint("[PID] Setpoint atualizado: " + String(setpoint));
    // Verifica se a EEPROM está inicializada antes de atualizar o setpoint
    if (!EEPROM_INITIALIZED) {
        debugPrint("[PID] EEPROM não inicializada, não é possível salvar o setpoint na memória");
        return;
    }
    writeFloatToEEPROM(12, setpoint); // Atualiza o setpoint na EEPROM
}