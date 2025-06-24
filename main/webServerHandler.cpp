// ? Description: This code implements a web server for an ESP32 device 
// ? that allows users to configure PID parameters, toggle manual mode,
// ? set PWM values, and retrieve data in JSON format. It uses the WebServer 
// ? and WebSocketsServer libraries to handle HTTP requests and WebSocket connections.

// ! Version: 1.0 | Author: Yuri dos Anjos | Date: 13/06/2025


#include <WebServer.h>
#include <WebSocketsServer.h>
#include "configs.h"
#include "debugHandler.h"
#include "pageContent.h"
#include "memoryHandler.h"
#include "flags.h"
#include "PIDController.h"
#include "motorController.h"

WebServer server(80);
WebSocketsServer webSocket(81);
// Function to get the HTML page content
String getPage() {
    return String(PAGE_CONTENT);
}

bool checkWebServerInitialized() {
    if (!WEBSERVER_INITIALIZED) {
        debugPrint("[WEBS] Servidor web não inicializado");
        return false;
    }
    return true;
}
bool checkWebServerError() {

    if (WEBSERVER_ERROR) {
        debugPrint("[WEBS] Servidor web em estado de erro");
        return true;
    }
    return false;
}

// Server root handler
void handleRoot() {
    if (!checkWebServerInitialized()) {
        server.send(500, "text/plain", "Servidor não inicializado");
        return; // Exit if server is not initialized
    }
    if (checkWebServerError()) {
        server.send(500, "text/plain", "Erro no servidor");
        return; // Exit if there is a server error
    }
    server.send(200, "text/html", getPage());
}

// Function to write new PID values to EEPROM
void handleSet() {
    if (!checkWebServerInitialized()) {
        server.send(500, "text/plain", "Servidor não inicializado");
        return; // Exit if server is not initialized
    }
    if (checkWebServerError()) {
        server.send(500, "text/plain", "Erro no servidor");
        return; // Exit if there is a server error
    }
    if (server.hasArg("kp")) {
        float newKp = server.arg("kp").toFloat();
        if (newKp >= 0 && newKp <= 10) {
            Kp = newKp;
            writeFloatToEEPROM(0, Kp);
        }
    }
    if (server.hasArg("ki")) {
        float newKi = server.arg("ki").toFloat();
        if (newKi >= 0 && newKi <= 5) {
            Ki = newKi;
            writeFloatToEEPROM(4, Ki);
        }
    }
    if (server.hasArg("kd")) {
        float newKd = server.arg("kd").toFloat();
        if (newKd >= 0 && newKd <= 10) {
            Kd = newKd;
            writeFloatToEEPROM(8, Kd);
        }
    }
    server.send(200, "text/html", getPage());
    debugPrint("[WEBS] Parâmetros PID atualizados: Kp=" + String(Kp) + ", Ki=" + String(Ki) + ", Kd=" + String(Kd));
    // Send updated PID values to the PIDController
    setPIDParameters(Kp, Ki, Kd);
}

// Function to toggle manual mode
void handleToggleManual() {
    if (!checkWebServerInitialized()) {
        server.send(500, "text/plain", "Servidor não inicializado");
        return; // Exit if server is not initialized
    }
    if (checkWebServerError()) {
        server.send(500, "text/plain", "Erro no servidor");
        return; // Exit if there is a server error
    }
    // Toggle the manual mode state
    debugPrint("[WEBS] Trocando modo manual");
    manualMode = !manualMode;
    debugPrint("[WEBS] Modo manual: " + String(manualMode));
    server.send(200, "text/plain", "ok");
}

// Function to set PWM value in manual mode
void handleSetPWM() {
    if (!checkWebServerInitialized()) {
        server.send(500, "text/plain", "Servidor não inicializado");
        return; // Exit if server is not initialized
    }
    if (checkWebServerError()) {
        server.send(500, "text/plain", "Erro no servidor");
        return; // Exit if there is a server error
    }
    // Check if the request has a "value" argument
    debugPrint("[WEBS] Recebendo valor de PWM manual");
    debugPrint("[WEBS] Valor recebido: " + server.arg("value"));
    // If the argument exists, parse it and set manualPWM
    if (server.hasArg("value")) {
        int val = server.arg("value").toInt();
        manualPWM = constrain(val, 1000, 2000);
    }
    server.send(200, "text/plain", "ok");
}

// Function to handle manual state
void handleManualState() {
    if (!checkWebServerInitialized()) {
        server.send(500, "text/plain", "Servidor não inicializado");
        return; // Exit if server is not initialized
    }
    if (checkWebServerError()) {
        server.send(500, "text/plain", "Erro no servidor");
        return; // Exit if there is a server error
    }
    // Send the current manual mode state
    debugPrint("[WEBS] Estado do modo manual: " + String(manualMode ? "Ativado" : "Desligado"));
    server.send(200, "text/plain", manualMode ? "Ativado" : "Desligado");
}

// Function to handle data retrieval
void handleData() {
    if (!checkWebServerInitialized()) {
        server.send(500, "text/plain", "Servidor não inicializado");
        return; // Exit if server is not initialized
    }
    if (checkWebServerError()) {
        server.send(500, "text/plain", "Erro no servidor");
        return; // Exit if there is a server error
    }
    // Prepare JSON response with current angle, PWM, and PID output
    String json = "{\"angle\":" + String(currentAngle) + ",\"pwm\":" + String(currentPWM) + ",\"pid\":" + String(currentPIDOutput) + "}";
    server.send(200, "application/json", json);
}

// Função para alternar o estado do motor (iniciar/parar)
void handleToggleMotor() {
    if (!checkWebServerInitialized()) {
        server.send(500, "text/plain", "Servidor não inicializado");
        return;
    }
    if (checkWebServerError()) {
        server.send(500, "text/plain", "Erro no servidor");
        return;
    }
    MOTOR_RUNNING = !MOTOR_RUNNING;
    MOTOR_STOPPED = !MOTOR_RUNNING;
    if (MOTOR_RUNNING) {
        resetPID(); // Reseta o PID ao ligar o motor
        setMotorPWM(1000); // Garante PWM inicial baixo
        rampaDePartidaAteControle(1200); // Rampa de partida suave para o controle
    } else {
        rampaDeDesligamento(2000); // Rampa de desligamento suave (ex: 1,2s)
        setMotorPWM(1000); // Garante que o motor pare
        resetPID(); // Reseta o PID ao desligar também
        
    }
    debugPrint("[WEBS] Estado do motor alterado: " + String(MOTOR_RUNNING ? "Ligado" : "Desligado"));
    server.send(200, "text/plain", MOTOR_RUNNING ? "Ligado" : "Desligado");
}

// Função para consultar o estado do motor
void handleMotorState() {
    if (!checkWebServerInitialized()) {
        server.send(500, "text/plain", "Servidor não inicializado");
        return;
    }
    if (checkWebServerError()) {
        server.send(500, "text/plain", "Erro no servidor");
        return;
    }
    server.send(200, "text/plain", MOTOR_RUNNING ? "Ligado" : "Desligado");
}

// Rota para alterar o setpoint via requisição GET: /setSetpoint?value=XX
void handleSetSetpoint() {
    if (!checkWebServerInitialized()) {
        server.send(500, "text/plain", "Servidor não inicializado");
        return;
    }
    if (checkWebServerError()) {
        server.send(500, "text/plain", "Erro no servidor");
        return;
    }
    if (server.hasArg("value")) {
        float newSetpoint = server.arg("value").toFloat();
        setSetpoint(newSetpoint);
        server.send(200, "text/plain", "Setpoint atualizado para " + String(newSetpoint));
        debugPrint("[WEBS] Setpoint alterado via web: " + String(newSetpoint));
    } else {
        server.send(400, "text/plain", "Parâmetro 'value' não informado");
    }
}

// Function to handle client requests
void handleClientRequests() {
    if (!checkWebServerInitialized()) {
        debugPrint("[WEBS] Servidor web não inicializado, não é possível processar requisições");
        return; // Exit if server is not initialized
    }
    if (checkWebServerError()) {
        debugPrint("[WEBS] Servidor web em estado de erro, não é possível processar requisições");
        return; // Exit if there is a server error
    }
    server.handleClient(); // Process incoming client requests
    webSocket.loop(); // Process WebSocket messages
}

// Server routes
void setupWebServer() {
    if(WIFI_ERROR && !AP_MODE) {
        debugPrint("[WEBS] ESP Não está conectado ao WiFi e não está em modo AP");
        WEBSERVER_INITIALIZED = false;
        WEBSOCKET_INITIALIZED = false;
        WEBSERVER_ERROR = true;
        WEBSOCKET_ERROR = true;
        return; // Exit if WiFi is not connected and not in AP mode
    }

    debugPrint("[WEBS] Iniciando o servidor web na porta 80");
    server.on("/", handleRoot);
    server.on("/set", handleSet);
    server.on("/toggleManual", handleToggleManual);
    server.on("/setPWM", handleSetPWM);
    server.on("/manualState", handleManualState);
    server.on("/data", handleData);
    server.on("/toggleMotor", handleToggleMotor);
    server.on("/motorState", handleMotorState);
    server.on("/setSetpoint", handleSetSetpoint);
    server.onNotFound([]() {
        server.send(404, "text/plain", "Not Found");
    });
    server.begin();
    webSocket.begin();
    debugPrint("[WEBS] Servidor web iniciado na porta 80");
    debugPrint("[WEBS] WebSocket iniciado na porta 81");
    WEBSERVER_INITIALIZED = true;
    WEBSOCKET_INITIALIZED = true;
}