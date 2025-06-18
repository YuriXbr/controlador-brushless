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

#endif // PAGE_CONTENT_H