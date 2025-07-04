#include "preferences_manager.h"
#include "WiTcontroller.h"

// Variables globales
Preferences nvsPrefs;
bool nvsInit = false;
bool nvsPrefsSaved = false;
bool preferencesRead = false;

// Variables de configuración cacheadas
String cachedLastSSID = "";
String cachedLastServerIP = "";
int cachedLastServerPort = 0;
int cachedSpeedMultiplier = 1;
bool cachedHeartbeatEnabled = true;
int cachedMaxThrottles = 2;
int cachedBatteryShowMode = 0;

void preferences_init() {
    debug_println("Initializing preferences...");
    
    setupPreferences(false);
    readPreferences();
    
    debug_println("Preferences initialized");
}

void preferences_loop() {
    // Guardar preferencias automáticamente cada 30 segundos si hay cambios
    static unsigned long lastSave = 0;
    if (!nvsPrefsSaved && (millis() - lastSave > 30000)) {
        writePreferences();
        lastSave = millis();
    }
}

void preferences_reset() {
    clearPreferences();
    preferences_init();
}

void setupPreferences(bool forceClear) {
    debug_println("setupPreferences()");
    
    if (forceClear) {
        debug_println("Force clearing preferences");
        clearPreferences();
    }
    
    if (!nvsInit) {
        nvsPrefs.begin(PREF_NAMESPACE, false);
        nvsInit = true;
        debug_println("NVS preferences initialized");
    }
    
    // Verificar si es la primera ejecución
    if (!nvsPrefs.isKey("initialized")) {
        debug_println("First run - setting default preferences");
        nvsPrefs.putBool("initialized", true);
        setPreferencesVersion(1);
        
        // Establecer valores por defecto
        saveSpeedMultiplier(1);
        saveHeartbeatEnabled(true);
        saveMaxThrottles(2);
        saveBatteryShowMode(0);
        saveAutoConnect(false);
        saveDebugLevel(0);
        saveDisplayBrightness(128);
        saveDisplayContrast(128);
        saveDisplayInvert(false);
        
        nvsPrefsSaved = true;
    }
}

void readPreferences() {
    debug_println("readPreferences()");
    
    if (!nvsInit) {
        setupPreferences(false);
    }
    
    // Leer configuración general
    cachedSpeedMultiplier = getSpeedMultiplier();
    cachedHeartbeatEnabled = getHeartbeatEnabled();
    cachedMaxThrottles = getMaxThrottles();
    cachedBatteryShowMode = getBatteryShowMode();
    
    // Leer última configuración de red
    cachedLastSSID = getLastUsedSSID();
    cachedLastServerIP = getLastServerIP();
    cachedLastServerPort = getLastServerPort();
    
    // Cargar configuración de display
    int brightness = getDisplayBrightness();
    int contrast = getDisplayContrast();
    bool invert = getDisplayInvert();
    
    debug_print("Loaded preferences - SSID: ");
    debug_print(cachedLastSSID);
    debug_print(", Server: ");
    debug_print(cachedLastServerIP);
    debug_print(":");
    debug_println(cachedLastServerPort);
    
    preferencesRead = true;
}

void writePreferences() {
    debug_println("writePreferences()");
    
    if (!nvsInit) {
        debug_println("NVS not initialized, skipping write");
        return;
    }
    
    try {
        // Marcar como guardado
        nvsPrefsSaved = true;
        
        debug_println("Preferences saved successfully");
    } catch (const std::exception& e) {
        debug_print("Error saving preferences: ");
        debug_println(e.what());
        nvsPrefsSaved = false;
    }
}

void clearPreferences() {
    debug_println("clearPreferences()");
    
    if (nvsInit) {
        nvsPrefs.clear();
        debug_println("Preferences cleared");
    }
    
    // Reinicializar
    nvsInit = false;
    nvsPrefsSaved = false;
    preferencesRead = false;
    
    // Limpiar cache
    cachedLastSSID = "";
    cachedLastServerIP = "";
    cachedLastServerPort = 0;
    cachedSpeedMultiplier = 1;
    cachedHeartbeatEnabled = true;
    cachedMaxThrottles = 2;
    cachedBatteryShowMode = 0;
}

void saveSSIDPreferences(const String& ssid, const String& password) {
    debug_print("Saving SSID preferences: ");
    debug_println(ssid);
    
    if (!nvsInit) return;
    
    // Guardar SSID como último usado
    nvsPrefs.putString(PREF_LAST_SSID, ssid);
    
    // Guardar contraseña (encriptada en implementación real)
    String passwordKey = String(PREF_PASSWORD_PREFIX) + ssid;
    nvsPrefs.putString(passwordKey.c_str(), password);
    
    cachedLastSSID = ssid;
    nvsPrefsSaved = false;
}

void loadSSIDPreferences() {
    debug_println("Loading SSID preferences");
    
    if (!nvsInit) return;
    
    cachedLastSSID = nvsPrefs.getString(PREF_LAST_SSID, "");
    
    debug_print("Loaded last SSID: ");
    debug_println(cachedLastSSID);
}

String getLastUsedSSID() {
    if (!nvsInit) return "";
    return nvsPrefs.getString(PREF_LAST_SSID, "");
}

String getPasswordForSSID(const String& ssid) {
    if (!nvsInit) return "";
    
    String passwordKey = String(PREF_PASSWORD_PREFIX) + ssid;
    return nvsPrefs.getString(passwordKey.c_str(), "");
}

void saveServerPreferences(const String& ip, int port) {
    debug_print("Saving server preferences: ");
    debug_print(ip);
    debug_print(":");
    debug_println(port);
    
    if (!nvsInit) return;
    
    nvsPrefs.putString(PREF_LAST_SERVER_IP, ip);
    nvsPrefs.putInt(PREF_LAST_SERVER_PORT, port);
    
    cachedLastServerIP = ip;
    cachedLastServerPort = port;
    nvsPrefsSaved = false;
}

void loadServerPreferences() {
    debug_println("Loading server preferences");
    
    if (!nvsInit) return;
    
    cachedLastServerIP = nvsPrefs.getString(PREF_LAST_SERVER_IP, "");
    cachedLastServerPort = nvsPrefs.getInt(PREF_LAST_SERVER_PORT, 12090);
    
    debug_print("Loaded last server: ");
    debug_print(cachedLastServerIP);
    debug_print(":");
    debug_println(cachedLastServerPort);
}

String getLastServerIP() {
    if (!nvsInit) return "";
    return nvsPrefs.getString(PREF_LAST_SERVER_IP, "");
}

int getLastServerPort() {
    if (!nvsInit) return 12090;
    return nvsPrefs.getInt(PREF_LAST_SERVER_PORT, 12090);
}

void saveThrottleConfig(int throttleIndex, const ThrottleConfig& config) {
    debug_print("Saving throttle config for throttle: ");
    debug_println(throttleIndex);
    
    if (!nvsInit || throttleIndex < 0 || throttleIndex >= 6) return;
    
    String prefix = "throttle" + String(throttleIndex) + "_";
    
    nvsPrefs.putInt((prefix + "locoCount").c_str(), config.locoCount);
    nvsPrefs.putBool((prefix + "active").c_str(), config.active);
    
    for (int i = 0; i < config.locoCount && i < 6; i++) {
        String locoPrefix = prefix + "loco" + String(i) + "_";
        nvsPrefs.putInt((locoPrefix + "addr").c_str(), config.locos[i].address);
        nvsPrefs.putString((locoPrefix + "name").c_str(), config.locos[i].name);
        nvsPrefs.putBool((locoPrefix + "dir").c_str(), config.locos[i].direction);
        nvsPrefs.putInt((locoPrefix + "speed").c_str(), config.locos[i].speed);
        
        // Guardar estados de funciones (como bitmap)
        uint32_t functionBits = 0;
        for (int j = 0; j < 32; j++) {
            if (config.locos[i].functions[j]) {
                functionBits |= (1 << j);
            }
        }
        nvsPrefs.putUInt((locoPrefix + "funcs").c_str(), functionBits);
    }
    
    nvsPrefsSaved = false;
}

void loadThrottleConfig(int throttleIndex, ThrottleConfig& config) {
    debug_print("Loading throttle config for throttle: ");
    debug_println(throttleIndex);
    
    if (!nvsInit || throttleIndex < 0 || throttleIndex >= 6) {
        // Inicializar con valores por defecto
        config.locoCount = 0;
        config.active = false;
        return;
    }
    
    String prefix = "throttle" + String(throttleIndex) + "_";
    
    config.locoCount = nvsPrefs.getInt((prefix + "locoCount").c_str(), 0);
    config.active = nvsPrefs.getBool((prefix + "active").c_str(), false);
    
    for (int i = 0; i < config.locoCount && i < 6; i++) {
        String locoPrefix = prefix + "loco" + String(i) + "_";
        config.locos[i].address = nvsPrefs.getInt((locoPrefix + "addr").c_str(), 0);
        config.locos[i].name = nvsPrefs.getString((locoPrefix + "name").c_str(), "");
        config.locos[i].direction = nvsPrefs.getBool((locoPrefix + "dir").c_str(), true);
        config.locos[i].speed = nvsPrefs.getInt((locoPrefix + "speed").c_str(), 0);
        
        // Cargar estados de funciones
        uint32_t functionBits = nvsPrefs.getUInt((locoPrefix + "funcs").c_str(), 0);
        for (int j = 0; j < 32; j++) {
            config.locos[i].functions[j] = (functionBits & (1 << j)) != 0;
        }
    }
}

void saveSpeedMultiplier(int multiplier) {
    if (!nvsInit) return;
    nvsPrefs.putInt(PREF_SPEED_MULTIPLIER, multiplier);
    cachedSpeedMultiplier = multiplier;
    nvsPrefsSaved = false;
}

int getSpeedMultiplier() {
    if (!nvsInit) return 1;
    return nvsPrefs.getInt(PREF_SPEED_MULTIPLIER, 1);
}

void saveHeartbeatEnabled(bool enabled) {
    if (!nvsInit) return;
    nvsPrefs.putBool(PREF_HEARTBEAT_ENABLED, enabled);
    cachedHeartbeatEnabled = enabled;
    nvsPrefsSaved = false;
}

bool getHeartbeatEnabled() {
    if (!nvsInit) return true;
    return nvsPrefs.getBool(PREF_HEARTBEAT_ENABLED, true);
}

void saveMaxThrottles(int maxThrottles) {
    if (!nvsInit) return;
    nvsPrefs.putInt(PREF_MAX_THROTTLES, maxThrottles);
    cachedMaxThrottles = maxThrottles;
    nvsPrefsSaved = false;
}

int getMaxThrottles() {
    if (!nvsInit) return 2;
    return nvsPrefs.getInt(PREF_MAX_THROTTLES, 2);
}

void saveBatteryShowMode(int showMode) {
    if (!nvsInit) return;
    nvsPrefs.putInt(PREF_BATTERY_SHOW, showMode);
    cachedBatteryShowMode = showMode;
    nvsPrefsSaved = false;
}

int getBatteryShowMode() {
    if (!nvsInit) return 0;
    return nvsPrefs.getInt(PREF_BATTERY_SHOW, 0);
}

void saveDisplayBrightness(int brightness) {
    if (!nvsInit) return;
    nvsPrefs.putInt(PREF_BRIGHTNESS, brightness);
    nvsPrefsSaved = false;
}

int getDisplayBrightness() {
    if (!nvsInit) return 128;
    return nvsPrefs.getInt(PREF_BRIGHTNESS, 128);
}

void saveDisplayContrast(int contrast) {
    if (!nvsInit) return;
    nvsPrefs.putInt(PREF_CONTRAST, contrast);
    nvsPrefsSaved = false;
}

int getDisplayContrast() {
    if (!nvsInit) return 128;
    return nvsPrefs.getInt(PREF_CONTRAST, 128);
}

void saveDisplayInvert(bool invert) {
    if (!nvsInit) return;
    nvsPrefs.putBool(PREF_INVERT_DISPLAY, invert);
    nvsPrefsSaved = false;
}

bool getDisplayInvert() {
    if (!nvsInit) return false;
    return nvsPrefs.getBool(PREF_INVERT_DISPLAY, false);
}

void saveAutoConnect(bool autoConnect) {
    if (!nvsInit) return;
    nvsPrefs.putBool(PREF_AUTO_CONNECT, autoConnect);
    nvsPrefsSaved = false;
}

bool getAutoConnect() {
    if (!nvsInit) return false;
    return nvsPrefs.getBool(PREF_AUTO_CONNECT, false);
}

void saveDebugLevel(int debugLevel) {
    if (!nvsInit) return;
    nvsPrefs.putInt(PREF_DEBUG_LEVEL, debugLevel);
    nvsPrefsSaved = false;
}

int getDebugLevel() {
    if (!nvsInit) return 0;
    return nvsPrefs.getInt(PREF_DEBUG_LEVEL, 0);
}

bool isPreferencesAvailable() {
    return nvsInit && preferencesRead;
}

void backupPreferences() {
    debug_println("Backing up preferences...");
    // En una implementación real, esto podría guardar en SPIFFS o SD
    exportPreferencesToSerial();
}

void restorePreferences() {
    debug_println("Restoring preferences...");
    // En una implementación real, esto cargaría desde backup
}

void exportPreferencesToSerial() {
    debug_println("=== PREFERENCES EXPORT ===");
    debug_println("Last SSID: " + getLastUsedSSID());
    debug_println("Last Server: " + getLastServerIP() + ":" + String(getLastServerPort()));
    debug_println("Speed Multiplier: " + String(getSpeedMultiplier()));
    debug_println("Heartbeat: " + String(getHeartbeatEnabled() ? "enabled" : "disabled"));
    debug_println("Max Throttles: " + String(getMaxThrottles()));
    debug_println("Battery Show Mode: " + String(getBatteryShowMode()));
    debug_println("Display Brightness: " + String(getDisplayBrightness()));
    debug_println("Display Contrast: " + String(getDisplayContrast()));
    debug_println("Display Inverted: " + String(getDisplayInvert() ? "yes" : "no"));
    debug_println("Auto Connect: " + String(getAutoConnect() ? "enabled" : "disabled"));
    debug_println("Debug Level: " + String(getDebugLevel()));
    debug_println("=== END EXPORT ===");
}

void importPreferencesFromSerial() {
    debug_println("Import from serial not implemented in this example");
}

void migratePreferences(int fromVersion, int toVersion) {
    debug_print("Migrating preferences from version ");
    debug_print(fromVersion);
    debug_print(" to ");
    debug_println(toVersion);
    
    // Implementar lógica de migración según sea necesario
    switch (fromVersion) {
        case 1:
            if (toVersion >= 2) {
                // Migración de v1 a v2
                // Ejemplo: agregar nuevas preferencias por defecto
            }
            break;
        default:
            debug_println("No migration needed");
            break;
    }
    
    setPreferencesVersion(toVersion);
}

int getPreferencesVersion() {
    if (!nvsInit) return 1;
    return nvsPrefs.getInt("version", 1);
}

void setPreferencesVersion(int version) {
    if (!nvsInit) return;
    nvsPrefs.putInt("version", version);
    nvsPrefsSaved = false;
}