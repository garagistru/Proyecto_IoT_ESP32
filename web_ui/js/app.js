/**
 * app.js
 * Главный файл приложения — связывает всё вместе
 */
class App {
    constructor() {
        this.isRunning = false;
        this.updateInterval = null;
        this.statusInterval = null;

        state.subscribe((data) => { ui.render(data); });
        this.init();
    }

    async init() {
        console.log('🕷️ EnrollaDatos (ED) - Web UI v1.2.0');
        console.log('📡 Режим:', CONFIG.API.BASE_URL.includes('localhost') ? 'ДЕМО' : 'РЕАЛЬНЫЙ');
        ui.render(state.data);
        this.start();
        await this.checkConnection();
    }

    start() {
        if (this.isRunning) return;
        this.isRunning = true;
        this.updateInterval = setInterval(() => { this.updateData(); }, CONFIG.TIMING.UPDATE_INTERVAL);
        this.statusInterval = setInterval(() => { this.checkConnection(); }, CONFIG.TIMING.STATUS_CHECK_INTERVAL);
        console.log('✅ Приложение запущено');
    }

    stop() {
        if (!this.isRunning) return;
        this.isRunning = false;
        clearInterval(this.updateInterval);
        clearInterval(this.statusInterval);
        console.log('⏹️ Приложение остановлено');
    }

    async updateData() {
        try {
            const nodes = await api.getNodes();
            const data = await api.getData();
            state.update({
                totalNodes: nodes.total || state.data.totalNodes,
                activeNodes: nodes.active || state.data.activeNodes,
                dormantNodes: nodes.dormant || state.data.dormantNodes,
                lastReceive: data.receive || state.data.lastReceive,
                lastTransmit: data.transmit || state.data.lastTransmit,
            });
        } catch (error) {
            console.warn('⚠️ Не удалось получить данные, используем мок-данные');
            state.update(state.generateMockData());
            state.setStatus(CONFIG.STATUS.ERROR);
        }
    }

    async checkConnection() {
        try {
            const isConnected = await api.ping();
            state.setStatus(isConnected ? CONFIG.STATUS.CONNECTED : CONFIG.STATUS.DISCONNECTED);
        } catch {
            state.setStatus(CONFIG.STATUS.DISCONNECTED);
        }
        state.updateClock();
    }
}

const app = new App();
console.log('🛠️ Доступны объекты: state, api, ui, app');