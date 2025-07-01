# Documentação da API do Sistema de Controle PID para ESP32

Esta documentação descreve todas as rotas disponíveis na API REST do sistema de controle PID para balanceamento com ESP32/Arduino.

## Informações Gerais

- **Servidor:** ESP32 executando na porta 80
- **Protocolo:** HTTP/REST
- **Formato de resposta:** JSON para dados, text/plain para comandos simples
- **CORS:** Habilitado para todas as rotas (`Access-Control-Allow-Origin: *`)
- **Cache:** Implementado com diferentes TTLs dependendo da rota

## Base URL
- **Modo Station (WiFi):** `http://<IP_DO_ESP32>/`
- **Modo Access Point:** `http://192.168.4.1/`

---

## 📊 Endpoints de Dados

### GET `/allData`
**Descrição:** Retorna todos os dados do sistema em formato JSON (endpoint principal recomendado)

**Cache:** 3 segundos

**Resposta:**
```json
{
  "angle": 12.34,
  "pwm": 1500,
  "pid": 85.67,
  "kp": 2.5,
  "ki": 1.2,
  "kd": 0.8,
  "setpoint": 0.0,
  "manual_mode": false,
  "manual_pwm": 1500,
  "motor_running": true,
  "motor_stopped": false,
  "wifi_connected": true,
  "timestamp": 123456789
}
```

**Exemplo de uso:**
```bash
# cURL
curl http://192.168.1.100/allData

# Browser
http://192.168.1.100/allData
```

### GET `/pidData`
**Descrição:** Retorna dados específicos do controlador PID

**Cache:** 2 segundos

**Resposta:**
```json
{
  "kp": 2.5,
  "ki": 1.2,
  "kd": 0.8,
  "setpoint": 0.0,
  "output": 85.67,
  "angle": 12.34,
  "initialized": true,
  "timestamp": 123456789
}
```

**Exemplo de uso:**
```bash
curl http://192.168.1.100/pidData
```

### GET `/motorData`
**Descrição:** Retorna dados específicos do motor

**Cache:** 1 segundo

**Resposta:**
```json
{
  "pwm": 1500,
  "running": true,
  "manual_mode": false,
  "manual_pwm": 1500,
  "stopped": false,
  "timestamp": 123456789
}
```

**Exemplo de uso:**
```bash
curl http://192.168.1.100/motorData
```

### GET `/getPIDValues`
**Descrição:** Retorna valores atuais dos parâmetros PID (para inicialização de formulários)

**Cache:** 10 segundos

**Resposta:**
```json
{
  "kp": 2.5,
  "ki": 1.2,
  "kd": 0.8,
  "setpoint": 0.0
}
```

### GET `/systemStatus`
**Descrição:** Retorna status geral do sistema

**Cache:** 5 segundos

**Resposta:**
```json
{
  "wifi_connected": true,
  "wifi_ssid": "MinhaRede",
  "wifi_ip": "192.168.1.100",
  "wifi_signal": -45,
  "uptime": 123456789,
  "free_heap": 245760,
  "webserver_initialized": true,
  "websocket_initialized": true,
  "motor_initialized": true,
  "pid_initialized": true,
  "gyro_initialized": true,
  "timestamp": 123456789
}
```

### GET `/data` ⚠️ **DEPRECATED**
**Descrição:** Endpoint legado - use `/allData` ao invés deste

---

## ⚙️ Endpoints de Configuração

### GET/POST `/set`
**Descrição:** Define parâmetros PID (Kp, Ki, Kd)

**Parâmetros (Query String):**
- `kp` (float): Ganho proporcional (0-10)
- `ki` (float): Ganho integral (0-5)  
- `kd` (float): Ganho derivativo (0-10)

**Validação:**
- Valores devem estar dentro dos ranges especificados
- Valores são salvos na EEPROM automaticamente

**Resposta:** HTML da página principal (GET) ou status de sucesso

**Exemplos de uso:**
```bash
# Definir apenas Kp
curl "http://192.168.1.100/set?kp=3.5"

# Definir múltiplos parâmetros
curl "http://192.168.1.100/set?kp=2.5&ki=1.0&kd=0.5"

# Via browser
http://192.168.1.100/set?kp=2.0&ki=0.8&kd=0.3
```

### POST `/setSetpoint`
**Descrição:** Define o setpoint (ângulo alvo) do sistema

**Parâmetros:**
- `value` (float): Valor do setpoint em graus (-180 a 180)

**Resposta:** `text/plain` com confirmação ou erro

**Exemplos de uso:**
```bash
# cURL
curl -X POST "http://192.168.1.100/setSetpoint?value=45.5"

# Via fetch (JavaScript)
fetch('/setSetpoint?value=30')
  .then(response => response.text())
  .then(data => console.log(data));
```

---

## 🎮 Endpoints de Controle Manual

### POST `/toggleManual`
**Descrição:** Alterna entre modo automático (PID) e manual

**Resposta:** `"ok"` se bem-sucedido

**Comportamento:**
- Reseta o PID quando muda de modo
- Invalida cache automaticamente

**Exemplo de uso:**
```bash
curl -X POST http://192.168.1.100/toggleManual
```

### GET `/manualState`
**Descrição:** Consulta o estado atual do modo manual

**Cache:** 5 segundos

**Resposta:** 
- `"Ativado"` - Modo manual ativo
- `"Desligado"` - Modo automático (PID) ativo

**Exemplo de uso:**
```bash
curl http://192.168.1.100/manualState
```

### POST `/setPWM`
**Descrição:** Define PWM manual do motor (apenas em modo manual)

**Parâmetros:**
- `value` (int): Valor PWM (1000-2000)

**Validação:**
- Só funciona se o modo manual estiver ativo
- Valor deve estar entre 1000-2000

**Resposta:** Confirmação com valor definido ou erro

**Exemplos de uso:**
```bash
# Definir PWM para 1750
curl -X POST "http://192.168.1.100/setPWM?value=1750"

# Exemplo com validação de erro
curl -X POST "http://192.168.1.100/setPWM?value=2500"
# Retorna: PWM value out of range (1000-2000)
```

---

## 🔧 Endpoints de Controle do Motor

### POST `/toggleMotor`
**Descrição:** Liga/desliga o motor principal

**Resposta:** `"ok"` se bem-sucedido

**Exemplo de uso:**
```bash
curl -X POST http://192.168.1.100/toggleMotor
```

### GET `/motorState`
**Descrição:** Consulta o estado atual do motor

**Resposta:** 
- `"Ligado"` - Motor está rodando
- `"Desligado"` - Motor está parado

**Exemplo de uso:**
```bash
curl http://192.168.1.100/motorState
```

### POST `/emergencyStop`
**Descrição:** Para o motor imediatamente (parada de emergência)

**Resposta:** `"Emergency stop activated"`

**Comportamento:**
- Para o motor instantaneamente
- Limpa cache
- Log de debug é gerado

**Exemplo de uso:**
```bash
curl -X POST http://192.168.1.100/emergencyStop
```

---

## 🌐 Interface Web

### GET `/`
**Descrição:** Página principal da interface web

**Resposta:** HTML completo da interface de controle

**Recursos incluídos:**
- Painel de controle PID unificado
- Controles manuais de PWM
- Monitoramento em tempo real
- Gráficos de resposta
- Interface responsiva

**Exemplo de uso:**
```bash
# Acessar via browser
http://192.168.1.100/
```

---

## 🔌 WebSocket

### Porta 81
**Descrição:** Conexão WebSocket para atualizações em tempo real

**Endpoint:** `ws://<IP_DO_ESP32>:81/`

**Exemplo de uso (JavaScript):**
```javascript
const socket = new WebSocket('ws://192.168.1.100:81');

socket.onmessage = function(event) {
    const data = JSON.parse(event.data);
    console.log('Dados recebidos:', data);
};

socket.onerror = function(error) {
    console.error('Erro WebSocket:', error);
};
```

---

## � Códigos de Status HTTP

| Código | Descrição | Situação |
|--------|-----------|----------|
| 200 | OK | Requisição bem-sucedida |
| 400 | Bad Request | Parâmetros inválidos ou em falta |
| 404 | Not Found | Rota não encontrada |
| 500 | Internal Server Error | Erro do servidor (não inicializado) |

---

## 🧪 Exemplos de Teste Completos

### Teste do Fluxo PID Completo
```bash
# 1. Verificar status do sistema
curl http://192.168.1.100/systemStatus

# 2. Configurar parâmetros PID
curl "http://192.168.1.100/set?kp=2.5&ki=1.0&kd=0.5"

# 3. Definir setpoint
curl -X POST "http://192.168.1.100/setSetpoint?value=30"

# 4. Verificar dados PID
curl http://192.168.1.100/pidData

# 5. Monitorar dados gerais
curl http://192.168.1.100/allData
```

### Teste do Modo Manual
```bash
# 1. Ativar modo manual
curl -X POST http://192.168.1.100/toggleManual

# 2. Verificar se está ativo
curl http://192.168.1.100/manualState

# 3. Definir PWM manual
curl -X POST "http://192.168.1.100/setPWM?value=1600"

# 4. Verificar dados do motor
curl http://192.168.1.100/motorData

# 5. Desativar modo manual
curl -X POST http://192.168.1.100/toggleManual
```

### Teste com Postman
```
Collection: ESP32 PID Controller

Request 1: Get System Status
- Method: GET
- URL: {{baseUrl}}/systemStatus
- Headers: None

Request 2: Set PID Parameters  
- Method: GET
- URL: {{baseUrl}}/set
- Params: kp=2.5, ki=1.0, kd=0.5

Request 3: Set Setpoint
- Method: POST  
- URL: {{baseUrl}}/setSetpoint
- Params: value=45

Variables:
- baseUrl: http://192.168.1.100
```

---

## 🛠️ Recursos Avançados

### Cache Inteligente
- Cache automático com TTLs diferentes por endpoint
- Invalidação automática quando dados mudam
- Otimização de memória automática

### Tratamento de Erros
- Validação de parâmetros
- Verificação de estado do servidor
- Logs de debug automáticos

### Segurança
- Validação de ranges para todos os parâmetros
- Verificação de estado antes da execução
- Parada de emergência sempre disponível

---

## � Integração com Frontend

### JavaScript Fetch API
```javascript
// Atualizar parâmetros PID
async function updatePID(kp, ki, kd) {
    try {
        const response = await fetch(`/set?kp=${kp}&ki=${ki}&kd=${kd}`);
        return response.ok;
    } catch (error) {
        console.error('Erro ao atualizar PID:', error);
        return false;
    }
}

// Definir setpoint
async function setSetpoint(value) {
    try {
        const response = await fetch(`/setSetpoint?value=${value}`, {
            method: 'POST'
        });
        const result = await response.text();
        return { success: response.ok, message: result };
    } catch (error) {
        return { success: false, message: error.message };
    }
}

// Monitorar dados em tempo real
async function fetchData() {
    try {
        const response = await fetch('/allData');
        const data = await response.json();
        return data;
    } catch (error) {
        console.error('Erro ao buscar dados:', error);
        return null;
    }
}
```

### Polling Recomendado
- `/allData`: Cada 1-2 segundos para interface principal
- `/systemStatus`: Cada 10-30 segundos para status geral  
- `/pidData`: Cada 1 segundo para gráficos em tempo real
- Use WebSocket para atualizações mais frequentes

---

## 🔧 Troubleshooting

### Problemas Comuns

**Erro 500 - Server Error**
- Servidor não inicializado
- Verificar conexão WiFi
- Reiniciar ESP32

**Erro 400 - Bad Request**
- Parâmetros faltando ou inválidos
- Verificar ranges dos valores
- Modo manual inativo para comandos PWM

**Sem resposta**
- Verificar IP do ESP32
- Verificar conexão de rede
- ESP32 pode ter travado (reset necessário)

### Logs de Debug
Os logs podem ser monitorados via Serial Monitor (115200 baud) para troubleshooting:
```
[WEBS] PID updated - Kp=2.5, Ki=1.0, Kd=0.5
[WEBS] Manual mode: ON
[WEBS] Manual PWM set to: 1600
[WEBS] Setpoint changed: 30.0
[WEBS] Emergency stop triggered via web interface
```

---

## 📋 Resumo das Rotas

| Rota | Método | Função | Cache |
|------|--------|--------|-------|
| `/` | GET | Interface web principal | - |
| `/allData` | GET | Todos os dados do sistema | 3s |
| `/pidData` | GET | Dados específicos do PID | 2s |
| `/motorData` | GET | Dados específicos do motor | 1s |
| `/getPIDValues` | GET | Valores PID atuais | 10s |
| `/systemStatus` | GET | Status geral do sistema | 5s |
| `/set` | GET/POST | Definir parâmetros PID | - |
| `/setSetpoint` | POST | Definir setpoint | - |
| `/toggleManual` | POST | Alternar modo manual | - |
| `/manualState` | GET | Estado do modo manual | 5s |
| `/setPWM` | POST | Definir PWM manual | - |
| `/toggleMotor` | POST | Ligar/desligar motor | - |
| `/motorState` | GET | Estado do motor | - |
| `/emergencyStop` | POST | Parada de emergência | - |
| `/resetPIDIntegral` | POST | Reset do termo integral | - |
| `/resetPID` | POST | Reset completo do PID | - |
| `/data` | GET | **DEPRECATED** | 1s |

---

*Documentação gerada em: 2024-12-19*  
*Versão do sistema: ESP32 PID Controller v1.0*
