#include "motorController.h"
#include <Arduino.h>
#include "debugHandler.h"
#include "flags.h"
#include "configs.h"
#include "PIDController.h"
#include "gyroController.h"


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
    if(MOTOR_RUNNING) debugPrint("[MOTOR] Setting motor PWM to: " + String(pwmValue));
    if(!MOTOR_RUNNING) debugPrint("[MOTOR] Motor is not running, setting PWM to minimum: 1000");
    // NÃO altere MOTOR_RUNNING/MOTOR_STOPPED aqui, Yuri, PORRA!
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

// Rampa de partida suave até o valor do PID ou manual
void rampaDePartidaAteControle(int tempoMs) {
    int pwmInicial = 1000;
    int pwmFinal = manualMode ? manualPWM : computePID(currentAngle);
    int steps = 20;
    int stepDelay = tempoMs / steps;
    for (int i = 1; i <= steps; i++) {
        int pwmAtual = pwmInicial + ((pwmFinal - pwmInicial) * i) / steps;
        setMotorPWM(pwmAtual);
        // Atualiza o PID a cada passo se não for modo manual
        if (!manualMode) {
            currentAngle = readGyroAngle();
            pwmFinal = computePID(currentAngle);
        }
        delay(stepDelay);
    }
    setMotorPWM(pwmFinal); // Garante valor final
}

// Rampa de desligamento suave do motor
void rampaDeDesligamento(int tempoMs) {
    int pwmInicial = manualMode ? manualPWM : computePID(currentAngle);
    int pwmFinal = 1000;
    int steps = 20;
    int stepDelay = tempoMs / steps;
    for (int i = 1; i <= steps; i++) {
        int pwmAtual = pwmInicial - ((pwmInicial - pwmFinal) * i) / steps;
        setMotorPWM(pwmAtual);
        // Atualiza o PID a cada passo se não for modo manual
        if (!manualMode) {
            currentAngle = readGyroAngle();
            pwmInicial = computePID(currentAngle);
        }
        delay(stepDelay);
    }
    setMotorPWM(pwmFinal); // Garante valor final
}