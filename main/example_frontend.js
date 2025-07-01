// Exemplo de como usar as novas APIs otimizadas do servidor web

class RobotController {
    constructor(baseUrl = 'http://192.168.1.100') {
        this.baseUrl = baseUrl;
        this.updateInterval = null;
        this.lastUpdate = 0;
    }

    // Função principal - obtém todos os dados de uma vez
    async getAllData() {
        try {
            const response = await fetch(`${this.baseUrl}/allData`);
            if (!response.ok) throw new Error(`HTTP ${response.status}`);
            return await response.json();
        } catch (error) {
            console.error('Erro ao obter todos os dados:', error);
            return null;
        }
    }

    // Status condensado do sistema (mais rápido)
    async getSystemStatus() {
        try {
            const response = await fetch(`${this.baseUrl}/systemStatus`);
            if (!response.ok) throw new Error(`HTTP ${response.status}`);
            return await response.json();
        } catch (error) {
            console.error('Erro ao obter status do sistema:', error);
            return null;
        }
    }

    // Dados específicos (para uso quando necessário)
    async getPIDData() {
        try {
            const response = await fetch(`${this.baseUrl}/pidData`);
            if (!response.ok) throw new Error(`HTTP ${response.status}`);
            return await response.json();
        } catch (error) {
            console.error('Erro ao obter dados do PID:', error);
            return null;
        }
    }

    async getMotorData() {
        try {
            const response = await fetch(`${this.baseUrl}/motorData`);
            if (!response.ok) throw new Error(`HTTP ${response.status}`);
            return await response.json();
        } catch (error) {
            console.error('Erro ao obter dados do motor:', error);
            return null;
        }
    }

    // Atualização automática otimizada
    startAutoUpdate(intervalMs = 1000) {
        this.stopAutoUpdate();
        
        this.updateInterval = setInterval(async () => {
            const startTime = performance.now();
            
            // Usa getAllData para obter tudo de uma vez
            const data = await this.getAllData();
            
            if (data) {
                const updateTime = performance.now() - startTime;
                console.log(`Dados atualizados em ${updateTime.toFixed(1)}ms`);
                
                // Atualiza a interface
                this.updateUI(data);
                
                // Armazena timestamp da última atualização
                this.lastUpdate = data.timestamp;
            }
        }, intervalMs);
    }

    stopAutoUpdate() {
        if (this.updateInterval) {
            clearInterval(this.updateInterval);
            this.updateInterval = null;
        }
    }

    // Atualização da interface (exemplo)
    updateUI(data) {
        // Atualiza dados do PID
        if (data.pid) {
            document.getElementById('kp-value').textContent = data.pid.kp.toFixed(3);
            document.getElementById('ki-value').textContent = data.pid.ki.toFixed(3);
            document.getElementById('kd-value').textContent = data.pid.kd.toFixed(3);
            document.getElementById('setpoint-value').textContent = data.pid.setpoint.toFixed(2);
            document.getElementById('pid-output').textContent = data.pid.output;
        }

        // Atualiza dados do sensor
        if (data.sensor) {
            document.getElementById('angle-value').textContent = data.sensor.angle.toFixed(2) + '°';
        }

        // Atualiza dados do motor
        if (data.motor) {
            document.getElementById('pwm-value').textContent = data.motor.pwm;
            document.getElementById('motor-status').textContent = data.motor.running ? 'Ligado' : 'Desligado';
            document.getElementById('manual-mode').textContent = data.motor.manual_mode ? 'Manual' : 'Automático';
        }

        // Atualiza status do sistema
        if (data.system) {
            document.getElementById('uptime').textContent = this.formatUptime(data.system.uptime);
            document.getElementById('free-heap').textContent = (data.system.free_heap / 1024).toFixed(1) + ' KB';
            
            // Atualiza indicadores de erro
            const errorElements = document.querySelectorAll('.error-indicator');
            errorElements.forEach(el => {
                const errorType = el.dataset.errorType;
                if (data.system.errors[errorType]) {
                    el.classList.add('error');
                } else {
                    el.classList.remove('error');
                }
            });
        }

        // Atualiza frequências
        if (data.timing) {
            document.getElementById('loop-freq').textContent = data.timing.loop_freq_hz.toFixed(1) + ' Hz';
            document.getElementById('pid-freq').textContent = data.timing.pid_freq_hz.toFixed(1) + ' Hz';
        }
    }

    // Funções de controle
    async setPIDParameters(kp, ki, kd) {
        try {
            const response = await fetch(`${this.baseUrl}/set?kp=${kp}&ki=${ki}&kd=${kd}`);
            return response.ok;
        } catch (error) {
            console.error('Erro ao definir parâmetros PID:', error);
            return false;
        }
    }

    async toggleMotor() {
        try {
            const response = await fetch(`${this.baseUrl}/toggleMotor`);
            return response.ok;
        } catch (error) {
            console.error('Erro ao alternar motor:', error);
            return false;
        }
    }

    async toggleManualMode() {
        try {
            const response = await fetch(`${this.baseUrl}/toggleManual`);
            return response.ok;
        } catch (error) {
            console.error('Erro ao alternar modo manual:', error);
            return false;
        }
    }

    async setManualPWM(value) {
        try {
            const response = await fetch(`${this.baseUrl}/setPWM?value=${value}`);
            return response.ok;
        } catch (error) {
            console.error('Erro ao definir PWM manual:', error);
            return false;
        }
    }

    async setSetpoint(value) {
        try {
            const response = await fetch(`${this.baseUrl}/setSetpoint?value=${value}`);
            return response.ok;
        } catch (error) {
            console.error('Erro ao definir setpoint:', error);
            return false;
        }
    }

    // Utilitários
    formatUptime(ms) {
        const seconds = Math.floor(ms / 1000);
        const minutes = Math.floor(seconds / 60);
        const hours = Math.floor(minutes / 60);
        const days = Math.floor(hours / 24);

        if (days > 0) return `${days}d ${hours % 24}h ${minutes % 60}m`;
        if (hours > 0) return `${hours}h ${minutes % 60}m ${seconds % 60}s`;
        if (minutes > 0) return `${minutes}m ${seconds % 60}s`;
        return `${seconds}s`;
    }

    // Monitoramento de performance
    async benchmarkAPIs() {
        console.log('Testando performance das APIs...');
        
        const tests = [
            { name: 'getAllData', fn: () => this.getAllData() },
            { name: 'systemStatus', fn: () => this.getSystemStatus() },
            { name: 'pidData', fn: () => this.getPIDData() },
            { name: 'motorData', fn: () => this.getMotorData() },
        ];

        for (const test of tests) {
            const start = performance.now();
            await test.fn();
            const time = performance.now() - start;
            console.log(`${test.name}: ${time.toFixed(1)}ms`);
        }
    }
}

// Exemplo de uso
const robot = new RobotController('http://192.168.1.100');

// Inicia atualização automática a cada segundo
robot.startAutoUpdate(1000);

// Testa performance das APIs
robot.benchmarkAPIs();

// Exemplo de controle manual
document.getElementById('start-motor')?.addEventListener('click', () => {
    robot.toggleMotor();
});

document.getElementById('toggle-manual')?.addEventListener('click', () => {
    robot.toggleManualMode();
});
