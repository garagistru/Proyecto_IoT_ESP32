// ===== 1. Конфигурация (ЗАЧЕМ: выносим настройки вверх, чтобы легко менять) =====
const CONFIG = {
    defaultTotal: 29,
    updateIntervalMs: 1000, // Обновление времени каждую секунду
};

// ===== 2. Состояние приложения (State) =====
let state = {
    totalNodes: CONFIG.defaultTotal,
    activeNodes: 24,
    sleepingNodes: 5,
    isConnected: true,
    lastUpdate: new Date().toTimeString().slice(0, 8),
};

// ===== 3. Кэширование DOM-элементов (ЗАЧЕМ: не искать их в дереве каждый раз, это быстрее) =====
const elements = {
    screen: document.getElementById('screen'),
    spiderIcon: document.getElementById('spiderIcon'),
    statusText: document.getElementById('statusText'),
    totalNodes: document.getElementById('totalNodes'),
    activeNodes: document.getElementById('activeNodes'),
    sleepingNodes: document.getElementById('sleepingNodes'),
    updateTime: document.getElementById('updateTime'),
    debugTotal: document.getElementById('debugTotal'),
    debugActive: document.getElementById('debugActive'),
    debugSleeping: document.getElementById('debugSleeping'),
    debugStatus: document.getElementById('debugStatus'),
    btnToggle: document.getElementById('btnToggle'),
    btnRandomize: document.getElementById('btnRandomize'),
};

// ===== 4. Функции обновления UI =====
function render() {
    // Обновляем тексты
    elements.totalNodes.textContent = state.totalNodes;
    elements.activeNodes.textContent = state.activeNodes;
    elements.sleepingNodes.textContent = state.sleepingNodes;
    elements.updateTime.textContent = state.lastUpdate;

    // Обновляем отладочную панель
    elements.debugTotal.textContent = state.totalNodes;
    elements.debugActive.textContent = state.activeNodes;
    elements.debugSleeping.textContent = state.sleepingNodes;

    // Обновляем статус подключения
    if (state.isConnected) {
        elements.screen.classList.remove('disconnected');
        elements.statusText.textContent = 'Conectado';
        elements.debugStatus.textContent = '✅ Conectado';
        elements.debugStatus.className = 'text-green';
    } else {
        elements.screen.classList.add('disconnected');
        elements.statusText.textContent = 'Desconectado';
        elements.debugStatus.textContent = '❌ Desconectado';
        elements.debugStatus.className = 'text-red'; // (можно добавить .text-red в CSS)
    }
}

// ===== 5. Обработчики событий (ЗАЧЕМ: отделяем логику от разметки) =====
elements.btnToggle.addEventListener('click', () => {
    state.isConnected = !state.isConnected;
    render();
});

elements.btnRandomize.addEventListener('click', () => {
    const total = Math.floor(Math.random() * 20) + 10;
    const active = Math.floor(Math.random() * total);
    
    state.totalNodes = total;
    state.activeNodes = active;
    state.sleepingNodes = total - active;
    state.lastUpdate = new Date().toTimeString().slice(0, 8);
    
    render();
});

// ===== 6. Инициализация =====
// Обновляем время каждую секунду, даже если данные не меняются
setInterval(() => {
    state.lastUpdate = new Date().toTimeString().slice(0, 8);
    elements.updateTime.textContent = state.lastUpdate;
}, CONFIG.updateIntervalMs);

// Первичная отрисовка
render();