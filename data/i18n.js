// Multi-language support for Quit Smoking Timer Box
class I18n {
    constructor() {
        this.currentLanguage = 'en';
        this.translations = {};
        this.supportedLanguages = [];
        this.fallbackLanguage = 'en';
        
        this.loadTranslations();
        this.detectLanguage();
    }

    async loadTranslations() {
        try {
            const response = await fetch('/translations.json');
            const data = await response.json();
            this.translations = data.translations;
            this.supportedLanguages = data.languages;
            
            // Apply saved language preference
            const savedLanguage = localStorage.getItem('preferred_language');
            if (savedLanguage && this.translations[savedLanguage]) {
                this.currentLanguage = savedLanguage;
            }
            
            this.translatePage();
        } catch (error) {
            console.error('Failed to load translations:', error);
        }
    }

    detectLanguage() {
        // Detect browser language if no preference is saved
        if (!localStorage.getItem('preferred_language')) {
            const browserLang = navigator.language.split('-')[0];
            if (this.translations[browserLang]) {
                this.currentLanguage = browserLang;
                localStorage.setItem('preferred_language', browserLang);
            }
        }
    }

    setLanguage(languageCode) {
        if (this.translations[languageCode]) {
            this.currentLanguage = languageCode;
            localStorage.setItem('preferred_language', languageCode);
            this.translatePage();
            
            // Trigger custom event for other components
            window.dispatchEvent(new CustomEvent('languageChanged', { 
                detail: { language: languageCode } 
            }));
        }
    }

    t(key, fallback = null) {
        const translation = this.translations[this.currentLanguage]?.[key] 
                         || this.translations[this.fallbackLanguage]?.[key] 
                         || fallback 
                         || key;
        return translation;
    }

    translatePage() {
        // Translate elements with data-i18n attribute
        document.querySelectorAll('[data-i18n]').forEach(element => {
            const key = element.getAttribute('data-i18n');
            const translation = this.t(key);
            
            if (element.tagName === 'INPUT' && (element.type === 'text' || element.type === 'email' || element.type === 'password')) {
                element.placeholder = translation;
            } else {
                element.textContent = translation;
            }
        });

        // Translate elements with data-i18n-html attribute (for HTML content)
        document.querySelectorAll('[data-i18n-html]').forEach(element => {
            const key = element.getAttribute('data-i18n-html');
            const translation = this.t(key);
            element.innerHTML = translation;
        });

        // Translate page title
        const titleKey = document.querySelector('meta[name="i18n-title"]')?.content;
        if (titleKey) {
            document.title = this.t(titleKey);
        }

        // Update language selector
        this.updateLanguageSelector();
    }

    updateLanguageSelector() {
        const selector = document.getElementById('languageSelector');
        if (selector) {
            const currentLang = this.supportedLanguages.find(lang => lang.code === this.currentLanguage);
            if (currentLang) {
                selector.innerHTML = `${currentLang.flag} ${currentLang.name}`;
            }
        }
    }

    createLanguageSelector() {
        return `
            <div class="language-selector">
                <button class="language-btn" id="languageSelector" onclick="i18n.toggleLanguageMenu()">
                    🌐 Language
                </button>
                <div class="language-menu" id="languageMenu" style="display: none;">
                    ${this.supportedLanguages.map(lang => `
                        <button class="language-option ${lang.code === this.currentLanguage ? 'active' : ''}" 
                                onclick="i18n.setLanguage('${lang.code}')">
                            ${lang.flag} ${lang.name}
                        </button>
                    `).join('')}
                </div>
            </div>
        `;
    }

    toggleLanguageMenu() {
        const menu = document.getElementById('languageMenu');
        if (menu) {
            menu.style.display = menu.style.display === 'none' ? 'block' : 'none';
        }
    }

    getCurrentLanguage() {
        return this.currentLanguage;
    }

    getSupportedLanguages() {
        return this.supportedLanguages;
    }

    // Format numbers according to current locale
    formatNumber(number, options = {}) {
        const lang = this.currentLanguage === 'pt' ? 'pt-PT' : this.currentLanguage;
        return new Intl.NumberFormat(lang, options).format(number);
    }

    // Format currency according to current locale
    formatCurrency(amount, currency = 'EUR') {
        const lang = this.currentLanguage === 'pt' ? 'pt-PT' : this.currentLanguage;
        return new Intl.NumberFormat(lang, {
            style: 'currency',
            currency: currency
        }).format(amount);
    }

    // Format dates according to current locale
    formatDate(date, options = {}) {
        const lang = this.currentLanguage === 'pt' ? 'pt-PT' : this.currentLanguage;
        return new Intl.DateTimeFormat(lang, options).format(date);
    }

    // Get direction for RTL languages (future enhancement)
    getDirection() {
        return 'ltr'; // All current languages are LTR
    }
}

// Global instance
const i18n = new I18n();

// Close language menu when clicking outside
document.addEventListener('click', (event) => {
    const menu = document.getElementById('languageMenu');
    const selector = document.getElementById('languageSelector');
    
    if (menu && selector && !selector.contains(event.target) && !menu.contains(event.target)) {
        menu.style.display = 'none';
    }
});

// Export for module usage
if (typeof module !== 'undefined' && module.exports) {
    module.exports = I18n;
}
