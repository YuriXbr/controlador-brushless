#include "debugHandler.h"
#include <vector>
#include <WebSocketsServer.h>
#include "configs.h"

extern WebSocketsServer webSocket;  // declaração para acesso ao webSocket

std::vector<String> debugBuffer;
unsigned long lastDebugSend = 0;

void debugPrint(String msg) {
  if(DEBUG) {
    // Envia a mensagem de depuração para o console serial
    Serial.println(msg);
    // Verifica se o buffer de depuração já atingiu o tamanho máximo
    if (debugBuffer.size() >= 100) { // Limita a 100 mensagens
      debugBuffer.erase(debugBuffer.begin());
    }
    // Adiciona a mensagem ao buffer de depuração
    debugBuffer.push_back(msg);
  }
}

void sendDebugBuffer() {
  // Verifica se é hora de enviar o buffer de depuração e se o buffer não está vazio
  if (millis() - lastDebugSend >= debugInterval && !debugBuffer.empty()) {
    String combined;
    for (const auto& line : debugBuffer) {
      combined += line + "\n";
    }
    // Envia o buffer combinado via WebSocket
    webSocket.broadcastTXT(combined);
    // Limpa o buffer de depuração após o envio
    debugBuffer.clear();
    lastDebugSend = millis();
  }
}