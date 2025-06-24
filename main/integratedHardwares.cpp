#include "integratedHardwares.h"
#include "flags.h"

// Função para inicializar os hardwares integrados
void initIntegratedHardwares() {
    pinMode(MOTOR_BUTTON_PIN, INPUT); // Inicializa o botão como entrada normal (pressionado = HIGH)
    // Inicialização de botões, displays e outros componentes
}

// Função para ler o estado de um botão
bool readButtonState(int buttonPin) {
    return digitalRead(buttonPin) == HIGH; // Botão pressionado = HIGH
}

// Função para atualizar o display
void updateDisplay(const String& message) {
    // Código para atualizar o display com a mensagem fornecida
}

// --- Controle do botão liga/desliga do motor com debounce/histerese ---
void handleMotorButton() {
    static bool lastButtonState = LOW;
    static unsigned long lastDebounceTime = 0;
    static const unsigned long debounceDelay = 50; // 50 ms para debounce
    static bool motorState = false;

    bool reading = digitalRead(MOTOR_BUTTON_PIN);
    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > debounceDelay) {
        // Botão pressionado (HIGH) e mudou de estado
        if (lastButtonState == LOW && reading == HIGH) {
            motorState = !motorState;
            MOTOR_RUNNING = motorState;
            MOTOR_STOPPED = !motorState;
        }
    }
    lastButtonState = reading;
}