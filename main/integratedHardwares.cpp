#include "integratedHardwares.h"

// Função para inicializar os hardwares integrados
void initIntegratedHardwares() {
    // Inicialização de botões, displays e outros componentes pode ser feita aqui
}

// Função para ler o estado de um botão
bool readButtonState(int buttonPin) {
    return digitalRead(buttonPin) == HIGH; // Retorna verdadeiro se o botão estiver pressionado
}

// Função para atualizar o display
void updateDisplay(const String& message) {
    // Código para atualizar o display com a mensagem fornecida
}