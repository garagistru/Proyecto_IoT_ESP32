@'
/**
 * api.js
 * Взаимодействие с бэкендом (ESP32)
 */
class API {
    constructor(config) {
        this.baseUrl = config.API.BASE_URL;
        this.endpoints = config.API.ENDPOINTS;
        this.timeout = 5000;
    }

    async request(endpoint, options = {}) {
        const url = `${this.baseUrl}${endpoint}`;
        const defaultOptions = {
            method: 'GET',
            headers: { 'Content-Type': 'application/json' },
            signal: AbortSignal.timeout(this.timeout),
        };
        try {
            const response = await fetch(url, { ...defaultOptions, ...options });
            if (!response.ok) {
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
            }
            return await response.json();
        } catch (error) {
            console.error(`[API Error] ${endpoint}:`, error);
            throw error;
        }
    }

    async getStatus() {
        return this.request(this.endpoints.STATUS);
    }

    async getNodes() {
        return this.request(this.endpoints.NODES);
    }

    async getData() {
        return this.request(this.endpoints.DATA);
    }

    async getSystem() {
        return this.request(this.endpoints.SYSTEM);
    }

    async ping() {
        try {
            const response = await fetch(this.baseUrl, {
                method: 'HEAD',
                signal: AbortSignal.timeout(2000),
            });
            return response.ok;
        } catch {
            return false;
        }
    }
}

const api = new API(CONFIG);
'@ | Out-File -FilePath "web_ui\js\api.js" -Encoding UTF8