#include "motorController.h"
#include <Arduino.h>
#include "debugHandler.h"
#include "flags.h"
#include "configs.h"


void setupMotorController() {
    ledcSetup(pwmChannel, freq, resolution);
    ledcAttachPin(escPin, pwmChannel);
    setMotorPWM(1000); // Inicializa o PWM com valor mínimo
    debugPrint("[MOTOR] Motor controller setup complete. PWM channel: " + String(pwmChannel) + ", Frequency: " + String(freq) + ", Resolution: " + String(resolution));
    MOTOR_READY = true; // Marca o motor como pronto
    MOTOR_RUNNING = false; // Inicialmente o motor não está rodando
    MOTOR_STOPPED = true; // Inicialmente o motor está parado
}

void setMotorPWM(int pwmValue) {
    uint32_t duty = pulseWidthToDuty(pwmValue);
    if (pwmValue < 1000 || pwmValue > 2000) {
        debugPrint("[MOTOR] Valor de PWM fora do intervalo permitido (1000-2000): " + String(pwmValue));
        return; // Evita valores de PWM fora do intervalo
    }
    debugPrint("[MOTOR] Setting motor PWM to: " + String(pwmValue));
    MOTOR_RUNNING = (pwmValue > 1000); // Atualiza o estado do motor
    MOTOR_STOPPED = (pwmValue <= 1000); // Atualiza o estado do motor    
    ledcWrite(pwmChannel, duty);
}

void motorEmergencyStop() {
    debugPrint("[MOTOR] Emergency stop triggered. Stopping motor.");
    setMotorPWM(1000); // Define o PWM para o valor mínimo
    EMERGENCY_STOP = true; // Marca a emergência como acionada
    MOTOR_RUNNING = false; // O motor não está mais rodando
    MOTOR_STOPPED = true; // O motor está parado
}

uint32_t pulseWidthToDuty(float pulse_us) {
    return (uint32_t)((pulse_us / 20000.0) * ((1 << resolution) - 1));
}