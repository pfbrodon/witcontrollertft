#ifndef PREFERENCES_MANAGER_H
#define PREFERENCES_MANAGER_H

#include <Preferences.h>
#include <String.h>

// Claves de preferencias
#define PREF_NAMESPACE "WiTcontroller"
#define PREF_SSID_COUNT "ssidCount"
#define PREF_SSID_PREFIX "ssid"
#define PREF_PASSWORD_PREFIX "pass"
#define PREF_LAST_SSID "lastSSID"
#define PREF_LAST_SERVER_IP "lastServerIP"
#define PREF_LAST_SERVER_PORT "lastServerPort"
#define PREF_SPEED_MULTIPLIER "speedMult"
#define PREF_HEARTBEAT_ENABLED "heartbeat"
#define PREF_MAX_THROTTLES "maxThrottles"
#define PREF_BATTERY_SHOW "batteryShow"
#define PREF_BRIGHTNESS "brightness"
#define PREF_CONTRAST "contrast"
#define PREF_INVERT_DISPLAY "invertDisp"
#define PREF_AUTO_CONNECT "autoConnect"
#define PREF_DEBUG_LEVEL "debugLevel"

// Estructura para configuración de locomotora
struct LocoConfig {
    int address;
    String name;
    bool direction;
    int speed;
    bool functions[32];
};

// Estructura para configuración de throttle
struct ThrottleConfig {
    LocoConfig locos[6]; // Máximo 6 locomotoras por throttle
    int locoCount;
    bool active;
};

// Variables globales
extern Preferences nvsPrefs;
extern bool nvsInit;
extern bool nvsPrefsSaved;
extern bool preferencesRead;

// Funciones principales del módulo
void preferences_init();
void preferences_loop();
void preferences_reset();

// Funciones de configuración del sistema
void setupPreferences(bool forceClear = false);
void readPreferences();
void writePreferences();
void clearPreferences();

// Funciones de gestión de SSIDs
void saveSSIDPreferences(const String& ssid, const String& password);
void loadSSIDPreferences();
String getLastUsedSSID();
String getPasswordForSSID(const String& ssid);

// Funciones de gestión de servidores
void saveServerPreferences(const String& ip, int port);
void loadServerPreferences();
String getLastServerIP();
int getLastServerPort();

// Funciones de configuración de throttle
void saveThrottleConfig(int throttleIndex, const ThrottleConfig& config);
void loadThrottleConfig(int throttleIndex, ThrottleConfig& config);
void saveAllThrottleConfigs();
void loadAllThrottleConfigs();

// Funciones de configuración de locomotoras
void saveLocoConfig(int throttleIndex, int locoIndex, const LocoConfig& config);
void loadLocoConfig(int throttleIndex, int locoIndex, LocoConfig& config);
void clearLocoConfig(int throttleIndex, int locoIndex);

// Funciones de configuración general
void saveSpeedMultiplier(int multiplier);
int getSpeedMultiplier();
void saveHeartbeatEnabled(bool enabled);
bool getHeartbeatEnabled();
void saveMaxThrottles(int maxThrottles);
int getMaxThrottles();
void saveBatteryShowMode(int showMode);
int getBatteryShowMode();

// Funciones de configuración de display
void saveDisplayBrightness(int brightness);
int getDisplayBrightness();
void saveDisplayContrast(int contrast);
int getDisplayContrast();
void saveDisplayInvert(bool invert);
bool getDisplayInvert();

// Funciones de configuración de conexión
void saveAutoConnect(bool autoConnect);
bool getAutoConnect();
void saveDebugLevel(int debugLevel);
int getDebugLevel();

// Funciones de utilidad
bool isPreferencesAvailable();
void backupPreferences();
void restorePreferences();
void exportPreferencesToSerial();
void importPreferencesFromSerial();

// Funciones de migración
void migratePreferences(int fromVersion, int toVersion);
int getPreferencesVersion();
void setPreferencesVersion(int version);

#endif // PREFERENCES_MANAGER_H