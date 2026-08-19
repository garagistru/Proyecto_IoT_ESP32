// data/js/app.js - Только статусная информация

async function updateDashboard() {
    try {
        // --- 1. Получаем данные о узлах ---
        const nodesRes = await fetch(CONFIG.API.BASE_URL + CONFIG.API.ENDPOINTS.NODES);
        const nodes = await nodesRes.json();
        
        document.getElementById('totalNodes').textContent = nodes.total || 0;
        document.getElementById('activeNodes').textContent = nodes.active || 0;
        document.getElementById('dormantNodes').textContent = nodes.dormant || 0;
        
        // --- 2. Получаем данные о передаче ---
        const dataRes = await fetch(CONFIG.API.BASE_URL + CONFIG.API.ENDPOINTS.DATA);
        const data = await dataRes.json();
        
        document.getElementById('lastReceive').textContent = data.receive || '---';
        document.getElementById('lastTransmit').textContent = data.transmit || '---';
        
        const bufferEl = document.getElementById('bufferSize');
        if (bufferEl) {
            bufferEl.textContent = data.buffer || 0;
        }
        
        // --- 3. Статус соединения ---
        const isConnected = nodesRes.ok && dataRes.ok;
        updateConnectionStatus(isConnected);
        
        // --- 4. Обновляем время ---
        const now = new Date();
        document.getElementById('updateTime').textContent = 
            now.toLocaleTimeString('es-ES');
            
        // --- 5. Панель отладки ---
        const debugNodes = document.getElementById('debug-nodes');
        if (debugNodes) {
            debugNodes.textContent = JSON.stringify(nodes, null, 2);
        }
        
        const debugData = document.getElementById('debug-data');
        if (debugData) {
            debugData.textContent = JSON.stringify(data, null, 2);
        }
            
    } catch (error) {
        console.error('Error:', error);
        updateConnectionStatus(false);
        
        const debugNodes = document.getElementById('debug-nodes');
        if (debugNodes) {
            debugNodes.textContent = 'ОШИБКА: ' + error.message;
        }
    }
}

function updateConnectionStatus(connected) {
    const dot = document.getElementById('statusDot');
    const status = document.getElementById('connectionStatus');
    
    if (connected) {
        dot.style.backgroundColor = '#00ff88';
        dot.style.boxShadow = '0 0 10px #00ff88';
        status.textContent = 'Conectado';
        status.style.color = '#00ff88';
    } else {
        dot.style.backgroundColor = '#ff3355';
        dot.style.boxShadow = '0 0 10px #ff3355';
        status.textContent = 'Desconectado';
        status.style.color = '#ff3355';
    }
}

// --- Запуск ---
document.addEventListener('DOMContentLoaded', () => {
    console.log('🕷️ EnrollaDatos (ED) - Monitor');
    updateDashboard();
    setInterval(updateDashboard, 2000);
});