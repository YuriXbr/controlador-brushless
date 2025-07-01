# Guia Prático de Testes das APIs - ESP32 PID Controller

Este guia contém exemplos práticos de como testar todas as APIs do sistema de controle PID.

## 🚀 Configuração Inicial

### Encontrar o IP do ESP32
1. **Monitor Serial (115200 baud):** Procure por mensagens como:
   ```
   [WIFI] Connected to WiFi
   [WIFI] IP address: 192.168.1.100
   ```

2. **Roteador:** Procure por dispositivo "ESP32-PID" na lista de dispositivos conectados

3. **Modo AP:** Se não conectar ao WiFi, use `192.168.4.1`

### Ferramentas Recomendadas
- **Browser:** Para interface visual e testes simples
- **cURL:** Para testes automatizados via linha de comando
- **Postman:** Para testes avançados e coleções
- **JavaScript Console:** Para testes dentro da interface web

---

## 🔍 Testes de Conectividade

### Teste Básico de Conectividade
```bash
# Ping básico (substitua pelo IP correto)
ping 192.168.1.100

# Teste HTTP básico
curl -I http://192.168.1.100/
```

**Resultado esperado:**
```
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: [tamanho]
```

### Teste da Interface Web
```bash
# Abrir no browser
http://192.168.1.100/
```

**Resultado esperado:** Interface web do painel de controle PID carregada

---

## 📊 Testes dos Endpoints de Dados

### 1. Teste do Endpoint Principal (`/allData`)
```bash
# Requisição básica
curl http://192.168.1.100/allData

# Com headers de debug
curl -v http://192.168.1.100/allData

# Formatado (se disponível jq)
curl http://192.168.1.100/allData | jq .
```

**Resultado esperado:**
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

### 2. Teste de Dados PID (`/pidData`)
```bash
curl http://192.168.1.100/pidData
```

### 3. Teste de Dados do Motor (`/motorData`)
```bash
curl http://192.168.1.100/motorData
```

### 4. Teste de Status do Sistema (`/systemStatus`)
```bash
curl http://192.168.1.100/systemStatus
```

### 5. Teste de Valores PID Iniciais (`/getPIDValues`)
```bash
curl http://192.168.1.100/getPIDValues
```

---

## ⚙️ Testes de Configuração

### 1. Configurar Parâmetros PID (`/set`)

**Teste individual:**
```bash
# Apenas Kp
curl "http://192.168.1.100/set?kp=3.0"

# Apenas Ki  
curl "http://192.168.1.100/set?ki=1.5"

# Apenas Kd
curl "http://192.168.1.100/set?kd=0.8"
```

**Teste múltiplos parâmetros:**
```bash
# Todos os parâmetros
curl "http://192.168.1.100/set?kp=2.5&ki=1.0&kd=0.5"
```

**Teste de validação (valores inválidos):**
```bash
# Kp muito alto (deve falhar)
curl "http://192.168.1.100/set?kp=15.0"

# Ki negativo (deve falhar)
curl "http://192.168.1.100/set?ki=-1.0"
```

### 2. Configurar Setpoint (`/setSetpoint`)

**Testes válidos:**
```bash
# Setpoint zero
curl -X POST "http://192.168.1.100/setSetpoint?value=0"

# Setpoint positivo
curl -X POST "http://192.168.1.100/setSetpoint?value=45.5"

# Setpoint negativo
curl -X POST "http://192.168.1.100/setSetpoint?value=-30.0"

# Limite máximo
curl -X POST "http://192.168.1.100/setSetpoint?value=180"

# Limite mínimo
curl -X POST "http://192.168.1.100/setSetpoint?value=-180"
```

**Testes de validação:**
```bash
# Acima do limite (deve falhar)
curl -X POST "http://192.168.1.100/setSetpoint?value=200"

# Sem parâmetro (deve falhar)
curl -X POST "http://192.168.1.100/setSetpoint"
```

---

## 🎮 Testes de Controle Manual

### 1. Alternar Modo Manual (`/toggleManual`)

**Sequência de teste:**
```bash
# 1. Verificar estado inicial
curl http://192.168.1.100/manualState

# 2. Alternar para manual
curl -X POST http://192.168.1.100/toggleManual

# 3. Verificar mudança
curl http://192.168.1.100/manualState

# 4. Alternar de volta para automático
curl -X POST http://192.168.1.100/toggleManual

# 5. Verificar estado final
curl http://192.168.1.100/manualState
```

### 2. Controle PWM Manual (`/setPWM`)

**Pré-requisito:** Modo manual deve estar ativo

```bash
# 1. Ativar modo manual
curl -X POST http://192.168.1.100/toggleManual

# 2. Testar PWM central
curl -X POST "http://192.168.1.100/setPWM?value=1500"

# 3. Testar PWM baixo
curl -X POST "http://192.168.1.100/setPWM?value=1100"

# 4. Testar PWM alto
curl -X POST "http://192.168.1.100/setPWM?value=1900"

# 5. Verificar dados do motor
curl http://192.168.1.100/motorData
```

**Testes de validação:**
```bash
# PWM muito baixo (deve falhar)
curl -X POST "http://192.168.1.100/setPWM?value=500"

# PWM muito alto (deve falhar)
curl -X POST "http://192.168.1.100/setPWM?value=2500"

# Modo automático ativo (deve falhar)
curl -X POST http://192.168.1.100/toggleManual  # Desativar manual
curl -X POST "http://192.168.1.100/setPWM?value=1500"  # Deve falhar
```

---

## 🔧 Testes de Controle do Motor

### 1. Ligar/Desligar Motor (`/toggleMotor`)

```bash
# 1. Verificar estado inicial
curl http://192.168.1.100/motorState

# 2. Alternar estado
curl -X POST http://192.168.1.100/toggleMotor

# 3. Verificar mudança
curl http://192.168.1.100/motorState

# 4. Verificar dados completos
curl http://192.168.1.100/motorData
```

### 2. Parada de Emergência (`/emergencyStop`)

```bash
# Executar parada de emergência
curl -X POST http://192.168.1.100/emergencyStop

# Verificar estado após parada
curl http://192.168.1.100/motorState
curl http://192.168.1.100/allData
```

---

## 🧪 Sequências de Teste Completas

### Teste Básico do Sistema
```bash
#!/bin/bash
ESP32_IP="192.168.1.100"
BASE_URL="http://$ESP32_IP"

echo "=== Teste Básico do Sistema ESP32 PID ==="

# 1. Conectividade
echo "1. Testando conectividade..."
curl -s -I $BASE_URL/ > /dev/null && echo "✅ Conectividade OK" || echo "❌ Falha na conectividade"

# 2. Status do sistema
echo "2. Verificando status do sistema..."
curl -s $BASE_URL/systemStatus > /dev/null && echo "✅ Sistema Status OK" || echo "❌ Erro no status"

# 3. Dados principais
echo "3. Verificando dados principais..."
curl -s $BASE_URL/allData > /dev/null && echo "✅ Dados principais OK" || echo "❌ Erro nos dados"

# 4. Configuração PID
echo "4. Testando configuração PID..."
curl -s "$BASE_URL/set?kp=2.0&ki=1.0&kd=0.5" > /dev/null && echo "✅ Configuração PID OK" || echo "❌ Erro na configuração"

# 5. Setpoint
echo "5. Testando setpoint..."
curl -s -X POST "$BASE_URL/setSetpoint?value=0" > /dev/null && echo "✅ Setpoint OK" || echo "❌ Erro no setpoint"

echo "=== Teste concluído ==="
```

### Teste do Modo Manual
```bash
#!/bin/bash
ESP32_IP="192.168.1.100"
BASE_URL="http://$ESP32_IP"

echo "=== Teste do Modo Manual ==="

# 1. Verificar estado inicial
echo "1. Estado inicial:"
curl -s $BASE_URL/manualState

# 2. Ativar modo manual
echo "2. Ativando modo manual..."
curl -s -X POST $BASE_URL/toggleManual
sleep 1

# 3. Verificar ativação
echo "3. Estado após ativação:"
curl -s $BASE_URL/manualState

# 4. Testar controle PWM
echo "4. Testando PWM 1500..."
curl -s -X POST "$BASE_URL/setPWM?value=1500"

echo "5. Testando PWM 1700..."
curl -s -X POST "$BASE_URL/setPWM?value=1700"

echo "6. Testando PWM 1300..."
curl -s -X POST "$BASE_URL/setPWM?value=1300"

# 7. Verificar dados do motor
echo "7. Dados do motor:"
curl -s $BASE_URL/motorData

# 8. Desativar modo manual
echo "8. Desativando modo manual..."
curl -s -X POST $BASE_URL/toggleManual

# 9. Estado final
echo "9. Estado final:"
curl -s $BASE_URL/manualState

echo "=== Teste concluído ==="
```

---

## 🌐 Testes via JavaScript (Console do Browser)

### Dentro da Interface Web

```javascript
// 1. Teste básico de conectividade
async function testarConectividade() {
    try {
        const response = await fetch('/allData');
        const data = await response.json();
        console.log('✅ Conectividade OK:', data);
        return true;
    } catch (error) {
        console.error('❌ Erro de conectividade:', error);
        return false;
    }
}

// 2. Teste de configuração PID
async function testarPID(kp, ki, kd) {
    try {
        const response = await fetch(`/set?kp=${kp}&ki=${ki}&kd=${kd}`);
        console.log(`✅ PID configurado: Kp=${kp}, Ki=${ki}, Kd=${kd}`);
        return response.ok;
    } catch (error) {
        console.error('❌ Erro ao configurar PID:', error);
        return false;
    }
}

// 3. Teste de setpoint
async function testarSetpoint(valor) {
    try {
        const response = await fetch(`/setSetpoint?value=${valor}`, {
            method: 'POST'
        });
        const result = await response.text();
        console.log(`✅ Setpoint definido para ${valor}°:`, result);
        return response.ok;
    } catch (error) {
        console.error('❌ Erro ao definir setpoint:', error);
        return false;
    }
}

// 4. Teste completo
async function testeCompleto() {
    console.log('🚀 Iniciando teste completo...');
    
    await testarConectividade();
    await testarPID(2.5, 1.0, 0.5);
    await testarSetpoint(30);
    
    console.log('✅ Teste completo finalizado!');
}

// Executar teste
testeCompleto();
```

---

## 📱 Testes via Postman

### Coleção Postman para ESP32 PID

**1. Criar nova coleção:** "ESP32 PID Controller"

**2. Configurar variável:**
- `baseUrl`: `http://192.168.1.100` (ajustar conforme seu IP)

**3. Requests da coleção:**

#### GET System Status
- **URL:** `{{baseUrl}}/systemStatus`
- **Method:** GET

#### GET All Data
- **URL:** `{{baseUrl}}/allData`  
- **Method:** GET

#### Set PID Parameters
- **URL:** `{{baseUrl}}/set`
- **Method:** GET
- **Params:** 
  - `kp`: `2.5`
  - `ki`: `1.0`
  - `kd`: `0.5`

#### Set Setpoint
- **URL:** `{{baseUrl}}/setSetpoint`
- **Method:** POST
- **Params:**
  - `value`: `30`

#### Toggle Manual Mode
- **URL:** `{{baseUrl}}/toggleManual`
- **Method:** POST

#### Set Manual PWM
- **URL:** `{{baseUrl}}/setPWM`
- **Method:** POST
- **Params:**
  - `value`: `1600`

### Scripts de Teste Postman

**Pre-request Script (para toda a coleção):**
```javascript
// Verificar conectividade antes de cada request
pm.sendRequest({
    url: pm.variables.get("baseUrl") + "/",
    method: 'GET'
}, function (err, res) {
    if (err) {
        console.log('❌ ESP32 não está acessível');
    } else {
        console.log('✅ ESP32 acessível');
    }
});
```

**Test Script (para /allData):**
```javascript
pm.test("Status code is 200", function () {
    pm.response.to.have.status(200);
});

pm.test("Response is JSON", function () {
    pm.response.to.be.json;
});

pm.test("Contains required fields", function () {
    const jsonData = pm.response.json();
    pm.expect(jsonData).to.have.property('angle');
    pm.expect(jsonData).to.have.property('pwm');
    pm.expect(jsonData).to.have.property('setpoint');
});

pm.test("Angle is within range", function () {
    const jsonData = pm.response.json();
    pm.expect(jsonData.angle).to.be.within(-180, 180);
});
```

---

## 🔧 Troubleshooting

### Problemas Comuns e Soluções

**1. Não consegue acessar o IP**
```bash
# Verificar conectividade
ping 192.168.1.100

# Verificar se ESP32 está no modo AP
# Conectar ao WiFi "ESP32-PID" e usar IP 192.168.4.1
```

**2. Timeout nas requisições**
```bash
# Adicionar timeout maior
curl --max-time 10 http://192.168.1.100/allData

# Verificar no Monitor Serial se há logs de erro
```

**3. Erro 500 - Internal Server Error**
```bash
# Reiniciar ESP32 (botão reset ou power cycle)
# Verificar logs via Monitor Serial
```

**4. PWM não funciona em modo manual**
```bash
# Verificar se modo manual está ativo
curl http://192.168.1.100/manualState

# Ativar modo manual antes de definir PWM
curl -X POST http://192.168.1.100/toggleManual
curl -X POST "http://192.168.1.100/setPWM?value=1500"
```

### Monitoramento de Performance

**JavaScript para medir latência:**
```javascript
async function medirLatencia() {
    const inicio = performance.now();
    await fetch('/allData');
    const fim = performance.now();
    console.log(`Latência: ${(fim - inicio).toFixed(1)}ms`);
}

// Executar 10 vezes
for(let i = 0; i < 10; i++) {
    await medirLatencia();
    await new Promise(resolve => setTimeout(resolve, 1000));
}
```

---

## 📋 Checklist de Validação

### ✅ Teste Básico (Obrigatório)
- [ ] Conectividade HTTP básica (`curl -I http://IP/`)
- [ ] Interface web carrega (`http://IP/`)
- [ ] Endpoint `/allData` responde
- [ ] Configuração PID funciona
- [ ] Setpoint funciona

### ✅ Teste Intermediário
- [ ] Modo manual funciona
- [ ] Controle PWM manual funciona
- [ ] Toggle motor funciona
- [ ] Parada de emergência funciona
- [ ] Cache funciona (verificar headers)

### ✅ Teste Avançado
- [ ] Validação de parâmetros funciona
- [ ] WebSocket conecta
- [ ] Performance adequada (<100ms)
- [ ] Logs de debug aparecem
- [ ] Sistema recupera de erros

---

*Guia de testes atualizado em: 2024-12-19*
