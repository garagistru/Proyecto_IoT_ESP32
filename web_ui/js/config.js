/**
 * config.js
 * Конфигурация приложения
 */
const CONFIG = {
    API: {
        BASE_URL: 'http://localhost:3000',
        ENDPOINTS: {
            STATUS: '/api/status',
            NODES: '/api/nodes',
            DATA: '/api/data',
            SYSTEM: '/api/system'
        }
    },
    TIMING: {
        UPDATE_INTERVAL: 3000,
        STATUS_CHECK_INTERVAL: 5000,
        RECONNECT_DELAY: 2000
    },
    STATUS: {
        CONNECTED: 'connected',
        DISCONNECTED: 'disconnected',
        ERROR: 'error'
    },
    UI: {
        DEFAULT_TOTAL_NODES: 24,
        DEFAULT_ACTIVE_NODES: 18,
        DEFAULT_DORMANT_NODES: 6,
        DEFAULT_SIGNAL_STRENGTH: 5
    }
};

if (typeof module !== 'undefined' && module.exports) {
    module.exports = CONFIG;
}