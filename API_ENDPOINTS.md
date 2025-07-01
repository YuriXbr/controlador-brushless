# 📋 Referência Rápida das APIs - ESP32 PID Controller

## 🌐 URLs Base
- **WiFi Station:** `http://<IP_DO_ESP32>/`
- **Access Point:** `http://192.168.4.1/`

---

## 📊 **DADOS (GET)**

| Endpoint | Cache | Descrição | Uso Recomendado |
|----------|-------|-----------|-----------------|
| `/allData` | 3s | **🎯 Principal** - Todos os dados | Interface principal |
| `/pidData` | 2s | Dados do controlador PID | Gráficos PID |
| `/motorData` | 1s | Dados do motor | Status motor |
| `/systemStatus` | 5s | Status geral do sistema | Monitoramento |
| `/getPIDValues` | 10s | Valores PID para formulários | Inicialização |
| `/data` | 1s | ⚠️ **DEPRECATED** | Não usar |

**Exemplo:**
```bash
curl http://192.168.1.100/allData
```

---

## ⚙️ **CONFIGURAÇÃO (GET/POST)**

### PID Parameters
```bash
# Configurar PID
GET /set?kp=2.5&ki=1.0&kd=0.5

# Ranges: kp=[0,10], ki=[0,5], kd=[0,10]
```

### Setpoint
```bash
# Definir setpoint
POST /setSetpoint?value=30.0

# Range: value=[-180,180]
```

---

## 🎮 **CONTROLE MANUAL (POST/GET)**

### Modo Manual
```bash
# Alternar modo
POST /toggleManual

# Verificar estado
GET /manualState
# Resposta: "Ativado" ou "Desligado"
```

### PWM Manual (apenas em modo manual)
```bash
# Definir PWM
POST /setPWM?value=1600

# Range: value=[1000,2000]
```

---

## 🔧 **CONTROLE MOTOR (POST/GET)**

```bash
# Ligar/desligar motor
POST /toggleMotor

# Verificar estado
GET /motorState
# Resposta: "Ligado" ou "Desligado"

# Parada de emergência
POST /emergencyStop
```

---

## 🌐 **INTERFACE WEB**

```bash
# Página principal
GET /
```

---

## 🔌 **WEBSOCKET**

```javascript
// Conexão WebSocket
const ws = new WebSocket('ws://192.168.1.100:81');
```

---

## 📝 **CÓDIGOS DE RESPOSTA**

| Código | Significado | Ação |
|--------|-------------|------|
| 200 | ✅ Sucesso | Continuar |
| 400 | ❌ Parâmetros inválidos | Verificar valores |
| 404 | ❌ Rota não encontrada | Verificar URL |
| 500 | ❌ Erro do servidor | Reiniciar ESP32 |

---

## 🧪 **EXEMPLOS RÁPIDOS**

### Configuração Básica
```bash
# 1. Status do sistema
curl http://192.168.1.100/systemStatus

# 2. Configurar PID
curl "http://192.168.1.100/set?kp=2.0&ki=0.8&kd=0.3"

# 3. Definir setpoint
curl -X POST "http://192.168.1.100/setSetpoint?value=0"
```

### Modo Manual
```bash
# 1. Ativar modo manual
curl -X POST http://192.168.1.100/toggleManual

# 2. Controlar PWM
curl -X POST "http://192.168.1.100/setPWM?value=1500"

# 3. Verificar dados
curl http://192.168.1.100/motorData
```

### Monitoramento
```bash
# Dados em tempo real
curl http://192.168.1.100/allData

# Status resumido
curl http://192.168.1.100/systemStatus
```

---

## 🚀 **PERFORMANCE**

### Polling Recomendado
- **Interface principal:** `/allData` a cada 1-2s
- **Gráficos:** `/pidData` a cada 1s  
- **Status:** `/systemStatus` a cada 10s
- **WebSocket:** Para alta frequência

### Cache
- Dados são cacheados no servidor
- Headers HTTP indicam TTL
- Cache é invalidado automaticamente

---

## 🔧 **TROUBLESHOOTING**

### Problemas Comuns
```bash
# Teste de conectividade
ping 192.168.1.100
curl -I http://192.168.1.100/

# PWM não funciona
curl http://192.168.1.100/manualState  # Verificar modo manual

# Erro 500
# Reiniciar ESP32 (reset físico)
```

### Logs de Debug
Monitor Serial (115200 baud):
```
[WEBS] PID updated - Kp=2.5, Ki=1.0, Kd=0.5
[WEBS] Manual mode: ON
[WEBS] Setpoint changed: 30.0
```

---

## 📱 **JAVASCRIPT (Frontend)**

```javascript
// Fetch dados
const data = await fetch('/allData').then(r => r.json());

// Configurar PID
await fetch('/set?kp=2.5&ki=1.0&kd=0.5');

// Definir setpoint
await fetch('/setSetpoint?value=30', { method: 'POST' });

// Alternar modo manual
await fetch('/toggleManual', { method: 'POST' });
```

---

*Referência atualizada em: 2024-12-19*
- **Response**: "Setpoint updated to X"
- **Validation**: value=[-180,180]

### 6. Dados do Sistema (APIs de Dados)

#### API Principal (Recomendada)
- **GET /allData** - Todos os dados do sistema (com cache inteligente)
- **Response**: JSON completo
- **Headers**: Cache-Control: max-age=1
- **Cache**: Atualiza a cada 100ms se houver mudanças

```json
{
  "pid": {
    "kp": 1.5,
    "ki": 0.1,
    "kd": 0.05,
    "setpoint": 15.0,
    "output": 1500,
    "initialized": true
  },
  "sensor": {
    "angle": 12.34,
    "error": false
  },
  "motor": {
    "pwm": 1500,
    "running": true,
    "manual_mode": false,
    "manual_pwm": 1000,
    "stopped": false
  },
  "timing": {
    "loop_freq_hz": 100.0,
    "pid_freq_hz": 50.0,
    "debug_interval": 500
  },
  "system": {
    "uptime": 123456,
    "wifi_connected": true,
    "ap_mode": false,
    "free_heap": 45000,
    "errors": {
      "eeprom": false,
      "mpu": false,
      "motor": false,
      "pid": false,
      "wifi": false,
      "webserver": false
    },
    "emergency_stop": false
  },
  "timestamp": 123456
}
```

#### APIs Específicas
- **GET /pidData** - Apenas dados PID
- **GET /motorData** - Apenas dados do motor
- **GET /systemStatus** - Status condensado do sistema
- **GET /data** - (DEPRECATED) Use /allData

### 7. WebSocket
- **WS ws://ip:81/** - Log em tempo real
- **Status**: Conectado/Desconectado no header da página

## Padrões de Resposta

### Headers Padrão
```
Access-Control-Allow-Origin: *
Cache-Control: max-age=X
Content-Type: application/json (para JSON)
```

### Códigos de Status
- **200**: Sucesso
- **400**: Parâmetros inválidos ou modo incorreto
- **404**: Endpoint não encontrado
- **500**: Erro do servidor

### Idioma
- Todas as respostas de texto estão em português
- Estados: "Ligado"/"Desligado", "Ativado"/"Desligado"
- Erros e mensagens em português

## Cache e Performance

### Cache Inteligente
- `/allData`: Cache de 100ms, invalidado por mudanças
- `/` (página): Cache de 1 hora
- Estados: Cache de 2-5 segundos
- Otimização automática de memória

### Frequência de Atualização
- Interface: Atualiza a cada 1000ms
- Cache interno: 100ms para dados dinâmicos
- Performance da API: Monitorada e exibida

## Segurança e Validação

### Validação de Entrada
- Todos os parâmetros numéricos são validados
- Ranges específicos para cada parâmetro
- Verificação de modo (manual/automático)

### Checks de Segurança
- Motor não liga se houver erros críticos
- Modo manual requerido para PWM manual
- Parada de emergência sempre disponível

### Estados de Erro
- Detecção automática de erros do sistema
- Bloqueio de operações perigosas
- Feedback visual na interface

## Compatibilidade com Interface HTML

### Dados Esperados pela Interface
- Estados em português (interface brasileira)
- Frequências com sufixo "Hz"
- Tempos de resposta em "ms"
- Memória em "KB"
- Performance com cores baseadas na latência

### Endpoints Usados pela Interface
1. `/getPIDValues` - Carregamento inicial dos formulários
2. `/allData` - Atualização principal (1000ms)
3. `/toggleMotor`, `/toggleManual` - Controles principais
4. `/setSetpoint`, `/setPWM` - Configurações
5. `/emergencyStop` - Segurança
6. `/manualState`, `/motorState` - Fallback (deprecated)

Todos os endpoints estão funcionais e otimizados para a interface HTML fornecida.
