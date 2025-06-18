#include <Wire.h>
#include <EEPROM.h>
#include "memoryHandler.h"
#include "wifiHandler.h"
#include "webServerHandler.h"
#include "gyroController.h"
#include "motorController.h"
#include "PIDController.h"
#include "integratedHardwares.h"
#include "debugHandler.h"
#include "configs.h"
#include "flags.h"
#include "memoryHandler.h"
#include "dump.h"

static unsigned long lastDebugSend = 0; // Variável para controlar o envio do buffer de depuração
void setup() {
    Wire.begin();
    Serial.begin(115200);
    setupWiFi(); // Configurações de Wi-Fi
    initMemory(); // Inicializa a memória EEPROM
    setupWebServer();  // Configura o servidor web
    Serial.println("Iniciando Sistema...");
    initGyro(); // Configura o giroscópio
    setupMotorController();  // Configura o controlador de motor
    setupPIDController(); // Configura o controlador PID

    dumpAll(); // Realiza o dump de todos os dados para depuração
    if(EEPROM_ERROR || MPU_ERROR || MOTOR_ERROR || PID_ERROR || WIFI_ERROR || WEBSERVER_ERROR || WEBSOCKET_ERROR) {
        debugPrint("[SETUP] Erros encontrados durante a inicialização. Verifique os logs.");
        sendDebugBuffer(); // Envia o buffer de depuração inicial
        while (true) {
            delay(1000); // Loop infinito para evitar continuar se houver erros
        }
        
    } else {
        debugPrint("[SETUP] Inicialização concluída com sucesso.");
        sendDebugBuffer(); // Envia o buffer de depuração inicial
    }
}

void loop() {
    handleClientRequests(); // Processa requisições do cliente
    
    currentPWM = 1000;
    currentAngle = readGyroAngle(); // Lê o ângulo do giroscópio

    static unsigned long lastPIDUpdate = 0;
    if(millis() - lastPIDUpdate >= 20) {
        lastPIDUpdate = millis();
        currentAngle = readGyroAngle(); // Atualiza o ângulo do giroscópio
        currentPWM = computePID(currentAngle); // Calcula o PWM com base no ângulo lido
        setMotorPWM(currentPWM); // Define o PWM do motor

        sendDebugBuffer(); // Envia o buffer de depuração
    }
    



    if (millis() - lastDebugSend > debugInterval) {
        sendDebugBuffer(); // Envia o buffer de depuração periodicamente
        lastDebugSend = millis();
    }
    if (MOTOR_ERROR || PID_ERROR || MPU_ERROR || PWM_ERROR) {
        debugPrint("[LOOP] Erro detectado! Verifique os logs.");
        motorEmergencyStop(); // Aciona a parada de emergência do motor
        emergencyDump(currentAngle, currentPWM, currentPIDOutput); // Realiza o dump de emergência
        sendDebugBuffer(); // Envia o buffer de depuração em caso de erro
        while (true) {
            delay(1000); // Loop infinito para evitar continuar se houver erros
        }
    }
    delay(10); // Pequeno atraso para evitar sobrecarga do loop
}