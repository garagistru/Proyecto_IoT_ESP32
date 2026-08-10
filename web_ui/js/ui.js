@'
/**
 * ui.js
 * Обновление DOM-элементов на основе данных
 */
class UI {
    constructor() {
        this.elements = {
            statusRing: document.getElementById('statusRing'),
            spiderIcon: document.getElementById('spiderIcon'),
            statusDot: document.getElementById('statusDot'),
            connectionStatus: document.getElementById('connectionStatus'),
            totalNodes: document.getElementById('totalNodes'),
            activeNodes: document.getElementById('activeNodes'),
            dormantNodes: document.getElementById('dormantNodes'),
            lastReceive: document.getElementById('lastReceive'),
            lastTransmit: document.getElementById('lastTransmit'),
            updateTime: document.getElementById('updateTime'),
            signalStrength: document.getElementById('signalStrength'),
            version: document.getElementById('version'),
        };
    }

    updateStatus(status, isConnected) {
        const isDisconnected = status === CONFIG.STATUS.DISCONNECTED;
        this.elements.statusRing.classList.toggle('disconnected', isDisconnected);
        this.elements.spiderIcon.classList.toggle('disconnected', isDisconnected);
        this.elements.spiderIcon.style.filter = isDisconnected
            ? 'drop-shadow(0 0 12px rgba(255,51,85,0.4))'
            : 'drop-shadow(0 0 12px rgba(0,255,136,0.4))';
        this.elements.statusDot.classList.toggle('disconnected', isDisconnected);
        this.elements.connectionStatus.classList.toggle('disconnected', isDisconnected);
        this.elements.connectionStatus.textContent = isDisconnected
            ? '🔴 Desconectado'
            : '🟢 Conectado';
    }

    updateNodes(total, active, dormant) {
        this.elements.totalNodes.textContent = total;
        this.elements.activeNodes.textContent = active;
        this.elements.dormantNodes.textContent = dormant;
    }

    updateTransfer(receive, transmit) {
        this.elements.lastReceive.textContent = receive;
        this.elements.lastTransmit.textContent = transmit;
    }

    updateFooter(time, signalStrength) {
        this.elements.updateTime.textContent = time;
        const bars = '█'.repeat(signalStrength) + '░'.repeat(5 - signalStrength);
        this.elements.signalStrength.textContent = bars;
        const colors = ['#ff3355', '#ff8833', '#ffd700', '#88ff88', '#00ff88'];
        this.elements.signalStrength.style.color = colors[signalStrength - 1] || '#00ff88';
    }

    updateVersion(version) {
        this.elements.version.textContent = version;
    }

    render(data) {
        this.updateStatus(data.status, data.isConnected);
        this.updateNodes(data.totalNodes, data.activeNodes, data.dormantNodes);
        this.updateTransfer(data.lastReceive, data.lastTransmit);
        this.updateFooter(data.updateTime, data.signalStrength);
        this.updateVersion(data.version);
    }
}

const ui = new UI();
'@ | Out-File -FilePath "web_ui\js\ui.js" -Encoding UTF8