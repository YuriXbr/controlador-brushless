#ifndef PAGE_CONTENT_H
#define PAGE_CONTENT_H

const char PAGE_CONTENT[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1"/>
    <title>Controle PID</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        body {
            font-family: Arial, sans-serif;
            background: #f9f9f9;
            padding: 20px;
            max-width: 1400px;
            margin: auto;
            color: #222;
        }
        .header {
            display: flex;
            align-items: center;
            justify-content: space-between;
            margin-bottom: 10px;
        }
        .header-title {
            font-size: 2.1rem;
            font-weight: bold;
            letter-spacing: 1px;
        }
        .header-status {
            font-size: 1.1rem;
            font-weight: bold;
            padding: 7px 18px;
            border-radius: 20px;
            background: #fff;
            box-shadow: 0 2px 8px rgba(0,0,0,0.07);
            color: #dc3545;
            min-width: 120px;
            text-align: right;
        }
        .header-status.connected {
            color: #28a745;
        }
        .main-grid {
            display: flex;
            gap: 24px;
            margin-top: 20px;
        }
        .left-panel, .right-panel {
            flex: 1 1 0;
            display: flex;
            flex-direction: column;
            gap: 24px;
        }
        .right-panel {
            flex: 1 1 0;
            display: flex;
            flex-direction: column;
            gap: 24px;
        }        form, #manualControl, #motorControl, #statusMessage, #serialLog, .chart-card, #controlPanel {
            background: #fff;
            padding: 10px 14px;
            border-radius: 10px;
            box-shadow: 0 2px 16px rgba(0,0,0,0.08);
            margin: 0;
            transition: box-shadow 0.2s;
        }
        form:hover, #manualControl:hover, #motorControl:hover, #serialLog:hover, .chart-card:hover, #controlPanel:hover {
            box-shadow: 0 4px 24px rgba(0,123,255,0.10);
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
        .charts-flex {
            display: flex;
            flex-direction: column;
            gap: 24px;
        }
        .chart-card {
            flex: 1 1 0;
            display: flex;
            flex-direction: column;
            align-items: center;
        }
        .chart-card canvas {
            width: 100% !important;
            max-width: 600px;
            height: 260px !important;
        }
        .toast {
            position: fixed;
            bottom: 32px;
            left: 50%;
            transform: translateX(-50%);
            background: #222;
            color: #fff;
            padding: 14px 32px;
            border-radius: 8px;
            font-size: 1.1rem;
            box-shadow: 0 2px 16px rgba(0,0,0,0.18);
            opacity: 0;
            pointer-events: none;
            transition: opacity 0.4s, bottom 0.4s;
            z-index: 9999;
        }
        .toast.show {
            opacity: 1;
            pointer-events: auto;
            bottom: 48px;
        }
        .pause-btn {
            background: #ffc107;
            color: #222;
            margin-left: 10px;
            padding: 4px 12px;
            font-size: 1.1rem;
            border-radius: 50px;
            min-width: 36px;
            min-height: 36px;
            display: inline-flex;
            align-items: center;
            justify-content: center;
            box-shadow: 0 1px 4px rgba(0,0,0,0.07);
        }
        .pause-btn.active {
            background: #28a745;
            color: #fff;
        }
        .pause-btn.pressed {
            filter: brightness(0.92);
        }
        .confirm-modal {
            position: fixed;
            top: 0; left: 0; right: 0; bottom: 0;
            background: rgba(0,0,0,0.25);
            display: flex;
            align-items: center;
            justify-content: center;
            z-index: 10000;
        }
        .confirm-box {
            background: #fff;
            padding: 38px 32px 32px 32px;
            border-radius: 16px;
            box-shadow: 0 6px 32px rgba(220,53,69,0.18), 0 1.5px 8px rgba(0,0,0,0.10);
            text-align: center;
            min-width: 280px;
            max-width: 90vw;
            position: relative;
        }
        .confirm-icon {
            font-size: 2.8rem;
            color: #dc3545;
            margin-bottom: 10px;
            display: block;
        }
        .confirm-title {
            font-size: 1.25rem;
            font-weight: bold;
            color: #dc3545;
            margin-bottom: 8px;
        }
        .confirm-text {
            font-size: 1.05rem;
            color: #333;
            margin-bottom: 22px;
        }
        .confirm-box button {
            margin: 0 12px;
            min-width: 100px;
            padding: 10px 0;
            font-size: 1rem;
            border-radius: 6px;
            font-weight: 600;
        }
        .confirm-box button:first-of-type {
            background: #dc3545;
            color: #fff;
        }
        .confirm-box button:last-of-type {
            background: #e0e0e0;
            color: #222;
        }        .confirm-box button:focus {
            outline: 2px solid #007BFF;
        }
        #controlPanel form {
            background: none !important;
            box-shadow: none !important;
            padding: 0 !important;
            margin: 0 0 24px 0 !important;
        }
        #controlPanel input[type="range"] {
            -webkit-appearance: none;
            appearance: none;
            height: 6px;
            background: #ddd;
            border-radius: 3px;
            outline: none;
        }
        #controlPanel input[type="range"]::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 18px;
            height: 18px;
            background: #007BFF;
            border-radius: 50%;
            cursor: pointer;
        }
        #controlPanel input[type="range"]::-moz-range-thumb {
            width: 18px;
            height: 18px;
            background: #007BFF;
            border-radius: 50%;
            cursor: pointer;
            border: none;
        }
        @media (max-width: 1000px) {
            .main-grid {
                flex-direction: column;
            }
            .right-panel {
                order: -1;
            }
        }        @media (max-width: 600px) {
            body {
                padding: 10px;
            }
            .main-grid, .charts-flex {
                flex-direction: column;
                gap: 12px;
            }
            .right-panel {
                order: -1;
            }
            #systemStatus {
                gap: 12px !important;
                font-size: 0.9rem;
            }
            #systemStatus > div {
                flex: 1 1 45%;
                min-width: 120px;
            }
            #controlPanel h3 {
                font-size: 1.1rem;
            }
            #controlPanel div[style*="display:flex"] {
                flex-wrap: wrap;
                gap: 8px !important;
            }
            #controlPanel input[type="number"] {
                width: 60px !important;
            }
            #controlPanel button {
                min-width: 100px !important;
                font-size: 14px;
                padding: 8px 12px;
            }
        }
            }
            .chart-card canvas {
                max-width: 100%;
                height: auto !important;
            }
            .header-title {
                font-size: 1.3rem;
            }
            .header-status {
                font-size: 0.95rem;
                padding: 5px 10px;
            }
        }
    </style>
</head>
<body>    <div class="header">
        <span class="header-title">Controle PID</span>
        <span id="webSocketStatus" class="header-status">Desconectado</span>
    </div>
    
    <!-- Nova seção de status do sistema -->
    <div id="systemStatus" style="background:#fff;padding:12px 16px;border-radius:8px;box-shadow:0 2px 12px rgba(0,0,0,0.08);margin-bottom:16px;display:flex;gap:24px;flex-wrap:wrap;align-items:center;">
        <div style="display:flex;align-items:center;gap:8px;">
            <span style="font-weight:bold;color:#666;">Sistema:</span>
            <span id="systemStatusText" style="font-weight:bold;color:#28a745;">OK</span>
        </div>
        <div style="display:flex;align-items:center;gap:8px;">
            <span style="font-weight:bold;color:#666;">Uptime:</span>
            <span id="systemUptime" style="font-weight:bold;">00:00:00</span>
        </div>
        <div style="display:flex;align-items:center;gap:8px;">
            <span style="font-weight:bold;color:#666;">Memória:</span>
            <span id="systemMemory" style="font-weight:bold;">--- KB</span>
        </div>
        <div style="display:flex;align-items:center;gap:8px;">
            <span style="font-weight:bold;color:#666;">PID:</span>
            <span id="pidFrequency" style="font-weight:bold;">--- Hz</span>
        </div>        <div style="display:flex;align-items:center;gap:8px;">
            <span style="font-weight:bold;color:#666;">Loop:</span>
            <span id="loopFrequency" style="font-weight:bold;">--- Hz</span>
        </div>        <div style="display:flex;align-items:center;gap:8px;">
            <span style="font-weight:bold;color:#666;">API:</span>
            <span id="apiResponseTime" style="font-weight:bold;color:#17a2b8;">--- ms</span>
        </div>
        <div style="display:flex;align-items:center;gap:8px;">
            <span style="font-weight:bold;color:#666;">Gráficos:</span>
            <span id="chartUpdateRate" style="font-weight:bold;color:#28a745;">5.0 Hz</span>
        </div>
    </div>
    
    <div class="main-grid">
        <div class="right-panel">
            <div class="charts-flex">
                <div class="chart-card">
                    <h3>Ângulo em tempo real
                        <button class="pause-btn" id="pauseAngleBtn" onclick="togglePause('angle')" title="Pausar atualização do gráfico">⏸</button>
                    </h3>
                    <canvas id="angleChart" width="600" height="260"></canvas>
                </div>
                <div class="chart-card">
                    <h3>PWM em tempo real
                        <button class="pause-btn" id="pausePWMBtn" onclick="togglePause('pwm')" title="Pausar atualização do gráfico">⏸</button>
                    </h3>
                    <canvas id="pwmChart" width="600" height="260"></canvas>
                </div>
            </div>            <button id="clearChartsBtn" onclick="clearCharts()" style="margin-top:8px;width:220px;align-self:center;">Limpar Gráficos</button>
              <!-- Controles de tempo real -->
            <div style="margin-top:12px;padding:10px;background:#f8f9fa;border-radius:6px;text-align:center;">
                <label style="font-size:14px;font-weight:bold;color:#555;">Frequência de Atualização:</label>
                <select id="updateFrequency" onchange="changeUpdateFrequency(this.value)" style="margin-left:8px;padding:4px;border-radius:4px;border:1px solid #ccc;">
                    <option value="100">10.0 Hz (100ms) - Ultra Rápido</option>
                    <option value="200" selected>5.0 Hz (200ms) - Tempo Real</option>
                    <option value="500">2.0 Hz (500ms) - Rápido</option>
                    <option value="1000">1.0 Hz (1000ms) - Normal</option>
                    <option value="2000">0.5 Hz (2000ms) - Lento</option>
                </select>
                <br>                <div style="margin-top:8px;font-size:12px;color:#666;">
                    <span style="margin-right:15px;">📊 Polling: 
                        <span id="pollingUpdateRate" style="font-weight:bold;color:#28a745;">5.0 Hz</span>
                    </span>
                    <span style="margin-right:15px;">⚡ WebSocket: 
                        <span id="wsUpdateRate" style="color:#666;font-size:11px;">Inativo</span>
                    </span>
                    <span id="dataSourceIndicator" style="color:#28a745;font-size:10px;">• Polling</span>
                </div>
            </div>
        </div><div class="left-panel">
            <!-- === PAINEL DE CONTROLE UNIFICADO === -->
            <div id="controlPanel" style="background:#fff;padding:16px;border-radius:10px;box-shadow:0 2px 16px rgba(0,0,0,0.08);margin-bottom:16px;">
                <h3 style="margin-top:0;margin-bottom:20px;text-align:center;color:#333;border-bottom:2px solid #f0f0f0;padding-bottom:10px;">Painel de Controle</h3>
                
                <!-- Parâmetros PID -->
                <form action="/set" method="get" style="margin-bottom:24px;">
                    <div style="display:flex;align-items:center;justify-content:space-between;margin-bottom:12px;">
                        <span style="font-weight:bold;color:#555;">Parâmetros PID:</span>
                    </div>
                    <div style="display:flex;align-items:center;gap:15px;margin-bottom:12px;">
                        <label for="kp" style="min-width:30px;">Kp:</label>
                        <input type="number" id="kp" name="kp" step="0.1" value="0" style="width:70px;">
                        <label for="ki" style="min-width:30px;">Ki:</label>
                        <input type="number" id="ki" name="ki" step="0.01" value="0" style="width:70px;">
                        <label for="kd" style="min-width:30px;">Kd:</label>
                        <input type="number" id="kd" name="kd" step="0.1" value="0" style="width:70px;">
                        <input type="submit" value="Atualizar" style="margin-left:auto;">
                    </div>                </form>

                <!-- Diagnósticos PID -->
                <div style="margin-bottom:24px;padding:12px;background:#f8f9fa;border-radius:6px;border-left:4px solid #007BFF;">
                    <div style="display:flex;align-items:center;justify-content:space-between;margin-bottom:12px;">
                        <span style="font-weight:bold;color:#555;">Diagnósticos PID:</span>
                    </div>
                    <div style="display:flex;align-items:center;gap:10px;flex-wrap:wrap;">
                        <button onclick="resetPIDIntegral()" style="background:#ffc107;color:#000;min-width:100px;font-size:14px;">Reset Integral</button>
                        <button onclick="resetCompletePID()" style="background:#dc3545;color:#fff;min-width:100px;font-size:14px;">Reset Completo</button>
                        <span id="pidStatus" style="font-size:12px;color:#666;margin-left:10px;">Status: Normal</span>
                    </div>
                </div>

                <!-- Modo Manual -->
                <div style="margin-bottom:24px;">
                    <div style="display:flex;align-items:center;justify-content:space-between;margin-bottom:12px;">
                        <span style="font-weight:bold;color:#555;">Modo Manual:</span>
                        <span id="manualState" style="font-weight:bold;color:#666;">Desligado</span>
                    </div>
                    <div style="display:flex;align-items:center;gap:15px;">
                        <button onclick="toggleManual()" style="min-width:120px;">Alternar Modo</button>
                        <label for="manualPWM" style="margin:0;">PWM:</label>
                        <input type="range" id="manualPWM" min="1000" max="2000" value="1000" oninput="updatePWM(this.value)" style="flex:1;margin:0 10px;">
                        <span id="manualPWMValue" style="font-weight:bold;min-width:45px;">1000</span>
                    </div>
                </div>                <!-- Setpoint -->
                <div style="margin-bottom:0;">
                    <div style="display:flex;align-items:center;justify-content:space-between;margin-bottom:12px;">
                        <span style="font-weight:bold;color:#555;">Setpoint:</span>
                        <span style="font-weight:bold;color:#666;"><span id="actualSetpointValue">0</span>°</span>
                    </div>                    <div style="display:flex;align-items:center;gap:15px;">
                        <input type="range" id="setpointSlider" min="-30" max="30" step="1" value="0" oninput="updateSetpointSlider(this.value)" style="flex:1;">
                        <span id="setpointSliderValue" style="font-weight:bold;min-width:45px;color:#007BFF;">0°</span>
                        <button id="setSetpointBtn" style="min-width:120px;">Atualizar</button>
                    </div>
                </div>
            </div>

            <div id="motorControl">
                <h3>Motor</h3>
                <p>Estado: <span id="motorState">Desligado</span></p>
                <button onclick="toggleMotor()" id="motorBtn">Iniciar Motor</button>
                <button onclick="emergencyStop()" id="emergencyBtn" style="background:#dc3545;margin-left:10px;">PARADA DE EMERGÊNCIA</button>
            </div>
            <div id="serialLog"></div>
            <button onclick="clearLog()" id="clearLogBtn" style="margin-top:8px;width:220px;align-self:center;">Limpar Log</button>
        </div>
    </div>
    <div id="toast" class="toast"></div>
    <div id="confirmModal" class="confirm-modal" style="display:none;">
        <div class="confirm-box">
            <span class="confirm-icon">&#9888;</span>
            <div class="confirm-title">Atenção!</div>
            <div id="confirmText" class="confirm-text">Deseja realmente ligar o motor?</div>
            <button onclick="confirmMotor(true)">Sim</button>
            <button onclick="confirmMotor(false)">Cancelar</button>
        </div>
    </div>    <script>
      // Element references
      const angleCtx = document.getElementById('angleChart').getContext('2d');
      const pwmCtx = document.getElementById('pwmChart').getContext('2d');
      const logDiv = document.getElementById('serialLog');
      const manualStateSpan = document.getElementById('manualState');
      const manualPWMInput = document.getElementById('manualPWM');
      const manualPWMValueSpan = document.getElementById('manualPWMValue');
      const motorStateSpan = document.getElementById('motorState');
      const motorBtn = document.getElementById('motorBtn');      const setpointSlider = document.getElementById('setpointSlider');
      const setpointSliderValue = document.getElementById('setpointSliderValue');
      const actualSetpointValue = document.getElementById('actualSetpointValue');
      const setSetpointBtn = document.getElementById('setSetpointBtn');      // Global variables
      let ws;
      let debounceTimeout;
      let pauseAngle = false, pausePWM = false;
      let pendingMotorOn = false;
      let confirmInProgress = false;
      let updateInterval = 200; // Default: 200ms for real-time
      let updateIntervalId = null;const angleChart = new Chart(angleCtx, {
        type: 'line',
        data: { 
          labels: [], 
          datasets: [{ 
            label: 'Ângulo (°)', 
            borderColor: 'blue', 
            backgroundColor: 'rgba(0, 0, 255, 0.1)', 
            data: [], 
            tension: 0.2,
            pointRadius: 1,        // Smaller points for better performance
            pointHoverRadius: 3,
            borderWidth: 2
          }] 
        },
        options: { 
          animation: false,      // Disabled for real-time performance
          responsive: true, 
          interaction: {
            intersect: false,
            mode: 'index'
          },
          scales: {
            x: {
              display: false     // Hide x-axis labels for better performance
            },
            y: {
              beginAtZero: false,
              grid: {
                color: 'rgba(0,0,0,0.1)'
              }
            }
          },
          plugins: { 
            legend: { 
              display: true, 
              position: 'top' 
            } 
          } 
        }
      });

      const pwmChart = new Chart(pwmCtx, {
        type: 'line',
        data: { 
          labels: [], 
          datasets: [{ 
            label: 'PWM (us)', 
            borderColor: 'green', 
            backgroundColor: 'rgba(0, 255, 0, 0.1)', 
            data: [], 
            tension: 0.2,
            pointRadius: 1,
            pointHoverRadius: 3,
            borderWidth: 2
          }] 
        },
        options: { 
          animation: false,
          responsive: true,
          interaction: {
            intersect: false,
            mode: 'index'
          },
          scales: {
            x: {
              display: false
            },
            y: {
              beginAtZero: false,
              min: 1000,
              max: 2000,
              grid: {
                color: 'rgba(0,0,0,0.1)'
              }
            }
          },
          plugins: { 
            legend: { 
              display: true, 
              position: 'top' 
            } 
          } 
        }
      });function keepLastPoints(chart, maxPoints = 50) { // Increased from 30 to 50 for real-time
        while (chart.data.labels.length > maxPoints) {
          chart.data.labels.shift();
          chart.data.datasets[0].data.shift();
        }
      }

      function connectWebSocket() {
        ws = new WebSocket(`ws://${location.hostname}:81/`);
        ws.onopen = () => {
          const status = document.getElementById('webSocketStatus');
          status.textContent = "Conectado";
          status.classList.add('connected');
          status.classList.remove('disconnected');
        };
        ws.onclose = () => {
          const status = document.getElementById('webSocketStatus');
          status.textContent = "Desconectado";
          status.classList.remove('connected');
          status.classList.add('disconnected');
          console.warn("WebSocket desconectado. Tentando reconectar...");
          setTimeout(connectWebSocket, 1000);
        };        ws.onmessage = (event) => {
          // Handle both debug messages and real-time data
          try {
            const data = JSON.parse(event.data);
            
            // If it's a data object, update charts immediately
            if(data.angle !== undefined && data.pwm !== undefined) {
              const now = new Date().toLocaleTimeString();
              
              if(!pauseAngle) {
                angleChart.data.labels.push(now);
                angleChart.data.datasets[0].data.push(data.angle);
                keepLastPoints(angleChart, 75); // More points for WebSocket data
                angleChart.update('none');
              }
              
              if(!pausePWM) {
                pwmChart.data.labels.push(now);
                pwmChart.data.datasets[0].data.push(data.pwm);
                keepLastPoints(pwmChart, 75);
                pwmChart.update('none');
              }
              
              // Update WebSocket indicators
              const wsFreqElement = document.getElementById('wsUpdateRate');
              const dataSourceElement = document.getElementById('dataSourceIndicator');
              
              if(wsFreqElement) {
                wsFreqElement.textContent = 'Ativo (~10Hz)';
                wsFreqElement.style.color = '#17a2b8';
              }
              
              if(dataSourceElement) {
                dataSourceElement.textContent = '• WebSocket + Polling';
                dataSourceElement.style.color = '#17a2b8';
              }
              
              // Reset indicator after some time if no more WebSocket data
              clearTimeout(window.wsTimeout);
              window.wsTimeout = setTimeout(() => {
                if(wsFreqElement) {
                  wsFreqElement.textContent = 'Inativo';
                  wsFreqElement.style.color = '#666';
                }
                if(dataSourceElement) {
                  dataSourceElement.textContent = '• Polling';
                  dataSourceElement.style.color = '#28a745';
                }
              }, 3000); // Reset after 3 seconds without WebSocket data
              
              return; // Don't add to log
            }          } catch(e) {
            // Not JSON or invalid JSON, treat as debug message
            console.warn('WebSocket received non-JSON data:', event.data.substring(0, 100));
          }
          
          // Handle debug messages (only add if it's a reasonable text message)
          if (typeof event.data === 'string' && event.data.length > 0 && event.data.length < 1000) {
            logDiv.innerText += event.data + "\n";
            const lines = logDiv.innerText.split('\n');
            if (lines.length > 50) {
              logDiv.innerText = lines.slice(lines.length - 50).join('\n');
            }
            logDiv.scrollTop = logDiv.scrollHeight;
          }
        };
      }
      connectWebSocket();

      function showToast(msg, color = '#222') {
        const toast = document.getElementById('toast');
        toast.textContent = msg;
        toast.style.background = color;
        toast.classList.add('show');
        setTimeout(() => {
          toast.classList.remove('show');
        }, 3500);
      }

      function toggleManual() {
        fetch('/toggleManual')
          .then(() => {
            fetch('/manualState')
              .then(r => r.text())
              .then(text => {
                manualStateSpan.textContent = text;
                showToast("Modo manual alternado com sucesso!", '#007BFF');
              });
          })
          .catch(() => {
            showToast("Erro ao alternar modo manual.", '#dc3545');
          });
      }

      function setBtnLoading(btn, loading) {
        if (loading) {
          btn.disabled = true;
          btn.classList.add('loading');
        } else {
          btn.disabled = false;
          btn.classList.remove('loading');
        }
      }      // Feedback visual para todos os botões
      document.querySelectorAll('button').forEach(btn => {
        btn.addEventListener('mousedown', () => btn.classList.add('pressed'));
        btn.addEventListener('mouseup', () => btn.classList.remove('pressed'));
        btn.addEventListener('mouseleave', () => btn.classList.remove('pressed'));
      });      function updatePWM(val) {
        clearTimeout(debounceTimeout);
        debounceTimeout = setTimeout(() => {
          manualPWMValueSpan.textContent = val;
          fetch('/setPWM?value=' + val)
            .then(async (response) => {
              if (response.ok) {
                const msg = await response.text();
                showToast(msg.includes('PWM set') ? msg : "PWM atualizado com sucesso!", '#007BFF');
              } else {
                const error = await response.text();
                showToast(error, '#dc3545');
              }
            })
            .catch(() => {
              showToast("Erro ao atualizar PWM.", '#dc3545');
            });
        }, 300);
      }

      function updateSetpointSlider(val) {
        console.log('updateSetpointSlider called with:', val); // Debug
        setpointSliderValue.textContent = val + '°';
        
        // Adiciona feedback visual mostrando que há uma mudança pendente
        const currentValue = actualSetpointValue.textContent.replace('°', '');
        if (val != currentValue) {
          setpointSliderValue.style.color = '#ffc107'; // Amarelo para indicar mudança pendente
          setSetpointBtn.style.background = '#ffc107'; // Destaca o botão
          setSetpointBtn.textContent = 'Aplicar';
        } else {
          setpointSliderValue.style.color = '#007BFF'; // Azul normal
          setSetpointBtn.style.background = '#007BFF';
          setSetpointBtn.textContent = 'Atualizar';
        }
      }

      function clearLog() {
        logDiv.innerText = "";
      }

      async function clearCharts() {
        const btn = document.getElementById('clearChartsBtn');
        setBtnLoading(btn, true);
        setTimeout(() => setBtnLoading(btn, false), 10000); // fallback 10s
        try {
          angleChart.data.labels = [];
          angleChart.data.datasets[0].data = [];
          angleChart.update();
          pwmChart.data.labels = [];
          pwmChart.data.datasets[0].data = [];
          pwmChart.update();
          showToast("Gráficos limpos!", '#007BFF');
        } finally {
          setBtnLoading(btn, false);
        }
      }

      function updateMotorUI(state) {
        motorStateSpan.textContent = state;
        if(state === "Ligado") {
          motorBtn.textContent = "Parar Motor";
          motorBtn.style.background = "#dc3545";
        } else {
          motorBtn.textContent = "Iniciar Motor";
          motorBtn.style.background = "#007BFF";
        }      }
      
      function togglePause(type) {
        if(type === 'angle') {
          pauseAngle = !pauseAngle;
          document.getElementById('pauseAngleBtn').classList.toggle('active', pauseAngle);
          document.getElementById('pauseAngleBtn').textContent = pauseAngle ? '▶' : '⏸';
        } else {
          pausePWM = !pausePWM;
          document.getElementById('pausePWMBtn').classList.toggle('active', pausePWM);
          document.getElementById('pausePWMBtn').textContent = pausePWM ? '▶' : '⏸';
        }
      }
      
      function toggleMotor() {
        if (motorBtn.disabled) return;
        if(motorBtn.textContent.includes('Iniciar')) {
          // Confirmação para ligar o motor
          document.getElementById('confirmModal').style.display = 'flex';
          pendingMotorOn = true;
        } else {
          doToggleMotor();
        }
      }
      async function confirmMotor(confirm) {
        if (confirmInProgress) return;
        confirmInProgress = true;
        document.getElementById('confirmModal').style.display = 'none';
        if(confirm && pendingMotorOn) {
          await doToggleMotor();
        }
        pendingMotorOn = false;
        setTimeout(() => { confirmInProgress = false; }, 400); // previne duplo clique
      }      async function doToggleMotor() {
        const btn = document.getElementById('motorBtn');
        setBtnLoading(btn, true);
        btn.blur(); // remove foco do botão para evitar disparo duplo
        let timeout;
        try {
          timeout = setTimeout(() => {
            setBtnLoading(btn, false);
            showToast("Tempo excedido ao alternar motor.", '#dc3545');
          }, 10000);
          const r = await fetch('/toggleMotor');
          if (r.ok) {
            const state = await r.text();
            updateMotorUI(state);
            showToast("Estado do motor alterado!", '#007BFF');
            // Update motor state after a short delay to catch state change
            setTimeout(pollMotorState, 500);
          } else {
            const error = await r.text();
            showToast(error, '#dc3545');
          }
        } catch {
          showToast("Erro ao alternar motor.", '#dc3545');
        } finally {
          clearTimeout(timeout);
          setBtnLoading(btn, false);
        }
      }
      function pollMotorState() {
        fetch('/motorState')
          .then(r => r.text())
          .then(state => updateMotorUI(state));      }
        pollMotorState(); // Só consulta ao carregar a página

      // Event listener do botão setpoint

      setSetpointBtn.addEventListener('click', async () => {
        setSetpointBtn.disabled = true;
        try {
          const val = parseFloat(setpointSlider.value);
          if (val < -180 || val > 180) {
            showToast('Setpoint deve estar entre -180° e 180°', '#dc3545');
            setSetpointBtn.disabled = false;
            return;
          }
          const r = await fetch(`/setSetpoint?value=${val}`);
          if (r.ok) {
            const txt = await r.text();
            actualSetpointValue.textContent = val.toFixed(1);
            // Reset visual feedback
            setpointSliderValue.style.color = '#007BFF';
            setSetpointBtn.style.background = '#007BFF';
            setSetpointBtn.textContent = 'Atualizar';
            showToast(txt && txt.length < 60 ? txt : `Setpoint atualizado para ${val.toFixed(1)}°`, '#28a745');
          } else {
            const error = await r.text();
            showToast(error && error.length < 60 ? error : `Erro HTTP ${r.status} ao atualizar setpoint`, '#dc3545');
          }
        } catch (error) {
          console.error('Erro ao atualizar setpoint:', error);
          showToast('Erro de conexão ao atualizar setpoint', '#dc3545');
        } finally {
          setSetpointBtn.disabled = false;
        }
      });

      // Load initial PID values
      async function loadInitialValues() {
        try {
          const res = await fetch('/getPIDValues');
          const data = await res.json();
          document.getElementById('kp').value = data.kp || 0;
          document.getElementById('ki').value = data.ki || 0;
          document.getElementById('kd').value = data.kd || 0;
          setpointSlider.value = data.setpoint || 0;
          setpointSliderValue.textContent = (data.setpoint || 0) + '°';
          actualSetpointValue.textContent = (data.setpoint || 0).toFixed(1);
        } catch (e) {
          console.warn('Failed to load initial PID values:', e);
          // Set default values if API fails
          document.getElementById('kp').value = 0;
          document.getElementById('ki').value = 0;
          document.getElementById('kd').value = 0;          
          setpointSlider.value = 0;
          setpointSliderValue.textContent = '0°';
          actualSetpointValue.textContent = '0.0';
        }
      }      // Load initial values when page loads
      loadInitialValues();
      
      // Função para atualizar setpoint periodicamente se não estiver na API principal
      async function updateSetpointPeriodically() {
        try {
          const res = await fetch('/getSetpoint');
          if (res.ok) {
            const data = await res.json();
            if (data.setpoint !== undefined) {
              actualSetpointValue.textContent = data.setpoint.toFixed(1);
            }
          }
        } catch (e) {
          // Silently fail - não queremos spam de erros
          console.warn('Falha ao buscar setpoint atual:', e);
        }
      }
      
      // Atualiza setpoint a cada 5 segundos como backup
      setInterval(updateSetpointPeriodically, 5000);
      
      // Emergency stop function
      async function emergencyStop() {
        const btn = document.getElementById('emergencyBtn');
        setBtnLoading(btn, true);
        try {
          const r = await fetch('/emergencyStop');
          const txt = await r.text();
          showToast(txt, '#dc3545');
          // Force immediate status update
          setTimeout(pollMotorState, 100);
        } catch {
          showToast('Erro na parada de emergência', '#dc3545');
        } finally {
          setBtnLoading(btn, false);
        }
      }
      
      // Function to change update frequency dynamically
      function changeUpdateFrequency(newInterval) {
        updateInterval = parseInt(newInterval);
        
        // Clear existing interval
        if (updateIntervalId) {
          clearInterval(updateIntervalId);
        }
        
        // Reset counters and timing arrays
        updateCounter = 0;
        chartUpdateTimes = [];
        
        // Start new interval with updated frequency
        startRealtimeUpdates();
        
        // Show toast confirmation
        const frequency = (1000 / updateInterval).toFixed(1);
        showToast(`Frequência alterada para ${frequency} Hz (${updateInterval}ms)`, '#007BFF');
        
        console.log(`[Frequency] Changed to ${frequency} Hz (${updateInterval}ms interval)`);
      }      
      // Function to start real-time updates
      function startRealtimeUpdates() {
        // Initialize update tracking variables
        let updateCounter = 0;
        let lastChartUpdate = performance.now();
        let chartUpdateTimes = [];
        
        updateIntervalId = setInterval(async () => {
          updateCounter++;
          const startTime = performance.now();
          
          try {
            // High frequency updates for charts
            const res = await fetch('/allData');
            const data = await res.json();
            const apiTime = performance.now() - startTime;
            const now = new Date().toLocaleTimeString();

            // Update charts every cycle for real-time feel
            let chartsUpdated = false;
            if(!pauseAngle && data.sensor) {
              angleChart.data.labels.push(now);
              angleChart.data.datasets[0].data.push(data.sensor.angle);
              keepLastPoints(angleChart, 50); // Keep more points for smooth curves
              angleChart.update('none'); // No animation for smoother real-time
              chartsUpdated = true;
            }

            if(!pausePWM && data.motor) {
              pwmChart.data.labels.push(now);
              pwmChart.data.datasets[0].data.push(data.motor.pwm);
              keepLastPoints(pwmChart, 50);
              pwmChart.update('none');
              chartsUpdated = true;
            }

            // Measure chart update frequency
            if(chartsUpdated) {
              const currentTime = performance.now();
              chartUpdateTimes.push(currentTime);
              
              // Keep only last 10 measurements for rolling average
              if(chartUpdateTimes.length > 10) {
                chartUpdateTimes.shift();
              }
              
              // Calculate frequency every 5 updates
              if(chartUpdateTimes.length >= 5 && updateCounter % 5 === 0) {
                const timeSpan = chartUpdateTimes[chartUpdateTimes.length - 1] - chartUpdateTimes[0];
                const avgInterval = timeSpan / (chartUpdateTimes.length - 1);
                const frequency = 1000 / avgInterval; // Convert to Hz
                  document.getElementById('chartUpdateRate').textContent = `${frequency.toFixed(1)} Hz`;
                document.getElementById('pollingUpdateRate').textContent = `${frequency.toFixed(1)} Hz`;
                
                // Color code based on frequency
                const chartRateElement = document.getElementById('chartUpdateRate');
                const pollingRateElement = document.getElementById('pollingUpdateRate');                if(frequency >= 4.5) {
                  chartRateElement.style.color = '#28a745'; // Green - Good
                  if(pollingRateElement) pollingRateElement.style.color = '#28a745';
                } else if(frequency >= 3.0) {
                  chartRateElement.style.color = '#ffc107'; // Yellow - OK
                  if(pollingRateElement) pollingRateElement.style.color = '#ffc107';
                } else {
                  chartRateElement.style.color = '#dc3545'; // Red - Slow
                  if(pollingRateElement) pollingRateElement.style.color = '#dc3545';
                }
              }
            }

            // Update motor and manual state every cycle
            if(data.motor) {
              manualStateSpan.textContent = data.motor.manual_mode ? 'Ativado' : 'Desligado';
              updateMotorUI(data.motor.running ? 'Ligado' : 'Desligado');
            }

            // Update setpoint display every cycle
            if(data.pid && data.pid.setpoint !== undefined) {
              actualSetpointValue.textContent = data.pid.setpoint.toFixed(1);
            } else if(data.setpoint !== undefined) {
              actualSetpointValue.textContent = data.setpoint.toFixed(1);
            }

            // Lower frequency updates - adjust cycle count based on interval
            const lowFreqCycles = Math.max(1, Math.round(1000 / updateInterval)); // Every ~1 second
            if(updateCounter % lowFreqCycles === 0) {
              // System status updates
              if(data.system) {
                const status = document.getElementById('webSocketStatus');
                const systemStatusText = document.getElementById('systemStatusText');
                const hasErrors = data.system.errors && Object.values(data.system.errors).some(error => error === true);
                
                if(hasErrors) {
                  status.textContent = "Sistema com Erros";
                  status.classList.remove('connected');
                  status.classList.add('disconnected');
                  systemStatusText.textContent = "ERRO";
                  systemStatusText.style.color = "#dc3545";
                } else {
                  status.textContent = "Sistema OK";
                  status.classList.add('connected');
                  status.classList.remove('disconnected');
                  systemStatusText.textContent = "OK";
                  systemStatusText.style.color = "#28a745";
                }

                // System info updates
                const uptimeMs = data.system.uptime;
                const uptimeSeconds = Math.floor(uptimeMs / 1000);
                const hours = Math.floor(uptimeSeconds / 3600);
                const minutes = Math.floor((uptimeSeconds % 3600) / 60);
                const seconds = uptimeSeconds % 60;
                document.getElementById('systemUptime').textContent = 
                  `${hours.toString().padStart(2, '0')}:${minutes.toString().padStart(2, '0')}:${seconds.toString().padStart(2, '0')}`;

                if(data.system.free_heap) {
                  document.getElementById('systemMemory').textContent = `${(data.system.free_heap / 1024).toFixed(1)} KB`;
                }
              }

              // Timing info updates
              if(data.timing) {
                document.getElementById('pidFrequency').textContent = `${data.timing.pid_freq_hz.toFixed(1)} Hz`;
                document.getElementById('loopFrequency').textContent = `${data.timing.loop_freq_hz.toFixed(1)} Hz`;
              }
            }

            // API performance monitoring (every cycle)
            const apiTimeElement = document.getElementById('apiResponseTime');
            apiTimeElement.textContent = `${apiTime.toFixed(1)} ms`;
            // Update color based on performance
            if(apiTime < 50) {
              apiTimeElement.style.color = '#28a745'; // Green - Excellent
            } else if(apiTime < 100) {
              apiTimeElement.style.color = '#17a2b8'; // Blue - Good  
            } else if(apiTime < 200) {
              apiTimeElement.style.color = '#ffc107'; // Yellow - Regular
            } else {
              apiTimeElement.style.color = '#dc3545'; // Red - Slow
            }

            // PID monitoring (every cycle for real-time diagnostics)
            if(data.pid) {
              const pidStatusElement = document.getElementById('pidStatus');
              let pidIssues = [];
              
              // Check for high integral/output
              if (Math.abs(data.pid.output) > 1800) {
                pidIssues.push('Output alto');
              }
              
              // Check for possible stuck integral
              if (Math.abs(data.sensor?.angle - data.pid.setpoint) < 1.0 && 
                  Math.abs(data.pid.output - 1100) > 100) {
                pidIssues.push('Possível integral preso');
              }
              
              if (pidIssues.length > 0) {
                pidStatusElement.textContent = 'Status: ' + pidIssues.join(', ');
                pidStatusElement.style.color = '#dc3545';
              } else if (!pidStatusElement.textContent.includes('resetado') && 
                        !pidStatusElement.textContent.includes('Reset completo')) {
                pidStatusElement.textContent = 'Status: Normal';
                pidStatusElement.style.color = '#28a745';
              }
            }

            // Performance logging (reduced frequency)
            const perfLogCycles = Math.max(5, Math.round(2000 / updateInterval)); // Every ~2 seconds
            if(updateCounter % perfLogCycles === 0) {
              console.log(`[Real-time] API: ${apiTime.toFixed(1)}ms | PID: ${data.pid?.output || 'N/A'} | Angle: ${data.sensor?.angle?.toFixed(2) || 'N/A'}° | PWM: ${data.motor?.pwm || 'N/A'} | Interval: ${updateInterval}ms`);
            }

            // Reset error counter on successful API call
            window.errorCount = 0;

          } catch (e) {
            console.error('Erro ao buscar dados da API otimizada:', e);
            
            // Update connection status to error
            const status = document.getElementById('webSocketStatus');
            status.textContent = "Erro na API";
            status.classList.remove('connected');
            status.classList.add('disconnected');
            
            // FALLBACK: Use legacy API in case of error
            try {
              const res = await fetch('/data'); // DEPRECATED API
              const json = await res.json();
              const now = new Date().toLocaleTimeString();

              if(!pauseAngle) {
                angleChart.data.labels.push(now);
                angleChart.data.datasets[0].data.push(json.angle);
                keepLastPoints(angleChart, 50);
                angleChart.update('none');
              }
              if(!pausePWM) {
                pwmChart.data.labels.push(now);
                pwmChart.data.datasets[0].data.push(json.pwm);
                keepLastPoints(pwmChart, 50);
                pwmChart.update('none');
              }

              fetch('/manualState').then(r => r.text()).then(text => manualStateSpan.textContent = text);
              console.warn('Using deprecated API as fallback');
            } catch (fallbackError) {
              console.error('Error also in fallback API:', fallbackError);
              // Force page reload after consecutive errors (adjusted for interval)
              if (!window.errorCount) window.errorCount = 0;
              window.errorCount++;
              const maxErrors = Math.round(30000 / updateInterval); // 30 seconds worth of errors
              if (window.errorCount > maxErrors) {
                showToast('Muitos erros consecutivos. Recarregando...', '#dc3545');
                setTimeout(() => location.reload(), 3000);
              }
            }
          }
        }, updateInterval);
      }
      
      // Start real-time updates when page loads
      startRealtimeUpdates();
    </script>
</body>
</html>
)rawliteral";

#endif // PAGE_CONTENT_H