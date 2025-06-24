#include "debugHandler.h"
#include <vector>
#include <WebSocketsServer.h>
#include "configs.h"

extern WebSocketsServer webSocket;  // declaração para acesso ao webSocket

std::vector<String> debugBuffer;
unsigned long lastDebugSend = 0;
int repeatedCount1 = 1, repeatedCount2 = 1;
String lastMsg1 = "", lastMsg2 = "";

void debugPrint(String msg) {
  if(DEBUG) {
    // Se igual à última mensagem
    if (!debugBuffer.empty() && (msg == lastMsg1 || msg == ("[" + String(repeatedCount1) + "] " + lastMsg1))) {
      repeatedCount1++;
      debugBuffer.pop_back();
      debugBuffer.push_back("[" + String(repeatedCount1) + "] " + lastMsg1);
    }
    // Se igual à penúltima mensagem
    else if (debugBuffer.size() > 1 && (msg == lastMsg2 || msg == ("[" + String(repeatedCount2) + "] " + lastMsg2))) {
      repeatedCount2++;
      debugBuffer.pop_back(); // Remove a última
      debugBuffer.pop_back(); // Remove a penúltima
      debugBuffer.push_back("[" + String(repeatedCount2) + "] " + lastMsg2);
      // Atualiza o histórico
      lastMsg1 = lastMsg2;
      repeatedCount1 = repeatedCount2;
      lastMsg2 = "";
      repeatedCount2 = 1;
    } else {
      // Nova mensagem diferente das duas últimas
      lastMsg2 = lastMsg1;
      repeatedCount2 = repeatedCount1;
      lastMsg1 = msg;
      repeatedCount1 = 1;
      debugBuffer.push_back(msg);
    }
    Serial.println(msg);
    if (debugBuffer.size() > 500) {
      debugBuffer.erase(debugBuffer.begin());
    }
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