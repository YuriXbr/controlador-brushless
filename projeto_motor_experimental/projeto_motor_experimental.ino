#include <Wire.h>
#include <MPU6050_light.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <vector>
#include <EEPROM.h>
#define EEPROM_SIZE 64 // Define o tamanho da EEPROM

std::vector<String> debugBuffer;
unsigned long lastDebugSend = 0;
const unsigned long debugInterval = 200;

MPU6050 mpu(Wire);

const char* ssid = "VIVOFIBRA-4C58";
const char* password = "84EED35225";

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

#define DEBUG 1
#define SETUP_OFFSET 5000
// PID variables
float Kp = 1, Ki = 1, Kd = 1; // Valores iniciais de Kp, Ki, Kd
float setpoint = 0.0;
float integral = 0, lastError = 0;
unsigned long lastTime = 0;
bool manualMode = false;
int manualPWM = 1000;

// ESC PWM config
const int escPin = 18;
const int pwmChannel = 0;
const int freq = 50;
const int resolution = 16;

// Valores atuais para gráficos
float currentAngle = 0;
int currentPWM = 1000;
float currentPIDOutput = 0;

// Funções para ler e escrever valores de ponto flutuante na EEPROM
float readFloatFromEEPROM(int address) {
  float value;
  EEPROM.get(address, value);
  return value;
}

void writeFloatToEEPROM(int address, float value) {
  EEPROM.put(address, value);
}

void debugPrint(String msg) {
  #ifdef DEBUG
    Serial.println(msg);
    if (debugBuffer.size() >= 100) { // Limita a 100 mensagens
      debugBuffer.erase(debugBuffer.begin());
    }
    debugBuffer.push_back(msg);
  #endif
}

void sendDebugBuffer() {
  if (millis() - lastDebugSend >= debugInterval && !debugBuffer.empty()) {
    String combined;
    for (const auto& line : debugBuffer) {
      combined += line + "\n";
    }
    webSocket.broadcastTXT(combined);
    debugBuffer.clear();
    lastDebugSend = millis();
  }
}


uint32_t pulseWidthToDuty(float pulse_us) {
  return (uint32_t)((pulse_us / 20000.0) * ((1 << resolution) - 1));
}

int computePID(float angle) {
  if (manualMode) {
    return manualPWM;
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
  currentPIDOutput = output;

  float pulse = constrain(map(output, -50, 50, 1000, 2000), 1000, 2000);
  return pulse;
}

String getPage() {
  return R"rawliteral(
  <!DOCTYPE html><html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1"/>
    <title>Controle PID</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
      body {
        font-family: Arial, sans-serif;
        background: #f9f9f9;
        padding: 20px;
        max-width: 900px;
        margin: auto;
        color: #222;
      }
      h2, h3 {
        color: #222;
      }
      form, canvas, #serialLog, #manualControl, #statusMessage {
        margin-top: 20px;
        background: #fff;
        padding: 15px;
        border-radius: 8px;
        box-shadow: 0 0 10px rgba(0,0,0,0.1);
      }
      input[type="number"] {
        width: 80px;
        margin-right: 15px;
        padding: 6px;
        border-radius: 4px;
        border: 1px solid #ccc;
      }
      input[type="submit"], button {
        background: #007BFF;
        color: white;
        border: none;
        padding: 10px 18px;
        border-radius: 5px;
        cursor: pointer;
        font-size: 16px;
        transition: background-color 0.3s ease;
      }
      input[type="submit"]:hover, button:hover {
        background: #0056b3;
      }
      #serialLog {
        background: black;
        color: lime;
        height: 180px;
        overflow-y: auto;
        padding: 10px;
        font-family: monospace;
        border-radius: 5px;
        white-space: pre-wrap;
      }
      label {
        font-weight: bold;
        margin-right: 10px;
      }
      #manualControl input[type="range"] {
        width: 200px;
        vertical-align: middle;
      }
      #manualPWMValue {
        font-weight: bold;
        margin-left: 10px;
      }
      #statusMessage {
        color: green;
        font-weight: bold;
        text-align: center;
      }
      @media (max-width: 600px) {
        body {
          padding: 10px;
        }
        #manualControl input[type="range"] {
          width: 100%;
        }
        canvas {
          max-width: 100%;
          height: auto;
        }
      }
    </style>
  </head>
  <body>
    <h2>Controle PID</h2>
    <form action="/set" method="get">
      <label for="kp">Kp:</label>
      <input type="number" id="kp" name="kp" step="0.1" value=")" + String(Kp) + R"rawliteral(">
      <label for="ki">Ki:</label>
      <input type="number" id="ki" name="ki" step="0.01" value=")" + String(Ki) + R"rawliteral(">
      <label for="kd">Kd:</label>
      <input type="number" id="kd" name="kd" step="0.1" value=")" + String(Kd) + R"rawliteral(">
      <input type="submit" value="Atualizar">
    </form>

    <div id="manualControl">
      <h3>Modo Manual</h3>
      <p>Estado: <span id="manualState">Desligado</span></p>
      <button onclick="toggleManual()">Alternar Modo Manual</button><br><br>
      <label for="manualPWM">PWM:</label>
      <input type="range" id="manualPWM" min="1000" max="2000" value="1000" oninput="updatePWM(this.value)">
      <span id="manualPWMValue">1000</span>
    </div>

    <h3>Gráficos em tempo real</h3>
    <canvas id="angleChart" width="400" height="150"></canvas>
    <canvas id="pwmChart" width="400" height="150"></canvas>

    <h3>Serial</h3>
    <div id="serialLog"></div>
    <button onclick="clearLog()">Limpar Log</button>

    <div id="statusMessage"></div>
    <p id="webSocketStatus" style="color: red; font-weight: bold;">Desconectado</p>

    <script>
      const angleCtx = document.getElementById('angleChart').getContext('2d');
      const pwmCtx = document.getElementById('pwmChart').getContext('2d');
      const logDiv = document.getElementById('serialLog');
      const manualStateSpan = document.getElementById('manualState');
      const manualPWMInput = document.getElementById('manualPWM');
      const manualPWMValueSpan = document.getElementById('manualPWMValue');

      const angleChart = new Chart(angleCtx, {
        type: 'line',
        data: { labels: [], datasets: [{ label: 'Ângulo (°)', borderColor: 'blue', backgroundColor: 'rgba(0, 0, 255, 0.1)', data: [], tension: 0.3 }] },
        options: { animation: false, responsive: true, plugins: { legend: { display: true, position: 'top' } } }
      });

      const pwmChart = new Chart(pwmCtx, {
        type: 'line',
        data: { labels: [], datasets: [{ label: 'PWM (us)', borderColor: 'green', backgroundColor: 'rgba(0, 255, 0, 0.1)', data: [], tension: 0.3 }] },
        options: { animation: false, responsive: true, plugins: { legend: { display: true, position: 'top' } } }
      });

      function keepLastPoints(chart, maxPoints = 30) {
        while (chart.data.labels.length > maxPoints) {
          chart.data.labels.shift();
          chart.data.datasets[0].data.shift();
        }
      }

      let ws;
      function connectWebSocket() {
        ws = new WebSocket(`ws://${location.hostname}:81/`);
        ws.onopen = () => {
          document.getElementById('webSocketStatus').textContent = "Conectado";
          document.getElementById('webSocketStatus').style.color = "green";
        };
        ws.onclose = () => {
          document.getElementById('webSocketStatus').textContent = "Desconectado";
          document.getElementById('webSocketStatus').style.color = "red";
          console.warn("WebSocket desconectado. Tentando reconectar...");
          setTimeout(connectWebSocket, 1000);
        };
        ws.onmessage = (event) => {
          logDiv.innerText += event.data + "\n";
          const lines = logDiv.innerText.split('\n');
          if (lines.length > 50) {
            logDiv.innerText = lines.slice(lines.length - 50).join('\n');
          }
          logDiv.scrollTop = logDiv.scrollHeight;
        };
      }
      connectWebSocket();

      function toggleManual() {
        fetch('/toggleManual')
          .then(() => {
            fetch('/manualState')
              .then(r => r.text())
              .then(text => {
                manualStateSpan.textContent = text;
                document.getElementById('statusMessage').textContent = "Modo manual alternado com sucesso!";
              });
          })
          .catch(() => {
            document.getElementById('statusMessage').textContent = "Erro ao alternar modo manual.";
            document.getElementById('statusMessage').style.color = "red";
          });
      }

      let debounceTimeout;
      function updatePWM(val) {
        clearTimeout(debounceTimeout);
        debounceTimeout = setTimeout(() => {
          manualPWMValueSpan.textContent = val;
          fetch('/setPWM?value=' + val)
            .then(() => {
              document.getElementById('statusMessage').textContent = "PWM atualizado com sucesso!";
            })
            .catch(() => {
              document.getElementById('statusMessage').textContent = "Erro ao atualizar PWM.";
              document.getElementById('statusMessage').style.color = "red";
            });
        }, 300);
      }

      function clearLog() {
        logDiv.innerText = "";
      }

      setInterval(async () => {
        try {
          const res = await fetch('/data');
          const json = await res.json();
          const now = new Date().toLocaleTimeString();

          angleChart.data.labels.push(now);
          angleChart.data.datasets[0].data.push(json.angle);
          keepLastPoints(angleChart);
          angleChart.update();

          pwmChart.data.labels.push(now);
          pwmChart.data.datasets[0].data.push(json.pwm);
          keepLastPoints(pwmChart);
          pwmChart.update();

          fetch('/manualState').then(r => r.text()).then(text => manualStateSpan.textContent = text);
        } catch (e) {
          console.error('Erro ao buscar dados:', e);
        }
      }, 1000);
    </script>
  </body>
  </html>
  )rawliteral";
}

void handleRoot() {
  server.send(200, "text/html", getPage());
}

void handleSet() {
  if (server.hasArg("kp")) {
    float newKp = server.arg("kp").toFloat();
    if (newKp >= 0 && newKp <= 10) {
      Kp = newKp;
      EEPROM.writeFloat(0, Kp); // Salva Kp na EEPROM
    }
  }
  if (server.hasArg("ki")) {
    float newKi = server.arg("ki").toFloat();
    if (newKi >= 0 && newKi <= 5) {
      Ki = newKi;
      EEPROM.writeFloat(4, Ki); // Salva Ki na EEPROM
    }
  }
  if (server.hasArg("kd")) {
    float newKd = server.arg("kd").toFloat();
    if (newKd >= 0 && newKd <= 10) {
      Kd = newKd;
      EEPROM.writeFloat(8, Kd); // Salva Kd na EEPROM
    }
  }
  EEPROM.commit(); // Confirma a gravação na EEPROM
  server.send(200, "text/html", getPage());
}

void handleToggleManual() {
  manualMode = !manualMode;
  debugPrint("Modo manual: " + String(manualMode));
  server.send(200, "text/plain", "ok");
}

void handleSetPWM() {
  if (server.hasArg("value")) {
    int val = server.arg("value").toInt();
    manualPWM = constrain(val, 1000, 2000);
  }
  server.send(200, "text/plain", "ok");
}

void handleManualState() {
  server.send(200, "text/plain", manualMode ? "Ativado" : "Desligado");
}

void setupWiFi() {
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 5) {
    delay(1000);
    debugPrint("Conectando ao WiFi...");
    attempts++;
  }

  if (WiFi.status() != WL_CONNECTED) {
    debugPrint("Falha ao conectar. Iniciando AP...");
    WiFi.softAP("ESP32-PID");
    IPAddress IP = WiFi.softAPIP();
    debugPrint("IP do AP: " + IP.toString());
  } else {
    debugPrint("Conectado ao WiFi");
    debugPrint(WiFi.localIP().toString());
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  EEPROM.begin(EEPROM_SIZE); // Inicializa a EEPROM
  Kp = readFloatFromEEPROM(0); // Lê o valor salvo de Kp
  Ki = readFloatFromEEPROM(4); // Lê o valor salvo de Ki
  Kd = readFloatFromEEPROM(8); // Lê o valor salvo de Kd
  setpoint = readFloatFromEEPROM(12); // Lê o valor do setpoint

  if (isnan(Kp)) {
    Kp = 1.5; // Define valores padrão se não houver dados válidos
    writeFloatToEEPROM(0, Kp);
  }
  if (isnan(Ki)) {
    Ki = 1.0; // Define valores padrão se não houver dados válidos
    writeFloatToEEPROM(4, Ki);
  }
  if (isnan(Kd)) {
    Kd = 1.0; // Define valores padrão se não houver dados válidos
    writeFloatToEEPROM(8, Kd);
  }
  if (isnan(setpoint)) {
    setpoint = 0.0; // Define valor padrão para o setpoint
    writeFloatToEEPROM(12, setpoint);
  }
  debugPrint("Kp: " + String(Kp) + ", Ki: " + String(Ki) + ", Kd: " + String(Kd) + ", Setpoint: " + String(setpoint));
  EEPROM.commit(); // Confirma a gravação na EEPROM
  debugPrint("Iniciando PWM no pino " + String(escPin));

  ledcSetup(pwmChannel, freq, resolution);
  ledcAttachPin(escPin, pwmChannel);

  setupWiFi();

  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.on("/toggleManual", handleToggleManual);
  server.on("/setPWM", handleSetPWM);
  server.on("/manualState", handleManualState);
  server.on("/data", []() {
    String json = "{\"angle\":" + String(currentAngle) + ",\"pwm\":" + String(currentPWM) + ",\"pid\":" + String(currentPIDOutput) + "}";
    server.send(200, "application/json", json);
  });

  server.begin();
  webSocket.begin();

  int status = mpu.begin();
  if (status != 0) {
    debugPrint("Erro ao iniciar MPU6050, código: " + String(status));
    while (true) delay(1000);
  }
  mpu.calcOffsets();

  // Envia sinal de PWM mínimo para armar o ESC
  uint32_t dutyMin = pulseWidthToDuty(1000); // 1000µs = mínimo
  ledcWrite(pwmChannel, dutyMin);

  debugPrint("ESPERANDO O MOTOR FICAR PRONTO...");
  delay(5000 + SETUP_OFFSET);  // Aguarda o tempo de inicialização da ESC
  debugPrint("Motor pronto!");
  lastTime = millis();
}

void loop() {
  server.handleClient();
  webSocket.loop();

  static unsigned long lastPIDTime = 0;
  if (millis() - lastPIDTime >= 20) {
    lastPIDTime = millis();
    mpu.update();
    currentAngle = mpu.getAngleX();

    currentPWM = computePID(currentAngle);

    ledcWrite(pwmChannel, pulseWidthToDuty(currentPWM));

    Serial.println("Ângulo: " + String(currentAngle, 2) + " | PWM: " + String(currentPWM));
    sendDebugBuffer();
  }
}
