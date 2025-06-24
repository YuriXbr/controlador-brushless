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
        }
        form, #manualControl, #motorControl, #statusMessage, #serialLog, .chart-card {
            background: #fff;
            padding: 10px 14px;
            border-radius: 10px;
            box-shadow: 0 2px 16px rgba(0,0,0,0.08);
            margin: 0;
            transition: box-shadow 0.2s;
        }
        form:hover, #manualControl:hover, #motorControl:hover, #serialLog:hover, .chart-card:hover {
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
        }
        .confirm-box button:focus {
            outline: 2px solid #007BFF;
        }
        @media (max-width: 1000px) {
            .main-grid {
                flex-direction: column;
            }
            .right-panel {
                order: -1;
            }
        }
        @media (max-width: 600px) {
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
<body>
    <div class="header">
        <span class="header-title">Controle PID</span>
        <span id="webSocketStatus" class="header-status">Desconectado</span>
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
            </div>
            <button id="clearChartsBtn" onclick="clearCharts()" style="margin-top:8px;width:220px;align-self:center;">Limpar Gráficos</button>
        </div>
        <div class="left-panel">
            <form action="/set" method="get">
                <label for="kp">Kp:</label>
                <input type="number" id="kp" name="kp" step="0.1" value="0">
                <label for="ki">Ki:</label>
                <input type="number" id="ki" name="ki" step="0.01" value="0">
                <label for="kd">Kd:</label>
                <input type="number" id="kd" name="kd" step="0.1" value="0">
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
            <div id="motorControl">
                <h3>Motor</h3>
                <p>Estado: <span id="motorState">Desligado</span></p>
                <button onclick="toggleMotor()" id="motorBtn">Iniciar Motor</button>
            </div>
            <div id="serialLog"></div>
            <button onclick="clearLog()" id="clearLogBtn" style="margin-top:8px;width:220px;align-self:center;">Limpar Log</button>
            <div id="setpointControl" style="background:#fff;padding:10px 14px;border-radius:10px;box-shadow:0 2px 16px rgba(0,0,0,0.08);margin-bottom:16px;">
                <h3>Setpoint</h3>
                <input type="range" id="setpointSlider" min="0" max="30" step="1" value="0" style="width:200px;">
                <span id="setpointValue">0</span>
                <button id="setSetpointBtn" style="margin-left:10px;">Atualizar</button>
            </div>
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
    </div>

    <script>
      const angleCtx = document.getElementById('angleChart').getContext('2d');
      const pwmCtx = document.getElementById('pwmChart').getContext('2d');
      const logDiv = document.getElementById('serialLog');
      const manualStateSpan = document.getElementById('manualState');
      const manualPWMInput = document.getElementById('manualPWM');
      const manualPWMValueSpan = document.getElementById('manualPWMValue');
      const motorStateSpan = document.getElementById('motorState');
      const motorBtn = document.getElementById('motorBtn');

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
      }
      // Feedback visual para todos os botões
      document.querySelectorAll('button').forEach(btn => {
        btn.addEventListener('mousedown', () => btn.classList.add('pressed'));
        btn.addEventListener('mouseup', () => btn.classList.remove('pressed'));
        btn.addEventListener('mouseleave', () => btn.classList.remove('pressed'));
      });

      let debounceTimeout;
      function updatePWM(val) {
        clearTimeout(debounceTimeout);
        debounceTimeout = setTimeout(() => {
          manualPWMValueSpan.textContent = val;
          fetch('/setPWM?value=' + val)
            .then(() => {
              showToast("PWM atualizado com sucesso!", '#007BFF');
            })
            .catch(() => {
              showToast("Erro ao atualizar PWM.", '#dc3545');
            });
        }, 300);
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
        }
      }
      let pauseAngle = false, pausePWM = false;
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
      let pendingMotorOn = false;
      let confirmInProgress = false;
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
      }
      async function doToggleMotor() {
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
          const state = await r.text();
          updateMotorUI(state);
          showToast("Estado do motor alterado!", '#007BFF');
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
          .then(state => updateMotorUI(state));
      }
      pollMotorState(); // Só consulta ao carregar a página

      // --- Slider de setpoint ---
      const setpointSlider = document.getElementById('setpointSlider');
      const setpointValue = document.getElementById('setpointValue');
      const setSetpointBtn = document.getElementById('setSetpointBtn');
      setpointSlider.addEventListener('input', () => {
        setpointValue.textContent = setpointSlider.value;
      });
      setSetpointBtn.addEventListener('click', async () => {
        setSetpointBtn.disabled = true;
        try {
          const val = setpointSlider.value;
          const r = await fetch(`/setSetpoint?value=${val}`);
          const txt = await r.text();
          showToast(txt, '#007BFF');
        } catch {
          showToast('Erro ao atualizar setpoint', '#dc3545');
        } finally {
          setSetpointBtn.disabled = false;
        }
      });

      setInterval(async () => {
        try {
          const res = await fetch('/data');
          const json = await res.json();
          const now = new Date().toLocaleTimeString();

          if(!pauseAngle) {
            angleChart.data.labels.push(now);
            angleChart.data.datasets[0].data.push(json.angle);
            keepLastPoints(angleChart);
            angleChart.update();
          }
          if(!pausePWM) {
            pwmChart.data.labels.push(now);
            pwmChart.data.datasets[0].data.push(json.pwm);
            keepLastPoints(pwmChart);
            pwmChart.update();
          }

          fetch('/manualState').then(r => r.text()).then(text => manualStateSpan.textContent = text);
        } catch (e) {
          console.error('Erro ao buscar dados:', e);
        }
      }, 1000);
    </script>
</body>
</html>
)rawliteral";

#endif // PAGE_CONTENT_H