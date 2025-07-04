#include "battery_monitor.h"
#include "WiTcontroller.h"
#include "display_manager.h"
#include "preferences_manager.h"

// Variables globales de batería
bool useBatteryTest = USE_BATTERY_TEST;
ShowBattery showBatteryTest = NONE;
bool useBatteryPercentAsWellAsIcon = USE_BATTERY_PERCENT_AS_WELL_AS_ICON;
int lastBatteryTestValue = 100;
int lastBatteryAnalogReadValue = 0;
double lastBatteryCheckTime = -10000;
int useBatterySleepAtPercent = USE_BATTERY_SLEEP_AT_PERCENT;

#if USE_BATTERY_TEST
Pangodream_18650_CL BL(BATTERY_TEST_PIN, BATTERY_CONVERSION_FACTOR);
#endif

// Variables internas
BatteryInfo batteryInfo;
int batterySamples[BATTERY_VOLTAGE_SAMPLES];
int sampleIndex = 0;
int sampleCount = 0;
unsigned long batteryUpdateInterval = BATTERY_CHECK_INTERVAL;
float conversionFactor = BATTERY_CONVERSION_FACTOR;
int batteryPin = BATTERY_TEST_PIN;
bool batteryCalibrated = false;

// Variables de umbral
int lowThreshold = BATTERY_LOW_THRESHOLD;
int criticalThreshold = BATTERY_CRITICAL_THRESHOLD;
int sleepThreshold = BATTERY_SLEEP_THRESHOLD;

void battery_monitor_init() {
    debug_println("Initializing battery monitor...");
    
    if (!useBatteryTest) {
        debug_println("Battery monitoring disabled");
        showBatteryTest = NONE;
        return;
    }
    
    // Configurar pin de batería
    pinMode(batteryPin, INPUT);
    
    // Inicializar estructura de información
    batteryInfo.percentage = 100;
    batteryInfo.voltage = 4.2;
    batteryInfo.isCharging = false;
    batteryInfo.isLow = false;
    batteryInfo.isCritical = false;
    batteryInfo.alertLevel = BATTERY_OK;
    batteryInfo.lastUpdate = 0;
    batteryInfo.isValid = false;
    
    // Configurar modo de visualización
    #if USE_BATTERY_PERCENT_AS_WELL_AS_ICON
    showBatteryTest = ICON_AND_PERCENT;
    #else
    showBatteryTest = ICON_ONLY;
    #endif
    
    // Cargar configuración guardada
    showBatteryTest = (ShowBattery)getBatteryShowMode();
    sleepThreshold = getSleepThreshold();
    
    // Limpiar muestras
    battery_clearSamples();
    
    // Realizar primera lectura
    battery_updateReading(true);
    
    debug_println("Battery monitor initialized");
}

void battery_monitor_loop() {
    if (!useBatteryTest) return;
    
    // Actualizar lectura periódicamente
    if (millis() - lastBatteryCheckTime > batteryUpdateInterval) {
        battery_updateReading();
        battery_checkSleepCondition();
    }
    
    // Manejar alertas de batería baja
    if (batteryInfo.isLow && !batteryInfo.isCritical) {
        battery_handleLowBattery();
    } else if (batteryInfo.isCritical) {
        battery_handleCriticalBattery();
    }
}

void battery_monitor_reset() {
    if (!useBatteryTest) return;
    
    battery_clearSamples();
    batteryInfo.isValid = false;
    lastBatteryCheckTime = -10000;
    
    debug_println("Battery monitor reset");
}

void battery_updateReading() {
    battery_updateReading(false);
}

void battery_updateReading(bool forceUpdate) {
    if (!useBatteryTest) return;
    
    if (!forceUpdate && (millis() - lastBatteryCheckTime < batteryUpdateInterval)) {
        return;
    }
    
    #if USE_BATTERY_TEST
    // Leer valor analógico
    int rawValue = battery_readRawValue();
    lastBatteryAnalogReadValue = rawValue;
    
    // Agregar muestra para promedio
    battery_addSample(rawValue);
    
    // Calcular porcentaje usando la librería Pangodream
    int percentage = BL.getBatteryChargeLevel();
    float voltage = BL.getBatteryVolts();
    
    // Validar lectura
    if (percentage >= 0 && percentage <= 100) {
        lastBatteryTestValue = percentage;
        batteryInfo.percentage = percentage;
        batteryInfo.voltage = voltage;
        batteryInfo.isCharging = battery_isCharging();
        batteryInfo.isLow = (percentage <= lowThreshold);
        batteryInfo.isCritical = (percentage <= criticalThreshold);
        batteryInfo.lastUpdate = millis();
        batteryInfo.isValid = true;
        
        // Determinar nivel de alerta
        if (percentage <= sleepThreshold) {
            batteryInfo.alertLevel = BATTERY_SLEEP;
        } else if (percentage <= criticalThreshold) {
            batteryInfo.alertLevel = BATTERY_CRITICAL;
        } else if (percentage <= lowThreshold) {
            batteryInfo.alertLevel = BATTERY_LOW;
        } else {
            batteryInfo.alertLevel = BATTERY_OK;
        }
        
        lastBatteryCheckTime = millis();
        
        // Debug si está habilitado
        #if WITCONTROLLER_DEBUG == 0
        battery_debugReading();
        #endif
        
        debug_print("Battery: ");
        debug_print(percentage);
        debug_print("% (");
        debug_print(voltage);
        debug_println("V)");
    }
    #endif
}

int battery_readRawValue() {
    if (!useBatteryTest) return 0;
    
    // Leer múltiples muestras para reducir ruido
    int total = 0;
    for (int i = 0; i < 5; i++) {
        total += analogRead(batteryPin);
        delay(1);
    }
    
    return total / 5;
}

float battery_calculateVoltage(int rawValue) {
    // Convertir valor ADC a voltaje usando el factor de conversión
    return (rawValue * 3.3 / 4095.0) * conversionFactor;
}

int battery_calculatePercentage(float voltage) {
    // Mapear voltaje a porcentaje (para LiPo: 3.0V = 0%, 4.2V = 100%)
    if (voltage >= 4.2) return 100;
    if (voltage <= 3.0) return 0;
    
    // Curva aproximada de descarga de LiPo
    return (int)((voltage - 3.0) / 1.2 * 100);
}

bool battery_isCharging() {
    // Detectar si está cargando comparando lecturas recientes
    static int lastReading = 0;
    int currentReading = lastBatteryTestValue;
    
    bool charging = (currentReading > lastReading + 2); // Umbral de 2%
    lastReading = currentReading;
    
    return charging;
}

BatteryInfo battery_getInfo() {
    return batteryInfo;
}

int battery_getPercentage() {
    return batteryInfo.isValid ? batteryInfo.percentage : 100;
}

float battery_getVoltage() {
    return batteryInfo.isValid ? batteryInfo.voltage : 4.2;
}

bool battery_isLow() {
    return batteryInfo.isValid && batteryInfo.isLow;
}

bool battery_isCritical() {
    return batteryInfo.isValid && batteryInfo.isCritical;
}

BatteryAlert battery_getAlertLevel() {
    return batteryInfo.isValid ? batteryInfo.alertLevel : BATTERY_OK;
}

void battery_updateDisplay() {
    if (!useBatteryTest || showBatteryTest == NONE) return;
    
    // La actualización del display se maneja en display_manager
    // Esta función puede usarse para forzar una actualización
    display_updateBattery();
}

String battery_getIconString() {
    if (!useBatteryTest || !batteryInfo.isValid) return "";
    
    int percentage = batteryInfo.percentage;
    String icon = "";
    
    // Crear icono de batería basado en el nivel
    if (batteryInfo.isCharging) {
        icon = "⚡"; // Icono de carga
    } else if (percentage > 75) {
        icon = "████"; // Batería llena
    } else if (percentage > 50) {
        icon = "███▒"; // Batería 3/4
    } else if (percentage > 25) {
        icon = "██▒▒"; // Batería 1/2
    } else if (percentage > 10) {
        icon = "█▒▒▒"; // Batería 1/4
    } else {
        icon = "▒▒▒▒"; // Batería vacía
    }
    
    return icon;
}

String battery_getPercentString() {
    if (!useBatteryTest || !batteryInfo.isValid) return "";
    
    return String(batteryInfo.percentage) + "%";
}

String battery_getStatusString() {
    if (!useBatteryTest) return "Battery monitoring disabled";
    if (!batteryInfo.isValid) return "Battery reading invalid";
    
    String status = String(batteryInfo.percentage) + "% ";
    status += "(" + String(batteryInfo.voltage, 2) + "V)";
    
    if (batteryInfo.isCharging) {
        status += " Charging";
    } else if (batteryInfo.isCritical) {
        status += " CRITICAL";
    } else if (batteryInfo.isLow) {
        status += " Low";
    }
    
    return status;
}

void battery_showAlert(BatteryAlert alert) {
    String message = "";
    
    switch (alert) {
        case BATTERY_LOW:
            message = "Low Battery: " + String(batteryInfo.percentage) + "%";
            break;
        case BATTERY_CRITICAL:
            message = "CRITICAL Battery: " + String(batteryInfo.percentage) + "%";
            break;
        case BATTERY_SLEEP:
            message = "Battery Empty - Sleeping";
            break;
        default:
            return;
    }
    
    display_showMessage(message, 3000);
}

void battery_setShowMode(ShowBattery mode) {
    showBatteryTest = mode;
    saveBatteryShowMode((int)mode);
    debug_print("Battery show mode set to: ");
    debug_println((int)mode);
}

ShowBattery battery_getShowMode() {
    return showBatteryTest;
}

void battery_setUsePercent(bool usePercent) {
    useBatteryPercentAsWellAsIcon = usePercent;
    
    if (usePercent && showBatteryTest == ICON_ONLY) {
        showBatteryTest = ICON_AND_PERCENT;
    } else if (!usePercent && showBatteryTest == ICON_AND_PERCENT) {
        showBatteryTest = ICON_ONLY;
    }
    
    battery_setShowMode(showBatteryTest);
}

bool battery_getUsePercent() {
    return useBatteryPercentAsWellAsIcon;
}

void battery_setSleepThreshold(int percentage) {
    sleepThreshold = constrain(percentage, -1, 50);
    // Guardar en preferencias si es necesario
    debug_print("Battery sleep threshold set to: ");
    debug_println(sleepThreshold);
}

int battery_getSleepThreshold() {
    return sleepThreshold;
}

void battery_calibrate() {
    debug_println("Starting battery calibration...");
    
    // Tomar múltiples muestras para calibración
    int samples[20];
    for (int i = 0; i < 20; i++) {
        samples[i] = analogRead(batteryPin);
        delay(100);
    }
    
    // Calcular promedio
    int average = 0;
    for (int i = 0; i < 20; i++) {
        average += samples[i];
    }
    average /= 20;
    
    // Calcular factor de conversión sugerido
    // Asumiendo que la batería está al 100% durante la calibración
    float suggestedFactor = (4.2 * 4095.0) / (average * 3.3);
    
    debug_print("Calibration complete. Suggested conversion factor: ");
    debug_println(suggestedFactor);
    
    conversionFactor = suggestedFactor;
    batteryCalibrated = true;
}

void battery_setConversionFactor(float factor) {
    conversionFactor = factor;
    #if USE_BATTERY_TEST
    BL.setConversionFactor(factor);
    #endif
    debug_print("Battery conversion factor set to: ");
    debug_println(factor);
}

float battery_getConversionFactor() {
    return conversionFactor;
}

void battery_resetCalibration() {
    conversionFactor = BATTERY_CONVERSION_FACTOR;
    batteryCalibrated = false;
    #if USE_BATTERY_TEST
    BL.setConversionFactor(conversionFactor);
    #endif
    debug_println("Battery calibration reset to default");
}

void battery_debugReading() {
    debug_print("BATTERY TestValue: ");
    debug_print(lastBatteryTestValue);
    debug_print(" (");
    debug_print(millis());
    debug_println(")");
    
    debug_print("BATTERY lastAnalogReadValue: ");
    debug_print(lastBatteryAnalogReadValue);
    debug_print(" (");
    debug_print(millis());
    debug_println(")");
    
    if (lastBatteryTestValue >= 98) {
        float suggestedFactor = (4.2 * 4095.0) / (lastBatteryAnalogReadValue * 3.3);
        debug_print("BATTERY If Battery full, BATTERY_CONVERSION_FACTOR should be: ");
        debug_print(suggestedFactor);
        debug_print(" (");
        debug_print(millis());
        debug_println(")");
    }
}

void battery_checkSleepCondition() {
    if (sleepThreshold >= 0 && batteryInfo.percentage <= sleepThreshold) {
        debug_println("Battery level critical - entering sleep mode");
        battery_showAlert(BATTERY_SLEEP);
        delay(5000);
        battery_enterSleepMode();
    }
}

void battery_enterSleepMode() {
    debug_println("Entering battery sleep mode");
    
    // Mostrar mensaje de sleep
    clearOledArray();
    setAppnameForOled();
    oledText[2] = "Battery Empty";
    oledText[3] = "Entering Sleep Mode";
    writeOledArray(false, false, true, true);
    
    delay(3000);
    
    // Entrar en deep sleep
    #ifdef ESP32
    esp_deep_sleep_start();
    #endif
}

void battery_handleLowBattery() {
    static unsigned long lastLowAlert = 0;
    
    if (millis() - lastLowAlert > 60000) { // Alerta cada minuto
        battery_showAlert(BATTERY_LOW);
        lastLowAlert = millis();
    }
}

void battery_handleCriticalBattery() {
    static unsigned long lastCriticalAlert = 0;
    
    if (millis() - lastCriticalAlert > 30000) { // Alerta cada 30 segundos
        battery_showAlert(BATTERY_CRITICAL);
        lastCriticalAlert = millis();
    }
}

void battery_addSample(int sample) {
    batterySamples[sampleIndex] = sample;
    sampleIndex = (sampleIndex + 1) % BATTERY_VOLTAGE_SAMPLES;
    
    if (sampleCount < BATTERY_VOLTAGE_SAMPLES) {
        sampleCount++;
    }
}

int battery_getAverageSample() {
    if (sampleCount == 0) return 0;
    
    int total = 0;
    for (int i = 0; i < sampleCount; i++) {
        total += batterySamples[i];
    }
    
    return total / sampleCount;
}

void battery_clearSamples() {
    sampleIndex = 0;
    sampleCount = 0;
    
    for (int i = 0; i < BATTERY_VOLTAGE_SAMPLES; i++) {
        batterySamples[i] = 0;
    }
}

bool battery_isSampleStable() {
    if (sampleCount < BATTERY_VOLTAGE_SAMPLES) return false;
    
    int average = battery_getAverageSample();
    int variance = 0;
    
    for (int i = 0; i < sampleCount; i++) {
        int diff = batterySamples[i] - average;
        variance += diff * diff;
    }
    
    variance /= sampleCount;
    
    return variance < 100; // Umbral de estabilidad
}

void battery_toggle() {
    switch (showBatteryTest) {
        case NONE:
            showBatteryTest = ICON_ONLY;
            break;
        case ICON_ONLY:
            showBatteryTest = ICON_AND_PERCENT;
            break;
        case ICON_AND_PERCENT:
            showBatteryTest = NONE;
            break;
    }
    
    battery_setShowMode(showBatteryTest);
    battery_updateDisplay();
}

void battery_showToggle() {
    battery_toggle();
    
    String message = "Battery Display: ";
    switch (showBatteryTest) {
        case NONE:
            message += "Off";
            break;
        case ICON_ONLY:
            message += "Icon Only";
            break;
        case ICON_AND_PERCENT:
            message += "Icon + Percent";
            break;
    }
    
    display_showMessage(message, 2000);
}

String battery_formatVoltage(float voltage) {
    return String(voltage, 2) + "V";
}

String battery_formatPercentage(int percentage) {
    return String(percentage) + "%";
}

bool battery_isEnabled() {
    return useBatteryTest;
}

void battery_runDiagnostic() {
    debug_println("=== BATTERY DIAGNOSTIC ===");
    debug_print("Enabled: ");
    debug_println(useBatteryTest ? "Yes" : "No");
    
    if (!useBatteryTest) {
        debug_println("=== END DIAGNOSTIC ===");
        return;
    }
    
    debug_print("Pin: ");
    debug_println(batteryPin);
    debug_print("Conversion Factor: ");
    debug_println(conversionFactor);
    debug_print("Show Mode: ");
    debug_println((int)showBatteryTest);
    debug_print("Percentage: ");
    debug_println(batteryInfo.percentage);
    debug_print("Voltage: ");
    debug_println(batteryInfo.voltage);
    debug_print("Raw Value: ");
    debug_println(lastBatteryAnalogReadValue);
    debug_print("Is Charging: ");
    debug_println(batteryInfo.isCharging ? "Yes" : "No");
    debug_print("Is Low: ");
    debug_println(batteryInfo.isLow ? "Yes" : "No");
    debug_print("Is Critical: ");
    debug_println(batteryInfo.isCritical ? "Yes" : "No");
    debug_print("Sleep Threshold: ");
    debug_println(sleepThreshold);
    debug_print("Update Interval: ");
    debug_println(batteryUpdateInterval);
    debug_print("Last Update: ");
    debug_println(batteryInfo.lastUpdate);
    debug_print("Sample Count: ");
    debug_println(sampleCount);
    debug_print("Average Sample: ");
    debug_println(battery_getAverageSample());
    debug_print("Sample Stable: ");
    debug_println(battery_isSampleStable() ? "Yes" : "No");
    debug_println("=== END DIAGNOSTIC ===");
}

void battery_exportDiagnostic() {
    battery_runDiagnostic();
    battery_exportToSerial();
}

void battery_logReading() {
    if (!useBatteryTest) return;
    
    debug_print("[BATTERY] ");
    debug_print(millis());
    debug_print(" ms: ");
    debug_print(batteryInfo.percentage);
    debug_print("% (");
    debug_print(batteryInfo.voltage);
    debug_print("V) Raw: ");
    debug_print(lastBatteryAnalogReadValue);
    debug_print(" Alert: ");
    debug_println((int)batteryInfo.alertLevel);
}

// Funciones de configuración adicionales
void battery_setPin(int pin) {
    batteryPin = pin;
    pinMode(pin, INPUT);
}

int battery_getPin() {
    return batteryPin;
}

void battery_setUpdateInterval(unsigned long interval) {
    batteryUpdateInterval = interval;
}

unsigned long battery_getUpdateInterval() {
    return batteryUpdateInterval;
}

void battery_setLowThreshold(int percentage) {
    lowThreshold = constrain(percentage, 5, 50);
}

int battery_getLowThreshold() {
    return lowThreshold;
}

void battery_setCriticalThreshold(int percentage) {
    criticalThreshold = constrain(percentage, 1, 25);
}

int battery_getCriticalThreshold() {
    return criticalThreshold;
}

// Función stub para getSleepThreshold si no está definida en preferences_manager
int getSleepThreshold() {
    return USE_BATTERY_SLEEP_AT_PERCENT;
}