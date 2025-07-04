#ifndef BATTERY_MONITOR_H
#define BATTERY_MONITOR_H

#include "Pangodream_18650_CL.h"
#include "config_keypad_etc.h"

// Constantes de batería
#define BATTERY_CHECK_INTERVAL 5000    // 5 segundos
#define BATTERY_LOW_THRESHOLD 15       // 15%
#define BATTERY_CRITICAL_THRESHOLD 5   // 5%
#define BATTERY_SLEEP_THRESHOLD 3      // 3%
#define BATTERY_VOLTAGE_SAMPLES 5      // Promedio de 5 muestras

// Estados de visualización de batería
enum ShowBattery {
    NONE = 0,
    ICON_ONLY = 1,
    ICON_AND_PERCENT = 2
};

// Estados de alerta de batería
enum BatteryAlert {
    BATTERY_OK = 0,
    BATTERY_LOW = 1,
    BATTERY_CRITICAL = 2,
    BATTERY_SLEEP = 3
};

// Estructura de información de batería
struct BatteryInfo {
    int percentage;
    float voltage;
    bool isCharging;
    bool isLow;
    bool isCritical;
    BatteryAlert alertLevel;
    unsigned long lastUpdate;
    bool isValid;
};

// Variables globales de batería
extern bool useBatteryTest;
extern ShowBattery showBatteryTest;
extern bool useBatteryPercentAsWellAsIcon;
extern int lastBatteryTestValue;
extern int lastBatteryAnalogReadValue;
extern double lastBatteryCheckTime;
extern int useBatterySleepAtPercent;

#if USE_BATTERY_TEST
extern Pangodream_18650_CL BL;
#endif

// Funciones principales del módulo
void battery_monitor_init();
void battery_monitor_loop();
void battery_monitor_reset();

// Funciones de medición
void battery_updateReading();
void battery_updateReading(bool forceUpdate);
int battery_readRawValue();
float battery_calculateVoltage(int rawValue);
int battery_calculatePercentage(float voltage);
bool battery_isCharging();

// Funciones de gestión de estado
BatteryInfo battery_getInfo();
int battery_getPercentage();
float battery_getVoltage();
bool battery_isLow();
bool battery_isCritical();
BatteryAlert battery_getAlertLevel();

// Funciones de visualización
void battery_updateDisplay();
String battery_getIconString();
String battery_getPercentString();
String battery_getStatusString();
void battery_showAlert(BatteryAlert alert);

// Funciones de configuración
void battery_setShowMode(ShowBattery mode);
ShowBattery battery_getShowMode();
void battery_setUsePercent(bool usePercent);
bool battery_getUsePercent();
void battery_setSleepThreshold(int percentage);
int battery_getSleepThreshold();

// Funciones de calibración
void battery_calibrate();
void battery_setConversionFactor(float factor);
float battery_getConversionFactor();
void battery_resetCalibration();
void battery_debugReading();

// Funciones de gestión de energía
void battery_checkSleepCondition();
void battery_enterSleepMode();
void battery_handleLowBattery();
void battery_handleCriticalBattery();

// Funciones de filtrado de señal
void battery_addSample(int sample);
int battery_getAverageSample();
void battery_clearSamples();
bool battery_isSampleStable();

// Funciones de configuración avanzada
void battery_setPin(int pin);
int battery_getPin();
void battery_setUpdateInterval(unsigned long interval);
unsigned long battery_getUpdateInterval();
void battery_setLowThreshold(int percentage);
int battery_getLowThreshold();
void battery_setCriticalThreshold(int percentage);
int battery_getCriticalThreshold();

// Funciones de utilidad
void battery_toggle();
void battery_showToggle();
String battery_formatVoltage(float voltage);
String battery_formatPercentage(int percentage);
bool battery_isEnabled();

// Funciones de diagnóstico
void battery_runDiagnostic();
void battery_exportDiagnostic();
void battery_testAccuracy();
void battery_logReading();

#endif // BATTERY_MONITOR_H