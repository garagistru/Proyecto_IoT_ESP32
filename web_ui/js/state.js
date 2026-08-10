/**
 * state.js
 * Управление состоянием приложения
 */
class AppState {
    constructor() {
        this.data = {
            status: CONFIG.STATUS.CONNECTED,
            isConnected: true,
            totalNodes: CONFIG.UI.DEFAULT_TOTAL_NODES,
            activeNodes: CONFIG.UI.DEFAULT_ACTIVE_NODES,
            dormantNodes: CONFIG.UI.DEFAULT_DORMANT_NODES,
            lastReceive: 'hace 2s',
            lastTransmit: 'hace 5s',
            updateTime: new Date().toTimeString().slice(0, 8),
            signalStrength: CONFIG.UI.DEFAULT_SIGNAL_STRENGTH,
            version: 'A.R.M.-v1.2.0',
        };
        this.listeners = [];
    }

    subscribe(listener) {
        this.listeners.push(listener);
        return () => { this.listeners = this.listeners.filter(l => l !== listener); };
    }

    notify() { this.listeners.forEach(listener => listener(this.data)); }

    update(newData) {
        Object.assign(this.data, newData);
        this.notify();
    }

    setStatus(status) {
        this.data.status = status;
        this.data.isConnected = status === CONFIG.STATUS.CONNECTED;
        this.notify();
    }

    updateClock() {
        this.data.updateTime = new Date().toTimeString().slice(0, 8);
        this.notify();
    }

    generateMockData() {
        const total = Math.floor(Math.random() * 20) + 20;
        const active = Math.floor(Math.random() * total);
        const dormant = total - active;
        return {
            totalNodes: total,
            activeNodes: active,
            dormantNodes: dormant,
            lastReceive: `hace ${Math.floor(Math.random() * 10) + 1}s`,
            lastTransmit: `hace ${Math.floor(Math.random() * 15) + 1}s`,
            signalStrength: Math.floor(Math.random() * 5) + 1,
        };
    }
}

const state = new AppState();