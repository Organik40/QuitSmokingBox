// Quit Smoking Timer Box - Web Interface JavaScript
class SmokingTimerBox {
    constructor() {
        this.apiBase = '';
        this.updateInterval = null;
        this.currentState = {};
        this.websocket = null;
        this.aiSession = null;
        this.setupStatus = null; // NEW: Track setup status
        
        this.initializeWebSocket();
        this.initializeEventListeners();
        this.startPeriodicUpdates();
        this.loadConfiguration();
        this.checkSetupStatus(); // NEW: Check if setup is needed
        
        // Initialize language selector
        this.initializeLanguageSelector();
        
        // Check for PWA support
        this.initializePWA();
    }

    initializeWebSocket() {
        const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
        const wsUrl = `${protocol}//${window.location.host}/ws`;
        
        this.websocket = new WebSocket(wsUrl);
        
        this.websocket.onopen = () => {
            console.log('🌐 WebSocket connected - Real-time updates enabled');
            this.showMessage('Real-time updates enabled', 'success');
        };
        
        this.websocket.onmessage = (event) => {
            try {
                const status = JSON.parse(event.data);
                this.currentState = status;
                this.updateDisplay(status);
            } catch (error) {
                console.error('WebSocket message parsing error:', error);
            }
        };
        
        this.websocket.onclose = () => {
            console.log('🔌 WebSocket disconnected - Falling back to periodic updates');
            this.showMessage('Real-time updates disabled - using periodic refresh', 'warning');
            
            // Attempt to reconnect after 5 seconds
            setTimeout(() => {
                this.initializeWebSocket();
            }, 5000);
        };
        
        this.websocket.onerror = (error) => {
            console.error('WebSocket error:', error);
        };
    }

    initializePWA() {
        // Register service worker if available
        if ('serviceWorker' in navigator) {
            window.addEventListener('load', () => {
                navigator.serviceWorker.register('/sw.js')
                    .then((registration) => {
                        console.log('SW registered: ', registration);
                    })
                    .catch((registrationError) => {
                        console.log('SW registration failed: ', registrationError);
                    });
            });
        }
        
        // Add to home screen prompt
        let deferredPrompt;
        window.addEventListener('beforeinstallprompt', (e) => {
            e.preventDefault();
            deferredPrompt = e;
            
            // Show install button
            this.showInstallPrompt(deferredPrompt);
        });
    }

    showInstallPrompt(deferredPrompt) {
        const installPrompt = document.createElement('div');
        installPrompt.className = 'install-prompt';
        installPrompt.innerHTML = `
            <div class="install-content">
                <span>📱 Install Quit Smoking Timer Box as an app?</span>
                <button id="installBtn" class="btn btn-primary">Install</button>
                <button id="dismissBtn" class="btn btn-secondary">Maybe Later</button>
            </div>
        `;
        
        document.body.appendChild(installPrompt);
        
        document.getElementById('installBtn').addEventListener('click', () => {
            deferredPrompt.prompt();
            deferredPrompt.userChoice.then((choiceResult) => {
                if (choiceResult.outcome === 'accepted') {
                    console.log('User accepted the A2HS prompt');
                }
                deferredPrompt = null;
                installPrompt.remove();
            });
        });
        
        document.getElementById('dismissBtn').addEventListener('click', () => {
            installPrompt.remove();
        });
    }

    initializeEventListeners() {
        // Form submission
        document.getElementById('configForm').addEventListener('submit', (e) => {
            e.preventDefault();
            this.saveConfiguration();
        });

        // Action buttons
        document.getElementById('unlockBtn').addEventListener('click', () => {
            this.showConfirmModal('Unlock Box', 'Are you sure you want to unlock the box now?', () => {
                this.unlockBox();
            });
        });

        document.getElementById('emergencyBtn').addEventListener('click', () => {
            this.handleEmergencyUnlock();
        });

        document.getElementById('resetBtn').addEventListener('click', () => {
            this.showConfirmModal('Reset Progress', 
                'This will reset all your progress and statistics. This cannot be undone!', 
                () => {
                    this.resetProgress();
                }
            );
        });

        document.getElementById('testBtn').addEventListener('click', () => {
            this.testServo();
        });

        // Timer mode change
        document.getElementById('timerMode').addEventListener('change', (e) => {
            this.updateFormVisibility(parseInt(e.target.value));
        });

        // Modal handlers
        document.getElementById('modalCancel').addEventListener('click', () => {
            this.hideModal();
        });

        document.getElementById('modalConfirm').addEventListener('click', () => {
            if (this.modalCallback) {
                this.modalCallback();
            }
            this.hideModal();
        });

        // Close modal on backdrop click
        document.getElementById('modal').addEventListener('click', (e) => {
            if (e.target.id === 'modal') {
                this.hideModal();
            }
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

    async loadConfiguration() {
        const config = await this.apiCall('/api/config');
        if (config) {
            const timerMode = parseInt(config.timerMode || 0);
            
            document.getElementById('timerMode').value = timerMode;
            document.getElementById('intervalMinutes').value = config.intervalMinutes || 30;
            document.getElementById('dailyLimit').value = config.dailyLimit || 10;
            document.getElementById('emergencyUnlocks').value = config.emergencyUnlocks || 3;
            
            // Load schedule data if available
            if (config.scheduleHour !== undefined && config.scheduleMinute !== undefined) {
                const timeValue = `${config.scheduleHour.toString().padStart(2, '0')}:${config.scheduleMinute.toString().padStart(2, '0')}`;
                document.getElementById('scheduleTime').value = timeValue;
            }
            
            if (config.unlockDuration !== undefined) {
                document.getElementById('unlockDuration').value = config.unlockDuration;
            }
            
            if (config.weekDay !== undefined) {
                document.getElementById('weekDay').value = config.weekDay;
            }
            
            this.updateFormVisibility(timerMode);
        }
    }

    // NEW: Check setup status and show setup flow if needed
    async checkSetupStatus() {
        try {
            const setupStatus = await this.apiCall('/api/setup-status');
            this.setupStatus = setupStatus;
            
            if (setupStatus && !setupStatus.configured) {
                this.showSetupFlow();
            } else {
                this.hideSetupFlow();
            }
        } catch (error) {
            console.error('Error checking setup status:', error);
        }
    }

    showSetupFlow() {
        const setupCard = document.getElementById('setupCard');
        if (setupCard) {
            setupCard.style.display = 'block';
        }
        
        // Hide main configuration card if not set up
        const configCard = document.querySelector('.config-card');
        if (configCard) {
            configCard.style.display = 'none';
        }
        
        this.updateSetupChecklist();
    }

    hideSetupFlow() {
        const setupCard = document.getElementById('setupCard');
        if (setupCard) {
            setupCard.style.display = 'none';
        }
        
        // Show main configuration card when setup is complete
        const configCard = document.querySelector('.config-card');
        if (configCard) {
            configCard.style.display = 'block';
        }
    }

    updateSetupChecklist() {
        if (!this.setupStatus) return;
        
        const checks = this.setupStatus.checks;
        
        // Update checklist items
        this.updateChecklistItem('timerCheck', checks.timer, 'Timer configuration');
        this.updateChecklistItem('servoCheck', checks.servo, 'Servo calibration');
        this.updateChecklistItem('wifiCheck', checks.wifi, 'WiFi connection');
        this.updateChecklistItem('costCheck', checks.cost, 'Cost settings');
    }

    updateChecklistItem(elementId, isComplete, label) {
        const element = document.getElementById(elementId);
        if (element) {
            element.className = `checklist-item ${isComplete ? 'complete' : 'incomplete'}`;
            element.innerHTML = `
                <span class="checklist-icon">${isComplete ? '✅' : '⭕'}</span>
                <span class="checklist-label">${label}</span>
                <span class="checklist-status">${isComplete ? 'Complete' : 'Pending'}</span>
            `;
        }
    }

    async saveConfiguration() {
        const formData = new FormData(document.getElementById('configForm'));
        const timerMode = parseInt(formData.get('timerMode'));
        
        const config = {
            timerMode: timerMode,
            intervalMinutes: parseInt(formData.get('intervalMinutes')),
            dailyLimit: parseInt(formData.get('dailyLimit')),
            emergencyUnlocks: parseInt(formData.get('emergencyUnlocks'))
        };

        // Add schedule data for scheduled modes
        if (timerMode === 4 || timerMode === 5 || timerMode === 6) { // Schedule modes
            const timeValue = document.getElementById('scheduleTime').value;
            const [hour, minute] = timeValue.split(':').map(Number);
            
            config.scheduleHour = hour;
            config.scheduleMinute = minute;
            config.unlockDuration = parseInt(document.getElementById('unlockDuration').value);
            
            if (timerMode === 5) { // Weekly schedule
                config.weekDay = parseInt(document.getElementById('weekDay').value);
            }
        }

        const result = await this.apiCall('/api/config', 'POST', config);
        if (result && result.success) {
            this.showMessage('Configuration saved successfully!', 'success');
            this.updateStatus();
            this.updateSchedulePreview();
        } else {
            this.showMessage('Failed to save configuration. Please try again.', 'error');
        }
    }

    async updateStatus() {
        const status = await this.apiCall('/api/status');
        if (status) {
            this.currentState = status;
            this.updateDisplay(status);
        }
    }

    updateDisplay(status) {
        // Update real-time indicator
        const realtimeIndicator = document.getElementById('realtimeIndicator');
        if (this.websocket && this.websocket.readyState === WebSocket.OPEN) {
            realtimeIndicator.classList.add('active');
            realtimeIndicator.textContent = 'Real-time updates';
        } else {
            realtimeIndicator.classList.remove('active');
            realtimeIndicator.textContent = 'Periodic updates';
        }
        
        // Update status badge
        const statusBadge = document.getElementById('statusBadge');
        const statusText = document.getElementById('statusText');
        
        switch (status.boxState) {
            case 0: // LOCKED
                statusText.textContent = 'Locked';
                statusBadge.style.background = '#EF4444';
                break;
            case 1: // UNLOCKED
                statusText.textContent = 'Unlocked';
                statusBadge.style.background = '#22C55E';
                break;
            case 2: // COUNTDOWN
                statusText.textContent = 'Countdown Active';
                statusBadge.style.background = '#F59E0B';
                break;
            default:
                statusText.textContent = 'Unknown';
                statusBadge.style.background = '#6B7280';
        }

        // Update network status
        const networkStatus = document.getElementById('networkStatus');
        if (status.wifiConnected) {
            if (status.currentNetwork === 'AP Mode') {
                networkStatus.textContent = 'AP Mode';
                networkStatus.className = 'network-status ap-mode';
            } else {
                networkStatus.textContent = status.currentNetwork;
                networkStatus.className = 'network-status connected';
            }
        } else {
            networkStatus.textContent = 'Disconnected';
            networkStatus.className = 'network-status disconnected';
        }

        // Update status values
        document.getElementById('boxState').textContent = this.getStateText(status.boxState);
        document.getElementById('timeRemaining').textContent = this.formatTime(status.timeRemaining || 0);
        document.getElementById('emergencyCount').textContent = status.emergencyCount || 0;
        document.getElementById('maxEmergency').textContent = status.maxEmergency || 3;

        // Update enhanced statistics (new layout)
        document.getElementById('smokeFree').textContent = status.smokeFree || 0;
        document.getElementById('moneySaved').textContent = `$${(status.moneySaved || 0).toFixed(2)}`;
        document.getElementById('totalCigarettes').textContent = status.totalCigarettes || 0;
        document.getElementById('totalDays').textContent = status.totalDays || 0;

        // Update old statistics section if it exists
        const totalSavedOld = document.getElementById('totalSaved');
        const totalCigarettesOld = document.getElementById('totalCigarettesOld');
        const totalDaysOld = document.getElementById('totalDaysOld');
        const longestStreak = document.getElementById('longestStreak');
        
        if (totalSavedOld) totalSavedOld.textContent = `$${(status.moneySaved || 0).toFixed(2)}`;
        if (totalCigarettesOld) totalCigarettesOld.textContent = status.totalCigarettes || 0;
        if (totalDaysOld) totalDaysOld.textContent = status.totalDays || 0;
        if (longestStreak) longestStreak.textContent = status.longestStreak || 0;

        // Update button states
        this.updateButtonStates(status);
        
        // Update AI emergency status if applicable
        if (status.aiEnabled) {
            this.updateAIStatus(status);
        }
    }

    updateAIStatus(status) {
        // Update emergency button text to indicate AI mode
        const emergencyBtn = document.getElementById('emergencyBtn');
        if (emergencyBtn && !emergencyBtn.disabled) {
            if (status.emergencyAllowed) {
                emergencyBtn.textContent = `🤖 AI Emergency (${status.emergencyCount || 0}/${status.maxEmergency || 3})`;
            } else {
                emergencyBtn.textContent = '🚫 Emergency Blocked (Network)';
                emergencyBtn.disabled = true;
            }
        }
        
        // Show AI session status if active
        if (status.activeSession) {
            const sessionInfo = document.createElement('div');
            sessionInfo.className = 'ai-session-info';
            sessionInfo.innerHTML = `
                <div style="background: var(--ai-bg); padding: 10px; border-radius: 8px; margin: 10px 0;">
                    🤖 AI Emergency session active - ${Math.floor(status.sessionElapsed / 60)}:${(status.sessionElapsed % 60).toString().padStart(2, '0')} elapsed
                </div>
            `;
            
            // Remove existing session info
            const existing = document.querySelector('.ai-session-info');
            if (existing) existing.remove();
            
            // Add to status card
            const statusCard = document.querySelector('.status-card');
            statusCard.appendChild(sessionInfo);
        }
    }

    getStateText(state) {
        const states = {
            0: '🔒 Locked',
            1: '🔓 Unlocked',
            2: '⏳ Countdown',
            3: '🚨 Emergency',
            4: '⚙️ Setup'
        };
        return states[state] || 'Unknown';
    }

    formatTime(seconds) {
        if (seconds <= 0) return '00:00:00';
        
        const hours = Math.floor(seconds / 3600);
        const minutes = Math.floor((seconds % 3600) / 60);
        const secs = seconds % 60;
        
        return `${hours.toString().padStart(2, '0')}:${minutes.toString().padStart(2, '0')}:${secs.toString().padStart(2, '0')}`;
    }

    updateButtonStates(status) {
        const unlockBtn = document.getElementById('unlockBtn');
        const emergencyBtn = document.getElementById('emergencyBtn');
        
        // Enable/disable buttons based on state
        unlockBtn.disabled = status.boxState === 1; // Already unlocked
        emergencyBtn.disabled = status.emergencyCount >= (status.maxEmergency || 3);
        
        // Update button text
        if (status.boxState === 1) {
            unlockBtn.textContent = '✅ Already Unlocked';
        } else {
            unlockBtn.textContent = '🔓 Unlock Now';
        }
        
        if (emergencyBtn.disabled) {
            emergencyBtn.textContent = '❌ No Emergency Unlocks Left';
        } else {
            emergencyBtn.textContent = `🚨 Emergency Unlock (${status.emergencyCount || 0}/${status.maxEmergency || 3})`;
        }
    }

    updateFormVisibility(timerMode) {
        const intervalGroup = document.getElementById('intervalGroup');
        const dailyLimitGroup = document.getElementById('dailyLimitGroup');
        const scheduleConfig = document.getElementById('scheduleConfig');
        const weekDayGroup = document.getElementById('weekDayGroup');
        
        // Hide all groups initially
        intervalGroup.style.display = 'none';
        dailyLimitGroup.style.display = 'none';
        scheduleConfig.style.display = 'none';
        weekDayGroup.style.display = 'none';
        
        switch (timerMode) {
            case 0: // Fixed Interval
                intervalGroup.style.display = 'block';
                dailyLimitGroup.style.display = 'block';
                break;
            case 1: // Gradual Reduction
                intervalGroup.style.display = 'block';
                dailyLimitGroup.style.display = 'block';
                break;
            case 2: // Complete Quit
                break;
            case 3: // Emergency Only
                break;
            case 4: // Daily Schedule
                scheduleConfig.style.display = 'block';
                this.updateSchedulePreview();
                break;
            case 5: // Weekly Schedule
                scheduleConfig.style.display = 'block';
                weekDayGroup.style.display = 'block';
                this.updateSchedulePreview();
                break;
            case 6: // Custom Schedule
                scheduleConfig.style.display = 'block';
                this.updateSchedulePreview();
                break;
        }
    }

    async updateSchedulePreview() {
        const status = await this.apiCall('/api/schedule-info');
        if (status && status.nextUnlock) {
            document.getElementById('nextUnlockTime').textContent = status.nextUnlock;
        }
    }

    async unlockBox() {
        const result = await this.apiCall('/api/unlock', 'POST');
        if (result && result.success) {
            this.showMessage('Box unlocked successfully!', 'success');
            this.updateStatus();
        } else {
            this.showMessage('Failed to unlock box. Please try again.', 'error');
        }
    }

    async emergencyUnlock() {
        const result = await this.apiCall('/api/emergency', 'POST');
        if (result && result.success) {
            this.showMessage(`Emergency unlock successful! Penalty: ${result.penalty} minutes added to next timer.`, 'success');
            this.updateStatus();
        } else {
            this.showMessage('Emergency unlock failed. Daily limit may be reached.', 'error');
        }
    }

    async resetProgress() {
        const result = await this.apiCall('/api/reset', 'POST');
        if (result && result.success) {
            this.showMessage('Progress reset successfully!', 'success');
            this.updateStatus();
        } else {
            this.showMessage('Failed to reset progress. Please try again.', 'error');
        }
    }

    async testServo() {
        const result = await this.apiCall('/api/test', 'POST');
        if (result && result.success) {
            this.showMessage('Servo test completed!', 'success');
        } else {
            this.showMessage('Servo test failed. Check hardware connections.', 'error');
        }
    }

    showConfirmModal(title, message, callback) {
        document.getElementById('modalTitle').textContent = title;
        document.getElementById('modalMessage').textContent = message;
        document.getElementById('modal').style.display = 'block';
        this.modalCallback = callback;
    }

    hideModal() {
        document.getElementById('modal').style.display = 'none';
        this.modalCallback = null;
    }

    showMessage(text, type = 'info') {
        // Remove existing messages
        const existingMessages = document.querySelectorAll('.message');
        existingMessages.forEach(msg => msg.remove());
        
        // Create new message
        const message = document.createElement('div');
        message.className = `message ${type}`;
        message.textContent = text;
        
        // Insert at top of main content
        const mainContent = document.querySelector('.main-content');
        mainContent.insertBefore(message, mainContent.firstChild);
        
        // Auto-remove after 5 seconds
        setTimeout(() => {
            if (message.parentNode) {
                message.remove();
            }
        }, 5000);
    }

    startPeriodicUpdates() {
        this.updateStatus(); // Initial update
        
        this.updateInterval = setInterval(() => {
            this.updateStatus();
        }, 2000); // Update every 2 seconds
    }

    stopPeriodicUpdates() {
        if (this.updateInterval) {
            clearInterval(this.updateInterval);
            this.updateInterval = null;
        }
    }

    async handleEmergencyUnlock() {
        // Check if emergency is allowed on current network
        const status = await this.apiCall('/api/status');
        if (status && !status.emergencyAllowed) {
            this.showMessage('Emergency unlock is blocked on this network for your safety.', 'error');
            return;
        }
        
        // First check if AI gatekeeper is enabled
        const aiSettings = await this.apiCall('/api/ai/config');
        
        if (aiSettings && aiSettings.enabled) {
            this.showEmergencyTriggerSelection();
        } else {
            // Normal emergency unlock with 5-minute delay
            this.startNormalEmergencyDelay();
        }
    }

    showEmergencyTriggerSelection() {
        const triggerModal = document.createElement('div');
        triggerModal.className = 'modal';
        triggerModal.style.display = 'flex';
        triggerModal.innerHTML = `
            <div class="modal-content">
                <h3>🤖 AI Emergency Gatekeeper</h3>
                <p>What triggered this craving? This helps me provide better support.</p>
                <div style="margin: 20px 0;">
                    <button class="trigger-btn btn btn-secondary" data-trigger="stress">😰 Stress</button>
                    <button class="trigger-btn btn btn-secondary" data-trigger="boredom">😴 Boredom</button>
                    <button class="trigger-btn btn btn-secondary" data-trigger="anger">😡 Anger</button>
                    <button class="trigger-btn btn btn-secondary" data-trigger="habit">🔄 Habit</button>
                    <button class="trigger-btn btn btn-secondary" data-trigger="social">👥 Social</button>
                    <button class="trigger-btn btn btn-secondary" data-trigger="other">❓ Other</button>
                </div>
                <div style="text-align: center; margin-top: 20px;">
                    <button id="cancelAI" class="btn btn-secondary">Cancel</button>
                </div>
            </div>
        `;
        
        document.body.appendChild(triggerModal);
        
        // Add event listeners for trigger buttons
        triggerModal.querySelectorAll('.trigger-btn').forEach(btn => {
            btn.addEventListener('click', () => {
                const trigger = btn.getAttribute('data-trigger');
                triggerModal.remove();
                this.startAIEmergencySession(trigger);
            });
        });
        
        document.getElementById('cancelAI').addEventListener('click', () => {
            triggerModal.remove();
        });
    }

    async startAIEmergencySession(trigger = 'general') {
        // Start AI session
        const response = await this.apiCall('/api/emergency/ai', 'POST', { trigger: trigger });
        
        if (response && response.success) {
            this.aiSession = {
                id: response.sessionId,
                startTime: Date.now(),
                minDuration: response.minDuration * 1000, // Convert to milliseconds
                trigger: trigger,
                messageCount: 0
            };
            
            this.showAIChatInterface();
        } else {
            this.showMessage(response ? response.message : 'Failed to start AI session', 'error');
        }
    }

    showAIChatInterface() {
        const chatModal = document.createElement('div');
        chatModal.className = 'modal ai-chat-modal';
        chatModal.style.display = 'flex';
        chatModal.innerHTML = `
            <div class="modal-content ai-chat-content">
                <div class="ai-chat-header">
                    <h3>🤖 AI Emergency Gatekeeper</h3>
                    <div class="session-timer">
                        <span id="sessionTimer">10:00</span> remaining
                    </div>
                </div>
                
                <div class="ai-chat-messages" id="aiChatMessages">
                    <div class="ai-message">
                        <strong>AI Counselor:</strong> I understand you're experiencing a craving. Let's talk through this together. What's going on right now?
                    </div>
                </div>
                
                <div class="ai-chat-input">
                    <div class="input-group">
                        <input type="text" id="userMessage" placeholder="Tell me what you're feeling..." class="form-control">
                        <button id="sendMessage" class="btn btn-primary">Send</button>
                    </div>
                </div>
                
                <div class="ai-chat-footer">
                    <div class="session-info">
                        <span id="messageCount">0</span> messages • 
                        <span id="sessionStatus">Minimum 10 minutes conversation required</span>
                    </div>
                    <button id="completeSession" class="btn btn-success" disabled>Complete Session & Unlock</button>
                    <button id="cancelSession" class="btn btn-secondary">Cancel Session</button>
                </div>
            </div>
        `;
        
        document.body.appendChild(chatModal);
        
        // Initialize chat functionality
        this.initializeAIChat(chatModal);
    }

    initializeAIChat(chatModal) {
        const userMessageInput = chatModal.querySelector('#userMessage');
        const sendBtn = chatModal.querySelector('#sendMessage');
        const messagesContainer = chatModal.querySelector('#aiChatMessages');
        const completeBtn = chatModal.querySelector('#completeSession');
        const cancelBtn = chatModal.querySelector('#cancelSession');
        const timerDisplay = chatModal.querySelector('#sessionTimer');
        const messageCountDisplay = chatModal.querySelector('#messageCount');
        const statusDisplay = chatModal.querySelector('#sessionStatus');
        
        // Send message functionality
        const sendMessage = async () => {
            const message = userMessageInput.value.trim();
            if (!message) return;
            
            // Add user message to chat
            this.addChatMessage(messagesContainer, message, true);
            userMessageInput.value = '';
            
            // Send to AI
            const response = await this.apiCall('/api/ai/chat', 'POST', { message: message });
            
            if (response && response.success) {
                // Add AI response
                this.addChatMessage(messagesContainer, response.message, false);
                
                // Update session status
                this.aiSession.messageCount = response.messageCount;
                messageCountDisplay.textContent = response.messageCount;
                
                // Check if unlock is available
                if (response.canUnlock) {
                    completeBtn.disabled = false;
                    statusDisplay.textContent = 'Session requirements met - you may unlock';
                    statusDisplay.style.color = 'var(--success-color)';
                }
            }
        };
        
        sendBtn.addEventListener('click', sendMessage);
        userMessageInput.addEventListener('keypress', (e) => {
            if (e.key === 'Enter') {
                sendMessage();
            }
        });
        
        // Complete session
        completeBtn.addEventListener('click', async () => {
            const result = await this.apiCall('/api/emergency/ai/complete', 'POST');
            if (result && result.success) {
                chatModal.remove();
                this.showMessage(`Emergency unlock granted with ${result.penalty} minute penalty.`, 'success');
                this.updateStatus();
            } else {
                this.showMessage(result ? result.message : 'Session requirements not met', 'error');
            }
        });
        
        // Cancel session
        cancelBtn.addEventListener('click', () => {
            chatModal.remove();
            this.aiSession = null;
        });
        
        // Start session timer
        this.startAISessionTimer(timerDisplay, statusDisplay);
    }

    addChatMessage(container, message, isUser) {
        const messageDiv = document.createElement('div');
        messageDiv.className = isUser ? 'user-message' : 'ai-message';
        messageDiv.innerHTML = `<strong>${isUser ? 'You' : 'AI Counselor'}:</strong> ${message}`;
        container.appendChild(messageDiv);
        container.scrollTop = container.scrollHeight;
    }

    startAISessionTimer(timerDisplay, statusDisplay) {
        const updateTimer = () => {
            if (!this.aiSession) return;
            
            const elapsed = Date.now() - this.aiSession.startTime;
            const remaining = Math.max(0, this.aiSession.minDuration - elapsed);
            
            const minutes = Math.floor(remaining / 60000);
            const seconds = Math.floor((remaining % 60000) / 1000);
            
            timerDisplay.textContent = `${minutes}:${seconds.toString().padStart(2, '0')}`;
            
            if (remaining <= 0 && this.aiSession.messageCount >= 5) {
                statusDisplay.textContent = 'Session requirements met - you may unlock';
                statusDisplay.style.color = 'var(--success-color)';
                document.getElementById('completeSession').disabled = false;
            }
        };
        
        updateTimer();
        const timerInterval = setInterval(updateTimer, 1000);
        
        // Clear interval when modal is removed
        const observer = new MutationObserver((mutations) => {
            mutations.forEach((mutation) => {
                mutation.removedNodes.forEach((node) => {
                    if (node.classList && node.classList.contains('ai-chat-modal')) {
                        clearInterval(timerInterval);
                        observer.disconnect();
                    }
                });
            });
        });
        
        observer.observe(document.body, { childList: true });
    }

    startNormalEmergencyDelay() {
        const delayMinutes = 5; // Default 5-minute delay for normal mode
        
        this.showConfirmModal('Emergency Unlock', 
            `Emergency unlock requires a ${delayMinutes}-minute cooling-off period. This will add penalty time to your next interval. Continue?`, 
            () => {
                this.startEmergencyCountdown(delayMinutes);
            }
        );
    }

    startEmergencyCountdown(delayMinutes) {
        const startTime = Date.now();
        const delayMs = delayMinutes * 60 * 1000;
        
        // Create countdown modal
        const countdownModal = document.createElement('div');
        countdownModal.className = 'modal';
        countdownModal.style.display = 'flex';
        countdownModal.innerHTML = `
            <div class="modal-content">
                <h3>⏰ Emergency Cooling-Off Period</h3>
                <div style="text-align: center; padding: 20px;">
                    <p style="margin-bottom: 20px;">Take this time to reconsider. The box will unlock when the timer reaches zero.</p>
                    <div style="font-size: 2rem; font-weight: bold; color: var(--primary-color); margin: 20px 0;">
                        <span id="emergencyCountdown">${delayMinutes}:00</span>
                    </div>
                    <p style="color: var(--text-muted); font-size: 0.9rem;">
                        💭 Consider: What triggered this need? Can you try a healthier alternative?
                    </p>
                </div>
                <div class="modal-buttons">
                    <button id="cancelEmergency" class="btn-secondary">Cancel</button>
                    <button id="proceedEmergency" class="btn-primary" disabled>Unlock Box</button>
                </div>
            </div>
        `;
        
        document.body.appendChild(countdownModal);
        
        // Cancel button
        countdownModal.querySelector('#cancelEmergency').onclick = () => {
            clearInterval(this.emergencyTimer);
            countdownModal.remove();
        };
        
        // Proceed button (enabled after countdown)
        const proceedBtn = countdownModal.querySelector('#proceedEmergency');
        proceedBtn.onclick = () => {
            clearInterval(this.emergencyTimer);
            countdownModal.remove();
            this.emergencyUnlock();
        };
        
        // Countdown timer
        this.emergencyTimer = setInterval(() => {
            const elapsed = Date.now() - startTime;
            const remaining = delayMs - elapsed;
            
            if (remaining <= 0) {
                document.getElementById('emergencyCountdown').textContent = '0:00';
                proceedBtn.disabled = false;
                proceedBtn.textContent = 'Unlock Now';
                clearInterval(this.emergencyTimer);
            } else {
                const minutes = Math.floor(remaining / 60000);
                const seconds = Math.floor((remaining % 60000) / 1000);
                document.getElementById('emergencyCountdown').textContent = 
                    `${minutes}:${seconds.toString().padStart(2, '0')}`;
            }
        }, 1000);
    }

    startAIEmergencySession() {
        this.openAIEmergencyModal();
        this.initializeAIEmergencySession();
    }

    openAIEmergencyModal() {
        document.getElementById('aiEmergencyModal').style.display = 'flex';
        this.aiEmergencySessionStartTime = Date.now();
        this.aiEmergencySessionActive = true;
        this.aiEmergencyMessageCount = 0;
        this.startAIEmergencyTimer();
        this.bindAIEmergencyEvents();
    }

    bindAIEmergencyEvents() {
        // Bind event listeners for AI emergency modal
        document.getElementById('aiEmergencySend').onclick = () => this.sendAIEmergencyMessage();
        document.getElementById('aiEmergencyUserInput').onkeypress = (e) => {
            if (e.key === 'Enter') this.sendAIEmergencyMessage();
        };
        document.getElementById('aiEmergencyCancel').onclick = () => this.closeAIEmergencyModal();
        document.getElementById('aiEmergencyComplete').onclick = () => this.completeAIEmergencySession();
    }

    closeAIEmergencyModal() {
        document.getElementById('aiEmergencyModal').style.display = 'none';
        this.aiEmergencySessionActive = false;
        this.clearAIEmergencyConversation();
        if (this.aiEmergencyTimerInterval) {
            clearInterval(this.aiEmergencyTimerInterval);
        }
    }

    async initializeAIEmergencySession() {
        const aiSettings = await this.apiCall('/api/ai/config');
        const personality = aiSettings?.personality || 'supportive';
        
        const conversation = document.getElementById('aiEmergencyConversation');
        
        // Initial AI message
        const welcomeMessage = this.getEmergencyWelcomeMessage(personality);
        this.addAIEmergencyMessage(welcomeMessage);

        // Wait a moment, then ask the first question
        setTimeout(() => {
            const firstQuestion = this.getEmergencyFirstQuestion(personality);
            this.addAIEmergencyMessage(firstQuestion);
        }, 2000);

        // Start breathing exercise if enabled
        if (aiSettings?.breathingExercise) {
            setTimeout(() => {
                this.startBreathingExercise();
            }, 5000);
        }
    }

    getEmergencyWelcomeMessage(personality) {
        const messages = {
            supportive: "I can see you're going through a difficult moment. I'm here to help you through this craving. You're not alone in this journey. 💙",
            strict: "Stop right there. Before you make a decision you might regret, let's talk about what's really happening here. You've come too far to give up now.",
            friend: "Hey, I notice you're having a tough time. That's okay - we all have these moments. Let's chat about what's going on. I believe in you. 🤗",
            professional: "I understand you're experiencing a strong craving. This is a normal part of recovery. Let's work through this systematically and find healthier ways to cope."
        };
        return messages[personality] || messages.supportive;
    }

    getEmergencyFirstQuestion(personality) {
        const questions = {
            supportive: "Can you tell me what happened just before this craving hit? Sometimes identifying the trigger helps us understand it better.",
            strict: "What exactly triggered this craving? Be honest - was it stress, boredom, habit, or something else? We need to face this head-on.",
            friend: "What's been happening today that brought you to this point? No judgment here - just want to understand so I can help.",
            professional: "Let's identify the immediate trigger. Was this emotional (stress, anxiety), environmental (certain location), or social (seeing others smoke)?"
        };
        return questions[personality] || questions.supportive;
    }

    startAIEmergencyTimer() {
        const timerElement = document.getElementById('emergencySessionTime');
        const delayMinutes = 10; // Default to 10 minutes
        
        this.aiEmergencyTimerInterval = setInterval(() => {
            if (!this.aiEmergencySessionActive) {
                clearInterval(this.aiEmergencyTimerInterval);
                return;
            }

            const elapsed = Date.now() - this.aiEmergencySessionStartTime;
            const remaining = (delayMinutes * 60 * 1000) - elapsed;

            if (remaining <= 0) {
                timerElement.textContent = '0:00';
                document.getElementById('aiEmergencyComplete').disabled = false;
                this.addAIEmergencyMessage("The minimum session time is complete. You can now choose to proceed with the emergency unlock, but I encourage you to continue our conversation. How are you feeling right now?");
                clearInterval(this.aiEmergencyTimerInterval);
            } else {
                const minutes = Math.floor(remaining / 60000);
                const seconds = Math.floor((remaining % 60000) / 1000);
                timerElement.textContent = `${minutes}:${seconds.toString().padStart(2, '0')}`;
            }
        }, 1000);
    }

    sendAIEmergencyMessage() {
        const input = document.getElementById('aiEmergencyUserInput');
        const message = input.value.trim();
        
        if (!message) return;

        this.addUserEmergencyMessage(message);
        input.value = '';
        this.aiEmergencyMessageCount++;

        // Generate AI response after a brief delay
        setTimeout(() => {
            this.generateAIEmergencyResponse(message);
        }, 1000 + Math.random() * 2000);
    }

    addAIEmergencyMessage(message) {
        const conversation = document.getElementById('aiEmergencyConversation');
        const messageDiv = document.createElement('div');
        messageDiv.className = 'ai-message';
        messageDiv.innerHTML = `<div class="message-content">🤖 ${message}</div>`;
        conversation.appendChild(messageDiv);
        conversation.scrollTop = conversation.scrollHeight;
    }

    addUserEmergencyMessage(message) {
        const conversation = document.getElementById('aiEmergencyConversation');
        const messageDiv = document.createElement('div');
        messageDiv.className = 'user-message';
        messageDiv.innerHTML = `<div class="message-content">👤 ${message}</div>`;
        conversation.appendChild(messageDiv);
        conversation.scrollTop = conversation.scrollHeight;
    }

    generateAIEmergencyResponse(userMessage) {
        const lowerMessage = userMessage.toLowerCase();
        let response;

        // AI tries to dissuade and offer alternatives
        if (lowerMessage.includes('need') && lowerMessage.includes('cigarette')) {
            response = "I hear that you feel like you need a cigarette, but let's challenge that thought. Your body doesn't actually need nicotine - that's the addiction talking. What you might need is a way to cope with what you're feeling right now. What if we tried something else for just 5 minutes?";
        } else if (lowerMessage.includes('stress') || lowerMessage.includes('anxiety')) {
            response = "Stress and anxiety are tough, but smoking will only provide temporary relief and add guilt afterward. Have you tried the 4-7-8 breathing technique? Breathe in for 4, hold for 7, exhale for 8. It can be more effective than nicotine for stress relief.";
        } else if (lowerMessage.includes('angry') || lowerMessage.includes('frustrated')) {
            response = "Anger and frustration are valid emotions, but using cigarettes to cope will just add more frustration later when you realize you broke your streak. What if you did 10 jumping jacks or stepped outside for fresh air instead?";
        } else if (lowerMessage.includes('bored')) {
            response = "Boredom is a common trigger, but it's also an opportunity. Instead of filling the void with smoke, what if you called someone you care about, listened to a favorite song, or did a quick creative task? The boredom will pass either way.";
        } else if (lowerMessage.includes('habit') || lowerMessage.includes('routine')) {
            response = "Habits are powerful, but they can be changed. You're in the process of building a new, healthier routine. What usually came after smoking in your old routine? Let's find a healthier replacement for that part.";
        } else {
            // General responses that try to dissuade
            const responses = [
                "I understand this is difficult. But think about tomorrow morning - would you rather wake up proud of staying strong, or disappointed that you gave in? You have the power to choose.",
                "You've already proven you can resist cravings - you're here talking to me instead of immediately smoking. That shows incredible strength. What would it take to continue showing that strength for just a few more minutes?",
                "This feeling will pass whether you smoke or not. The difference is that if you don't smoke, you'll feel proud and your streak continues. If you do smoke, you'll feel guilty and have to start over. Which future do you prefer?",
                "Let's be honest - has smoking ever actually solved the underlying problem that's bothering you right now? Or does it just distract you temporarily while adding new problems?",
                "You quit for important reasons. What were they? Are those reasons still important to you today?",
                "Right now, your brain is trying to trick you into thinking you need this. But you've been proving for days/weeks that you don't actually need cigarettes. Trust the progress you've already made."
            ];
            response = responses[Math.floor(Math.random() * responses.length)];
        }

        this.addAIEmergencyMessage(response);

        // Occasionally offer specific coping strategies
        if (this.aiEmergencyMessageCount % 3 === 0) {
            setTimeout(() => {
                this.offerCopingStrategy();
            }, 3000);
        }
    }

    offerCopingStrategy() {
        const strategies = [
            "Quick coping idea: Try the 5-4-3-2-1 grounding technique. Name 5 things you can see, 4 you can touch, 3 you can hear, 2 you can smell, and 1 you can taste. This can help reset your focus.",
            "Here's something that might help: Drink a large glass of cold water slowly. It gives your hands and mouth something to do, and hydration can sometimes reduce cravings.",
            "Alternative action: Do 20 deep breaths while walking around your space. Movement + breathing can be more effective than nicotine for managing stress.",
            "Try this: Text or call someone who supports your quit journey. Social connection often satisfies the same need that triggers cravings.",
            "Practical tip: Brush your teeth or chew sugar-free gum. The fresh taste can help reset your craving and make cigarettes less appealing."
        ];
        
        const strategy = strategies[Math.floor(Math.random() * strategies.length)];
        this.addAIEmergencyMessage(strategy);
    }

    startBreathingExercise() {
        this.addAIEmergencyMessage("Let's try a powerful breathing exercise that's scientifically proven to reduce cravings. This will guide you through the 4-7-8 technique - just follow along! 🫁");
        
        // Add breathing exercise button
        setTimeout(() => {
            this.addBreathingExerciseOption();
        }, 1000);
    }

    addBreathingExerciseOption() {
        const chatContainer = document.getElementById('aiEmergencyConversation');
        const breathingButton = document.createElement('button');
        breathingButton.textContent = '🫁 Start Guided Breathing Exercise';
        breathingButton.className = 'btn-primary breathing-btn';
        breathingButton.style.margin = '10px 0';
        breathingButton.style.width = '100%';
        breathingButton.onclick = () => {
            breathingExercise.setSmokerBox(this);
            breathingExercise.start();
            breathingButton.style.display = 'none';
        };
        
        chatContainer.appendChild(breathingButton);
        
        // Also add a text message explaining the option
        setTimeout(() => {
            this.addAIEmergencyMessage("Click the button above to start the interactive breathing guide, or continue our conversation. The breathing exercise typically takes 2-3 minutes and many people find it incredibly helpful for cravings.");
        }, 500);
    }

    async completeAIEmergencySession() {
        // Final check - make sure they really want to proceed
        this.addAIEmergencyMessage("I respect your decision, but I want to ask one final question: After our conversation, do you still feel this is a genuine emergency that requires unlocking the box? Or have you found some clarity?");
        
        // Wait for response before allowing unlock
        const completeBtn = document.getElementById('aiEmergencyComplete');
        completeBtn.disabled = true;
        completeBtn.textContent = 'Waiting for response...';
        
        setTimeout(() => {
            completeBtn.disabled = false;
            completeBtn.textContent = 'Proceed with Emergency Unlock';
            completeBtn.onclick = () => this.finalEmergencyUnlock();
        }, 30000); // Wait 30 seconds
    }

    async finalEmergencyUnlock() {
        this.closeAIEmergencyModal();
        
        // Show final confirmation
        this.showConfirmModal('Final Confirmation', 
            'You completed the AI session. Emergency unlock will still add penalty time. Final confirmation to proceed?', 
            () => {
                this.emergencyUnlock();
                this.showMessage('Emergency unlock completed. Remember: you are stronger than your cravings! 💪', 'info');
            }
        );
    }

    clearAIEmergencyConversation() {
        document.getElementById('aiEmergencyConversation').innerHTML = '';
        document.getElementById('aiEmergencyComplete').disabled = true;
        document.getElementById('aiEmergencyComplete').textContent = 'Complete & Unlock';
        document.getElementById('emergencySessionTime').textContent = '10:00';
        this.aiEmergencyMessageCount = 0;
    }

    initializeLanguageSelector() {
        if (typeof i18n !== 'undefined') {
            const container = document.getElementById('languageSelectorContainer');
            if (container) {
                container.innerHTML = i18n.createLanguageSelector();
            }
        }
    }
}

// Initialize the application when the page loads
document.addEventListener('DOMContentLoaded', () => {
    window.smokingTimer = new SmokingTimerBox();
});

// Clean up when page unloads
window.addEventListener('beforeunload', () => {
    if (window.smokingTimer) {
        window.smokingTimer.stopPeriodicUpdates();
    }
});

// Add some visual feedback for interactions
document.addEventListener('click', (e) => {
    if (e.target.matches('button') || e.target.matches('.btn-action')) {
        e.target.style.transform = 'scale(0.95)';
        setTimeout(() => {
            e.target.style.transform = '';
        }, 150);
    }
});

// Add keyboard shortcuts
document.addEventListener('keydown', (e) => {
    if (e.ctrlKey || e.metaKey) {
        switch (e.key) {
            case 's':
                e.preventDefault();
                document.getElementById('configForm').dispatchEvent(new Event('submit'));
                break;
            case 'u':
                e.preventDefault();
                document.getElementById('unlockBtn').click();
                break;
        }
    }
});

// Breathing Exercise Class
class BreathingExercise {
    constructor() {
        this.isActive = false;
        this.currentPhase = 'inhale';
        this.cycleCount = 0;
        this.maxCycles = 8; // 4-7-8 breathing, 8 cycles
        this.smokerBox = null;
    }

    setSmokerBox(smokerBox) {
        this.smokerBox = smokerBox;
    }

    start() {
        if (this.isActive) return;
        
        this.isActive = true;
        this.cycleCount = 0;
        this.showBreathingInterface();
        this.runCycle();
    }

    showBreathingInterface() {
        const breathingHTML = `
            <div id="breathingExercise" class="breathing-container">
                <div class="breathing-header">
                    <h3>🫁 Breathing Exercise</h3>
                    <p>Let's practice the 4-7-8 breathing technique</p>
                </div>
                <div class="breathing-circle" id="breathingCircle"></div>
                <div class="breathing-text" id="breathingText">Prepare to breathe</div>
                <div class="breathing-counter" id="breathingCounter">Get ready...</div>
                <div class="breathing-progress">
                    <div class="progress-bar" id="breathingProgress"></div>
                    <div class="progress-text">Cycle <span id="currentCycle">0</span> of ${this.maxCycles}</div>
                </div>
                <div class="breathing-controls">
                    <button onclick="breathingExercise.stop()" class="btn-secondary">Stop Exercise</button>
                </div>
            </div>
        `;
        
        const container = document.getElementById('aiEmergencyConversation');
        const breathingDiv = document.createElement('div');
        breathingDiv.innerHTML = breathingHTML;
        container.appendChild(breathingDiv);
    }

    async runCycle() {
        while (this.isActive && this.cycleCount < this.maxCycles) {
            this.updateCycleDisplay();
            
            await this.inhale();
            if (!this.isActive) break;
            
            await this.hold();
            if (!this.isActive) break;
            
            await this.exhale();
            if (!this.isActive) break;
            
            this.cycleCount++;
            this.updateProgress();
            
            if (this.cycleCount < this.maxCycles) {
                await this.pause(1000); // 1 second between cycles
            }
        }
        
        if (this.isActive) {
            this.complete();
        }
    }

    async inhale() {
        this.currentPhase = 'inhale';
        const circle = document.getElementById('breathingCircle');
        const text = document.getElementById('breathingText');
        const counter = document.getElementById('breathingCounter');
        
        text.textContent = 'Breathe In Slowly';
        circle.classList.add('inhale');
        
        for (let i = 4; i > 0; i--) {
            if (!this.isActive) return;
            counter.textContent = i;
            await this.delay(1000);
        }
        
        circle.classList.remove('inhale');
    }

    async hold() {
        this.currentPhase = 'hold';
        const text = document.getElementById('breathingText');
        const counter = document.getElementById('breathingCounter');
        
        text.textContent = 'Hold Your Breath';
        
        for (let i = 7; i > 0; i--) {
            if (!this.isActive) return;
            counter.textContent = i;
            await this.delay(1000);
        }
    }

    async exhale() {
        this.currentPhase = 'exhale';
        const circle = document.getElementById('breathingCircle');
        const text = document.getElementById('breathingText');
        const counter = document.getElementById('breathingCounter');
        
        text.textContent = 'Breathe Out Slowly';
        circle.classList.add('exhale');
        
        for (let i = 8; i > 0; i--) {
            if (!this.isActive) return;
            counter.textContent = i;
            await this.delay(1000);
        }
        
        circle.classList.remove('exhale');
    }

    updateCycleDisplay() {
        const currentCycleElement = document.getElementById('currentCycle');
        if (currentCycleElement) {
            currentCycleElement.textContent = this.cycleCount + 1;
        }
    }

    updateProgress() {
        const progress = document.getElementById('breathingProgress');
        if (progress) {
            const percentage = (this.cycleCount / this.maxCycles) * 100;
            progress.style.width = percentage + '%';
        }
    }

    complete() {
        const text = document.getElementById('breathingText');
        const counter = document.getElementById('breathingCounter');
        
        text.textContent = 'Exercise Complete! Well Done! 🌟';
        counter.textContent = '✓';
        
        setTimeout(() => {
            this.stop();
            // Send completion message to AI chat
            this.sendCompletionMessage();
        }, 3000);
    }

    sendCompletionMessage() {
        if (this.smokerBox) {
            const message = "I completed the breathing exercise. I feel a bit calmer now and more centered.";
            // Add to chat and send to AI
            this.smokerBox.addAIEmergencyMessage(message, 'user');
            this.smokerBox.sendAIEmergencyMessage(message);
        }
    }

    stop() {
        this.isActive = false;
        const exerciseDiv = document.getElementById('breathingExercise');
        if (exerciseDiv) {
            exerciseDiv.remove();
        }
    }

    delay(ms) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }

    pause(ms) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }
}

// Initialize breathing exercise globally
const breathingExercise = new BreathingExercise();
