// Quit Smoking Timer Box - Web Interface JavaScript
class SmokingTimerBox {
    constructor() {
        this.apiBase = '';
        this.updateInterval = null;
        this.currentState = {};
        
        this.initializeEventListeners();
        this.startPeriodicUpdates();
        this.loadConfiguration();
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
            this.showConfirmModal('Emergency Unlock', 
                'Emergency unlock will add penalty time to your next interval. Continue?', 
                () => {
                    this.emergencyUnlock();
                }
            );
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
            document.getElementById('timerMode').value = config.timerMode || 0;
            document.getElementById('intervalMinutes').value = config.intervalMinutes || 30;
            document.getElementById('dailyLimit').value = config.dailyLimit || 10;
            document.getElementById('emergencyUnlocks').value = config.emergencyUnlocks || 3;
            
            this.updateFormVisibility(parseInt(config.timerMode || 0));
        }
    }

    async saveConfiguration() {
        const formData = new FormData(document.getElementById('configForm'));
        const config = {
            timerMode: parseInt(formData.get('timerMode')),
            intervalMinutes: parseInt(formData.get('intervalMinutes')),
            dailyLimit: parseInt(formData.get('dailyLimit')),
            emergencyUnlocks: parseInt(formData.get('emergencyUnlocks'))
        };

        const result = await this.apiCall('/api/config', 'POST', config);
        if (result && result.success) {
            this.showMessage('Configuration saved successfully!', 'success');
            this.updateStatus();
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

        // Update status values
        document.getElementById('boxState').textContent = this.getStateText(status.boxState);
        document.getElementById('timeRemaining').textContent = this.formatTime(status.timeRemaining || 0);
        document.getElementById('todayCount').textContent = status.todayCount || 0;
        document.getElementById('smokeFree').textContent = status.smokeFree || 0;

        // Update statistics
        document.getElementById('totalSaved').textContent = `$${(status.totalSaved || 0).toFixed(2)}`;
        document.getElementById('totalCigarettes').textContent = status.totalCigarettes || 0;
        document.getElementById('totalDays').textContent = status.totalDays || 0;
        document.getElementById('longestStreak').textContent = status.longestStreak || 0;

        // Update button states
        this.updateButtonStates(status);
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
                intervalGroup.style.display = 'none';
                dailyLimitGroup.style.display = 'none';
                break;
            case 3: // Emergency Only
                intervalGroup.style.display = 'none';
                dailyLimitGroup.style.display = 'none';
                break;
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
