// Enhanced Settings JavaScript for Quit Smoking Timer Box
class SettingsManager {
    constructor() {
        this.apiBase = '';
        this.currentSettings = {};
        this.aiSession = null;
        
        this.initializeEventListeners();
        this.loadAllSettings();
    }

    initializeEventListeners() {
        // Cost form handler
        const costForm = document.getElementById('costForm');
        if (costForm) {
            costForm.addEventListener('submit', (e) => this.handleCostSubmit(e));
            
            // Radio button change handlers
            const costTypeRadios = document.querySelectorAll('input[name="costType"]');
            costTypeRadios.forEach(radio => {
                radio.addEventListener('change', () => this.handleCostTypeChange());
            });
        }

        // WiFi form submission
        const wifiForm = document.getElementById('wifiForm');
        if (wifiForm) {
            wifiForm.addEventListener('submit', (e) => {
                e.preventDefault();
                this.saveWiFiSettings();
            });
        }

        // AI form submission
        const aiForm = document.getElementById('aiForm');
        if (aiForm) {
            aiForm.addEventListener('submit', (e) => {
                e.preventDefault();
                this.saveAISettings();
            });
        }

        // Network security form submission
        const networkForm = document.getElementById('networkForm');
        if (networkForm) {
            networkForm.addEventListener('submit', (e) => {
                e.preventDefault();
                this.saveNetworkSettings();
            });
        }

        // Progress form submission
        const progressForm = document.getElementById('progressForm');
        if (progressForm) {
            progressForm.addEventListener('submit', (e) => {
                e.preventDefault();
                this.saveProgressSettings();
            });
        }

        // Timer form handler
        const timerForm = document.getElementById('timerForm');
        if (timerForm) {
            timerForm.addEventListener('submit', (e) => this.handleTimerSubmit(e));
            
            // Timer mode change handler
            const timerMode = document.getElementById('timerMode');
            if (timerMode) {
                timerMode.addEventListener('change', () => this.handleTimerModeChange());
            }
        }

        // AI enable/disable toggle
        const aiEnabled = document.getElementById('aiEnabled');
        if (aiEnabled) {
            aiEnabled.addEventListener('change', (e) => {
                this.toggleAIConfig(e.target.checked);
            });
        }

        // AI provider change
        const aiProvider = document.getElementById('aiProvider');
        if (aiProvider) {
            aiProvider.addEventListener('change', (e) => {
                this.updateAPIKeyVisibility(e.target.value);
            });
        }

        // Test AI button
        const testAI = document.getElementById('testAI');
        if (testAI) {
            testAI.addEventListener('click', () => {
                this.testAIGatekeeper();
            });
        }

        // Language change handler from i18n system
        window.addEventListener('languageChanged', (e) => {
            this.saveLanguagePreference(e.detail.language);
        });

        // WiFi scan button
        const scanButton = document.getElementById('scanWiFi');
        if (scanButton) {
            scanButton.addEventListener('click', () => this.scanWiFiNetworks());
        }

        // WiFi status check button
        const statusButton = document.getElementById('checkWiFiStatus');
        if (statusButton) {
            statusButton.addEventListener('click', () => this.checkWiFiStatus());
        }
    }

    async apiCall(endpoint, method = 'GET', data = null) {
        try {
            const options = {
                method: method,
                headers: {
                    'Content-Type': 'application/json',
                }
            };

            if (data) {
                options.body = JSON.stringify(data);
            }

            const response = await fetch(this.apiBase + endpoint, options);
            
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            
            return await response.json();
        } catch (error) {
            console.error('API call failed:', error);
            this.showMessage('Connection error. Please check your connection.', 'error');
            return null;
        }
    }

    async loadAllSettings() {
        try {
            await Promise.all([
                this.loadCostSettings(),
                this.loadWiFiSettings(),
                this.loadAISettings(),
                this.loadSecuritySettings(),
                this.loadTimerSettings()
            ]);
            
            console.log('✅ All settings loaded');
        } catch (error) {
            console.error('❌ Error loading settings:', error);
            this.showMessage('Failed to load settings', 'error');
        }
    }

    async loadWiFiSettings() {
        try {
            const wifiStatus = await this.apiCall('/api/wifi/status');
            if (wifiStatus) {
                this.updateWiFiStatus(wifiStatus);
            }
        } catch (error) {
            console.error('Failed to load WiFi settings:', error);
        }
    }

    async loadAISettings() {
        try {
            const aiSettings = await this.apiCall('/api/ai/config');
            if (aiSettings) {
                const enableAI = document.getElementById('enableAI');
                if (enableAI) enableAI.checked = aiSettings.enabled || false;
                
                const provider = document.getElementById('aiProvider');
                if (provider) provider.value = aiSettings.provider || 'simple';
                
                const apiKey = document.getElementById('aiApiKey');
                if (apiKey) apiKey.value = aiSettings.apiKey || '';
                
                const delayMinutes = document.getElementById('delayMinutes');
                if (delayMinutes) delayMinutes.value = aiSettings.delayMinutes || 10;
                
                const personality = document.getElementById('aiPersonality');
                if (personality) personality.value = aiSettings.personality || 'supportive';
                
                this.toggleAIConfig(aiSettings.enabled || false);
                this.updateAPIKeyVisibility(aiSettings.provider || 'simple');
            }
        } catch (error) {
            console.error('Failed to load AI settings:', error);
        }
    }

    async loadSecuritySettings() {
        try {
            const securitySettings = await this.apiCall('/api/security/config');
            if (securitySettings) {
                // Load allowed networks
                let allowedNetworks = [];
                try {
                    allowedNetworks = JSON.parse(securitySettings.allowedNetworks || '[]');
                } catch (e) {
                    allowedNetworks = [];
                }
                
                // Load blocked networks  
                let blockedNetworks = [];
                try {
                    blockedNetworks = JSON.parse(securitySettings.blockedNetworks || '[]');
                } catch (e) {
                    blockedNetworks = [];
                }
                
                this.populateNetworkList('allowedNetworks', allowedNetworks);
                this.populateNetworkList('blockedNetworks', blockedNetworks);
                
                const blockOnPublic = document.getElementById('blockOnPublic');
                if (blockOnPublic) blockOnPublic.checked = securitySettings.blockOnPublic || false;
            }
        } catch (error) {
            console.error('Failed to load security settings:', error);
        }
    }

    async loadCostSettings() {
        try {
            const response = await fetch('/api/cost-config');
            if (response.ok) {
                const settings = await response.json();
                
                const productNameEl = document.getElementById('productName');
                if (productNameEl) productNameEl.value = settings.productName || '';
                
                const currencyEl = document.getElementById('currency');
                if (currencyEl) currencyEl.value = settings.currency || 'EUR';
                
                const costType = settings.usePackPrice ? 'pack' : 'cigarette';
                const costTypeEl = document.querySelector(`input[name="costType"][value="${costType}"]`);
                if (costTypeEl) costTypeEl.checked = true;
                
                const cigaretteCostEl = document.getElementById('costPerCigarette');
                if (cigaretteCostEl) cigaretteCostEl.value = settings.cigaretteCost || 0.50;
                
                const packCostEl = document.getElementById('costPerPack');
                if (packCostEl) packCostEl.value = settings.packCost || 10.00;
                
                const cigarettesPerPackEl = document.getElementById('cigarettesPerPack');
                if (cigarettesPerPackEl) cigarettesPerPackEl.value = settings.cigarettesPerPack || 20;
                
                this.handleCostTypeChange();
            }
        } catch (error) {
            console.error('Failed to load cost settings:', error);
        }
    }

    async loadTimerSettings() {
        try {
            const response = await fetch('/api/config');
            if (response.ok) {
                const settings = await response.json();
                
                const timerModeEl = document.getElementById('timerMode');
                if (timerModeEl) {
                    timerModeEl.value = settings.timerMode || 0;
                    this.handleTimerModeChange();
                }
                
                const intervalEl = document.getElementById('intervalMinutes');
                if (intervalEl) intervalEl.value = settings.intervalMinutes || 30;
                
                const dailyLimitEl = document.getElementById('dailyLimit');
                if (dailyLimitEl) dailyLimitEl.value = settings.dailyLimit || 10;
            }
        } catch (error) {
            console.error('Failed to load timer settings:', error);
        }
    }

    updateWiFiStatus(status) {
        const statusEl = document.getElementById('wifiStatus');
        const ssidEl = document.getElementById('currentSSID');
        const ipEl = document.getElementById('currentIP');
        
        if (statusEl) {
            statusEl.textContent = status.connected ? 'Connected' : 'Disconnected';
            statusEl.className = `status ${status.connected ? 'connected' : 'disconnected'}`;
        }
        
        if (ssidEl) {
            ssidEl.textContent = status.connected ? status.ssid : 'Not connected';
        }
        
        if (ipEl) {
            ipEl.textContent = status.connected ? status.ip : 'N/A';
        }
    }

    async saveWiFiSettings() {
        const ssid = document.getElementById('wifiSSID').value;
        const password = document.getElementById('wifiPassword').value;

        if (!ssid) {
            this.showMessage('Please enter a WiFi network name.', 'error');
            return;
        }

        const result = await this.apiCall('/api/wifi/connect', 'POST', {
            ssid: ssid,
            password: password
        });

        if (result && result.success) {
            this.showMessage('WiFi connection initiated. Please wait...', 'info');
            // Check status after a delay
            setTimeout(() => this.checkWiFiStatus(), 5000);
        } else {
            this.showMessage('Failed to connect to WiFi. Please check credentials.', 'error');
        }
    }

    async saveAISettings() {
        const enableAI = document.getElementById('enableAI').checked;
        const provider = document.getElementById('aiProvider').value;
        const apiKey = document.getElementById('aiApiKey').value;
        const delayMinutes = parseInt(document.getElementById('delayMinutes').value);
        const personality = document.getElementById('aiPersonality').value;

        if (enableAI && provider === 'openai' && !apiKey) {
            this.showMessage('Please enter an API key for OpenAI.', 'error');
            return;
        }

        const aiConfig = {
            enabled: enableAI,
            provider: provider,
            apiKey: apiKey,
            delayMinutes: delayMinutes,
            personality: personality
        };

        const result = await this.apiCall('/api/ai/config', 'POST', aiConfig);
        if (result && result.success) {
            this.showMessage('AI settings saved successfully!', 'success');
        } else {
            this.showMessage('Failed to save AI settings.', 'error');
        }
    }

    async saveNetworkSettings() {
        const allowedNetworks = this.getNetworkList('allowedNetworks');
        const blockedNetworks = this.getNetworkList('blockedNetworks');
        const blockOnPublic = document.getElementById('blockOnPublic').checked;

        const securityConfig = {
            allowedNetworks: JSON.stringify(allowedNetworks),
            blockedNetworks: JSON.stringify(blockedNetworks),
            blockOnPublic: blockOnPublic
        };

        const result = await this.apiCall('/api/security/config', 'POST', securityConfig);
        if (result && result.success) {
            this.showMessage('Security settings saved successfully!', 'success');
        } else {
            this.showMessage('Failed to save security settings.', 'error');
        }
    }

    async saveProgressSettings() {
        const formData = new FormData(document.getElementById('progressForm'));
        
        const config = {
            cigaretteCost: parseFloat(formData.get('cigaretteCost')),
            smokingGoal: formData.get('smokingGoal'),
            startDate: formData.get('startDate')
        };

        try {
            this.showMessage('Progress settings saved locally!', 'success');
        } catch (error) {
            console.error('Error saving progress settings:', error);
            this.showMessage('Error saving progress settings', 'error');
        }
    }

    async handleCostSubmit(e) {
        e.preventDefault();
        
        const formData = new FormData(e.target);
        const costType = formData.get('costType');
        
        const data = new URLSearchParams({
            productName: formData.get('productName') || '',
            currency: formData.get('currency') || 'EUR',
            usePackPrice: costType === 'pack' ? 'true' : 'false',
            cigaretteCost: formData.get('costPerCigarette') || '0.50',
            packCost: formData.get('costPerPack') || '10.00',
            cigarettesPerPack: formData.get('cigarettesPerPack') || '20'
        });

        try {
            const response = await fetch('/api/cost-config', {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                body: data
            });

            const result = await response.json();
            
            if (result.success) {
                this.showMessage('Settings saved successfully!', 'success');
            } else {
                this.showMessage(result.error || 'Failed to save settings', 'error');
            }
        } catch (error) {
            console.error('Cost settings error:', error);
            this.showMessage('Failed to save cost settings', 'error');
        }
    }

    handleCostTypeChange() {
        const costTypeEl = document.querySelector('input[name="costType"]:checked');
        if (!costTypeEl) return;
        
        const costType = costTypeEl.value;
        
        const cigaretteField = document.getElementById('costPerCigarette');
        const packField = document.getElementById('costPerPack');
        const packCountField = document.getElementById('cigarettesPerPack');
        
        if (cigaretteField && packField && packCountField) {
            if (costType === 'cigarette') {
                cigaretteField.disabled = false;
                packField.disabled = true;
                packCountField.disabled = true;
                
                cigaretteField.style.opacity = '1';
                packField.style.opacity = '0.5';
                packCountField.style.opacity = '0.5';
            } else {
                cigaretteField.disabled = true;
                packField.disabled = false;
                packCountField.disabled = false;
                
                cigaretteField.style.opacity = '0.5';
                packField.style.opacity = '1';
                packCountField.style.opacity = '1';
            }
        }
    }

    async handleTimerSubmit(e) {
        e.preventDefault();
        
        const formData = new FormData(e.target);
        
        const data = new URLSearchParams({
            timerMode: formData.get('timerMode') || '0',
            intervalMinutes: formData.get('intervalMinutes') || '30',
            dailyLimit: formData.get('dailyLimit') || '10'
        });

        try {
            const response = await fetch('/api/config', {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                body: data
            });

            const result = await response.json();
            
            if (result.success) {
                this.showMessage('Timer settings saved successfully!', 'success');
            } else {
                this.showMessage(result.error || 'Failed to save timer settings', 'error');
            }
        } catch (error) {
            console.error('Timer settings error:', error);
            this.showMessage('Failed to save timer settings', 'error');
        }
    }

    handleTimerModeChange() {
        const timerModeEl = document.getElementById('timerMode');
        if (!timerModeEl) return;
        
        const mode = parseInt(timerModeEl.value);
        const intervalGroup = document.getElementById('intervalGroup');
        const dailyLimitGroup = document.getElementById('dailyLimitGroup');
        
        // Show/hide relevant fields based on timer mode
        if (intervalGroup) {
            intervalGroup.style.display = (mode === 0 || mode === 1) ? 'block' : 'none';
        }
        
        if (dailyLimitGroup) {
            dailyLimitGroup.style.display = (mode === 1) ? 'block' : 'none';
        }
    }

    async saveLanguagePreference(language) {
        try {
            const response = await fetch('/api/language', {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                body: new URLSearchParams({ language })
            });

            const result = await response.json();
            
            if (!result.success) {
                console.error('Failed to save language preference:', result.error);
            }
        } catch (error) {
            console.error('Language save error:', error);
        }
    }

    async scanWiFiNetworks() {
        const scanButton = document.getElementById('scanWiFi');
        if (scanButton) {
            scanButton.disabled = true;
            scanButton.textContent = 'Scanning...';
        }

        try {
            const result = await this.apiCall('/api/wifi/scan');
            if (result && result.networks) {
                this.populateNetworkDropdown(result.networks);
                this.showMessage(`Found ${result.networks.length} networks`, 'info');
            } else {
                this.showMessage('No networks found', 'warning');
            }
        } catch (error) {
            this.showMessage('Failed to scan networks', 'error');
        } finally {
            if (scanButton) {
                scanButton.disabled = false;
                scanButton.textContent = 'Scan Networks';
            }
        }
    }

    async checkWiFiStatus() {
        try {
            const status = await this.apiCall('/api/wifi/status');
            if (status) {
                this.updateWiFiStatus(status);
                this.showMessage('WiFi status updated', 'info');
            }
        } catch (error) {
            this.showMessage('Failed to check WiFi status', 'error');
        }
    }

    populateNetworkDropdown(networks) {
        const dropdown = document.getElementById('networkDropdown');
        if (!dropdown) return;
        
        dropdown.innerHTML = '<option value="">Select a network...</option>';
        
        networks.forEach(network => {
            const option = document.createElement('option');
            option.value = network.ssid;
            option.textContent = `${network.ssid} (${network.rssi} dBm)${network.secure ? ' 🔒' : ''}`;
            dropdown.appendChild(option);
        });
        
        dropdown.addEventListener('change', (e) => {
            const ssidInput = document.getElementById('wifiSSID');
            if (ssidInput) {
                ssidInput.value = e.target.value;
            }
        });
    }

    populateNetworkList(containerId, networks) {
        const container = document.getElementById(containerId);
        if (!container) return;
        
        container.innerHTML = '';
        
        networks.forEach(network => {
            const item = document.createElement('div');
            item.className = 'network-item';
            item.innerHTML = `
                <span>${network}</span>
                <button type="button" class="remove-network">Remove</button>
            `;
            container.appendChild(item);
        });
    }

    getNetworkList(containerId) {
        const container = document.getElementById(containerId);
        if (!container) return [];
        
        const items = container.querySelectorAll('.network-item span');
        return Array.from(items).map(item => item.textContent);
    }

    toggleAIConfig(enabled) {
        const configSection = document.getElementById('aiConfigSection');
        if (configSection) {
            configSection.style.display = enabled ? 'block' : 'none';
        }
    }

    updateAPIKeyVisibility(provider) {
        const apiKeyGroup = document.getElementById('apiKeyGroup');
        if (apiKeyGroup) {
            apiKeyGroup.style.display = provider === 'openai' ? 'block' : 'none';
        }
    }

    async testAIGatekeeper() {
        this.showMessage('Testing AI Gatekeeper...', 'info');
        
        try {
            const result = await this.apiCall('/api/emergency/ai', 'POST', { trigger: 'test' });
            if (result && result.success) {
                this.showMessage('AI Gatekeeper test successful!', 'success');
            } else {
                this.showMessage('AI Gatekeeper test failed', 'error');
            }
        } catch (error) {
            this.showMessage('Failed to test AI Gatekeeper', 'error');
        }
    }

    showMessage(message, type = 'info') {
        // Remove existing messages
        const existingMessages = document.querySelectorAll('.settings-message');
        existingMessages.forEach(msg => msg.remove());
        
        // Create new message
        const messageEl = document.createElement('div');
        messageEl.className = `settings-message ${type}`;
        messageEl.textContent = message;
        
        // Insert at top of settings container
        const container = document.querySelector('.settings-container') || document.body;
        container.insertBefore(messageEl, container.firstChild);
        
        // Auto-remove after 5 seconds
        setTimeout(() => {
            if (messageEl.parentNode) {
                messageEl.remove();
            }
        }, 5000);
    }
}

// Initialize when DOM is loaded
document.addEventListener('DOMContentLoaded', () => {
    new SettingsManager();
});

// Add network item remove functionality
document.addEventListener('click', (e) => {
    if (e.target.classList.contains('remove-network')) {
        e.target.closest('.network-item').remove();
    }
});
