// Settings Page JavaScript for Quit Smoking Timer Box
class SettingsManager {
    constructor() {
        this.apiBase = '';
        this.initializeEventListeners();
        this.loadSettings();
    }

    initializeEventListeners() {
        // WiFi form submission
        document.getElementById('wifiForm').addEventListener('submit', (e) => {
            e.preventDefault();
            this.saveWiFiSettings();
        });

        // AI form submission
        document.getElementById('aiForm').addEventListener('submit', (e) => {
            e.preventDefault();
            this.saveAISettings();
        });

        // Security form submission
        document.getElementById('securityForm').addEventListener('submit', (e) => {
            e.preventDefault();
            this.saveSecuritySettings();
        });

        // AI enable/disable toggle
        document.getElementById('enableAI').addEventListener('change', (e) => {
            this.toggleAIConfig(e.target.checked);
        });

        // AI provider change
        document.getElementById('aiProvider').addEventListener('change', (e) => {
            this.updateAPIKeyVisibility(e.target.value);
        });

        // Network management
        document.getElementById('addNetwork').addEventListener('click', () => {
            this.addNetworkInput('allowedNetworks');
        });

        document.getElementById('addBlockedNetwork').addEventListener('click', () => {
            this.addNetworkInput('blockedNetworks');
        });

        // Test AI button
        document.getElementById('testAI').addEventListener('click', () => {
            this.testAIGatekeeper();
        });

        // AI Modal handlers
        document.getElementById('aiSend').addEventListener('click', () => {
            this.sendAIMessage();
        });

        document.getElementById('aiUserInput').addEventListener('keypress', (e) => {
            if (e.key === 'Enter') {
                this.sendAIMessage();
            }
        });

        document.getElementById('aiCancel').addEventListener('click', () => {
            this.closeAIModal();
        });

        document.getElementById('aiComplete').addEventListener('click', () => {
            this.completeAISession();
        });
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

    async loadSettings() {
        // Load WiFi settings
        const wifiStatus = await this.apiCall('/api/wifi/status');
        if (wifiStatus) {
            this.updateWiFiStatus(wifiStatus);
        }

        // Load AI settings
        const aiSettings = await this.apiCall('/api/ai/config');
        if (aiSettings) {
            this.loadAISettings(aiSettings);
        }

        // Load security settings
        const securitySettings = await this.apiCall('/api/security/config');
        if (securitySettings) {
            this.loadSecuritySettings(securitySettings);
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
        const requireQuestions = document.getElementById('requireQuestions').checked;
        const breathingExercise = document.getElementById('breathingExercise').checked;

        if (enableAI && provider === 'openai' && !apiKey) {
            this.showMessage('Please enter an API key for OpenAI.', 'error');
            return;
        }

        const aiConfig = {
            enabled: enableAI,
            provider: provider,
            apiKey: apiKey,
            delayMinutes: delayMinutes,
            personality: personality,
            requireQuestions: requireQuestions,
            breathingExercise: breathingExercise
        };

        const result = await this.apiCall('/api/ai/config', 'POST', aiConfig);
        if (result && result.success) {
            this.showMessage('AI settings saved successfully!', 'success');
        } else {
            this.showMessage('Failed to save AI settings.', 'error');
        }
    }

    async saveSecuritySettings() {
        const allowedNetworks = this.getNetworkList('allowedNetworks');
        const blockedNetworks = this.getNetworkList('blockedNetworks');

        const securityConfig = {
            allowedNetworks: allowedNetworks,
            blockedNetworks: blockedNetworks
        };

        const result = await this.apiCall('/api/security/config', 'POST', securityConfig);
        if (result && result.success) {
            this.showMessage('Security settings saved successfully!', 'success');
        } else {
            this.showMessage('Failed to save security settings.', 'error');
        }
    }

    toggleAIConfig(enabled) {
        const aiConfig = document.getElementById('aiConfig');
        aiConfig.style.display = enabled ? 'block' : 'none';
        
        if (enabled) {
            this.updateAPIKeyVisibility(document.getElementById('aiProvider').value);
        }
    }

    updateAPIKeyVisibility(provider) {
        const apiKeyGroup = document.getElementById('apiKeyGroup');
        apiKeyGroup.style.display = (provider === 'openai') ? 'block' : 'none';
    }

    addNetworkInput(containerId) {
        const container = document.getElementById(containerId);
        const networkItem = document.createElement('div');
        networkItem.className = 'network-item';
        networkItem.innerHTML = `
            <input type="text" placeholder="Network name" class="input-field network-input">
            <button type="button" class="btn-small remove-network">❌</button>
        `;

        // Add remove functionality
        networkItem.querySelector('.remove-network').addEventListener('click', () => {
            networkItem.remove();
        });

        container.appendChild(networkItem);
    }

    getNetworkList(containerId) {
        const container = document.getElementById(containerId);
        const inputs = container.querySelectorAll('.network-input');
        return Array.from(inputs)
            .map(input => input.value.trim())
            .filter(value => value !== '');
    }

    loadAISettings(settings) {
        document.getElementById('enableAI').checked = settings.enabled || false;
        document.getElementById('aiProvider').value = settings.provider || 'simple';
        document.getElementById('aiApiKey').value = settings.apiKey || '';
        document.getElementById('delayMinutes').value = settings.delayMinutes || 10;
        document.getElementById('aiPersonality').value = settings.personality || 'supportive';
        document.getElementById('requireQuestions').checked = settings.requireQuestions !== false;
        document.getElementById('breathingExercise').checked = settings.breathingExercise !== false;

        this.toggleAIConfig(settings.enabled || false);
    }

    loadSecuritySettings(settings) {
        // Load allowed networks
        this.loadNetworkList('allowedNetworks', settings.allowedNetworks || []);
        // Load blocked networks
        this.loadNetworkList('blockedNetworks', settings.blockedNetworks || []);
    }

    loadNetworkList(containerId, networks) {
        const container = document.getElementById(containerId);
        // Clear existing except the first placeholder
        const items = container.querySelectorAll('.network-item');
        for (let i = 1; i < items.length; i++) {
            items[i].remove();
        }

        // Set first input if networks exist
        if (networks.length > 0) {
            const firstInput = container.querySelector('.network-input');
            firstInput.value = networks[0];

            // Add additional networks
            for (let i = 1; i < networks.length; i++) {
                this.addNetworkInput(containerId);
                const newInput = container.lastElementChild.querySelector('.network-input');
                newInput.value = networks[i];
            }
        }
    }

    updateWiFiStatus(status) {
        const indicator = document.getElementById('wifiIndicator');
        const statusText = document.getElementById('wifiStatusText');

        if (status.connected) {
            indicator.textContent = '🟢';
            statusText.textContent = `Connected to ${status.ssid}`;
        } else {
            indicator.textContent = '🔴';
            statusText.textContent = 'Not Connected';
        }
    }

    async checkWiFiStatus() {
        const status = await this.apiCall('/api/wifi/status');
        if (status) {
            this.updateWiFiStatus(status);
        }
    }

    // AI Gatekeeper Test Functions
    testAIGatekeeper() {
        this.openAIModal();
        this.startAISession(true); // Test mode
    }

    openAIModal() {
        document.getElementById('aiModal').style.display = 'flex';
        this.aiSessionStartTime = Date.now();
        this.aiSessionActive = true;
        this.startAITimer();
    }

    closeAIModal() {
        document.getElementById('aiModal').style.display = 'none';
        this.aiSessionActive = false;
        this.clearAIConversation();
    }

    startAITimer() {
        const timerElement = document.getElementById('sessionTime');
        const delayMinutes = parseInt(document.getElementById('delayMinutes').value) || 10;
        
        this.aiTimerInterval = setInterval(() => {
            if (!this.aiSessionActive) {
                clearInterval(this.aiTimerInterval);
                return;
            }

            const elapsed = Date.now() - this.aiSessionStartTime;
            const remaining = (delayMinutes * 60 * 1000) - elapsed;

            if (remaining <= 0) {
                timerElement.textContent = '0:00';
                document.getElementById('aiComplete').disabled = false;
                clearInterval(this.aiTimerInterval);
            } else {
                const minutes = Math.floor(remaining / 60000);
                const seconds = Math.floor((remaining % 60000) / 1000);
                timerElement.textContent = `${minutes}:${seconds.toString().padStart(2, '0')}`;
            }
        }, 1000);
    }

    async startAISession(testMode = false) {
        const conversation = document.getElementById('aiConversation');
        const personality = document.getElementById('aiPersonality').value;
        
        const welcomeMessage = this.getWelcomeMessage(personality);
        this.addAIMessage(welcomeMessage);

        // Start with initial questions
        setTimeout(() => {
            this.addAIMessage(this.getInitialQuestion(personality));
        }, 2000);
    }

    getWelcomeMessage(personality) {
        const messages = {
            supportive: "Hi there. I noticed you're trying to access an emergency unlock. I'm here to support you through this moment. Let's talk about what's happening.",
            strict: "Hold on. Before we proceed with any emergency unlock, we need to have a serious conversation about what led you here.",
            friend: "Hey, I can see you're having a tough moment. I'm here to listen and help you through this. What's going on?",
            professional: "I understand you're experiencing a craving. This is a normal part of the recovery process. Let's work through this together."
        };
        return messages[personality] || messages.supportive;
    }

    getInitialQuestion(personality) {
        const questions = {
            supportive: "Can you tell me what triggered this craving? Sometimes talking about it helps reduce its power.",
            strict: "On a scale of 1-10, how strong is this craving right now? And what specifically happened to trigger it?",
            friend: "What's been going on today that brought you to this point? I'm here to listen without judgment.",
            professional: "Let's identify the trigger. What situation, emotion, or thought led to this moment?"
        };
        return questions[personality] || questions.supportive;
    }

    sendAIMessage() {
        const input = document.getElementById('aiUserInput');
        const message = input.value.trim();
        
        if (!message) return;

        this.addUserMessage(message);
        input.value = '';

        // Simulate AI response
        setTimeout(() => {
            this.generateAIResponse(message);
        }, 1000 + Math.random() * 2000);
    }

    addAIMessage(message) {
        const conversation = document.getElementById('aiConversation');
        const messageDiv = document.createElement('div');
        messageDiv.className = 'ai-message';
        messageDiv.innerHTML = `<div class="message-content">🤖 ${message}</div>`;
        conversation.appendChild(messageDiv);
        conversation.scrollTop = conversation.scrollHeight;
    }

    addUserMessage(message) {
        const conversation = document.getElementById('aiConversation');
        const messageDiv = document.createElement('div');
        messageDiv.className = 'user-message';
        messageDiv.innerHTML = `<div class="message-content">👤 ${message}</div>`;
        conversation.appendChild(messageDiv);
        conversation.scrollTop = conversation.scrollHeight;
    }

    generateAIResponse(userMessage) {
        const personality = document.getElementById('aiPersonality').value;
        const responses = this.getAIResponses(personality, userMessage.toLowerCase());
        
        const response = responses[Math.floor(Math.random() * responses.length)];
        this.addAIMessage(response);
    }

    getAIResponses(personality, userMessage) {
        // Simple rule-based responses
        if (userMessage.includes('stress') || userMessage.includes('anxiety')) {
            return [
                "I hear that you're feeling stressed. Let's try a quick breathing exercise. Can you take 5 deep breaths with me?",
                "Stress is a common trigger. What usually helps you manage stress in healthy ways?"
            ];
        }
        
        if (userMessage.includes('work') || userMessage.includes('job')) {
            return [
                "Work stress can be overwhelming. Have you considered taking a short walk or doing a quick meditation instead?",
                "What would happen if you delayed this for just 10 more minutes? Would the work situation change?"
            ];
        }

        // Default responses
        const defaults = {
            supportive: [
                "Thank you for sharing that with me. You've made it this far in your quit journey - that's already incredible strength.",
                "I can hear this is difficult. Remember, cravings are temporary, but your progress is real and lasting."
            ],
            strict: [
                "Think about why you started this journey. Are you really ready to throw away all your progress for a temporary feeling?",
                "This is exactly the kind of moment that separates success from failure. What choice are you going to make?"
            ],
            friend: [
                "I get it, this is tough. But you know what? You've handled tough moments before. What helped you then?",
                "What would you tell me if I was in your situation right now? Sometimes we give better advice than we take."
            ],
            professional: [
                "This reaction is part of the recovery process. Your brain is trying to return to old patterns. How can we redirect this energy?",
                "Let's explore this feeling without acting on it immediately. What physical sensations are you experiencing right now?"
            ]
        };

        return defaults[personality] || defaults.supportive;
    }

    completeAISession() {
        this.closeAIModal();
        this.showMessage('AI session completed. Remember: you are stronger than your cravings!', 'success');
    }

    clearAIConversation() {
        document.getElementById('aiConversation').innerHTML = '';
        document.getElementById('aiComplete').disabled = true;
        document.getElementById('sessionTime').textContent = '10:00';
    }

    showMessage(message, type = 'info') {
        // Simple message display - you could enhance this with a proper toast system
        const colors = {
            success: '#22C55E',
            error: '#EF4444',
            info: '#3B82F6'
        };

        const messageDiv = document.createElement('div');
        messageDiv.style.cssText = `
            position: fixed;
            top: 20px;
            right: 20px;
            background: ${colors[type] || colors.info};
            color: white;
            padding: 12px 20px;
            border-radius: 8px;
            z-index: 1000;
            font-weight: 500;
        `;
        messageDiv.textContent = message;
        
        document.body.appendChild(messageDiv);
        
        setTimeout(() => {
            messageDiv.remove();
        }, 4000);
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
