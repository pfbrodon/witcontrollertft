/**
 * WiTcontroller - Versión Refactorizada (Ejemplo)
 * 
 * Este archivo muestra cómo quedaría el WiTcontroller.ino principal
 * después de la refactorización en módulos separados.
 * 
 * Tamaño reducido: ~200 líneas vs 3778 líneas originales
 */

#include <string>

// Librerías principales
#include <WiFi.h>
#include <ESPmDNS.h>
#include <esp_wifi.h>
#include <Preferences.h>
#include <Keypad.h>
#include <U8g2lib.h>
#include <WiThrottleProtocol.h>
#include <AiEsp32RotaryEncoder.h>

// Archivos de configuración del usuario
#include "config_network.h"
#include "config_buttons.h"

// Archivos de configuración del sistema
#include "config_keypad_etc.h"
#include "static.h"
#include "actions.h"
#include "WiTcontroller.h"

// NUEVOS MÓDULOS REFACTORIZADOS
#include "wifi_manager.h"
#include "wit_server.h"
#include "display_manager.h"
#include "input_handler.h"
#include "roster_manager.h"
#include "preferences_manager.h"
#include "battery_monitor.h"

// Macros de debug
#if WITCONTROLLER_DEBUG == 0
 #define debug_print(params...) Serial.print(params)
 #define debug_println(params...) Serial.print(params); Serial.print(" ("); Serial.print(millis()); Serial.println(")")
 #define debug_printf(params...) Serial.printf(params)
#else
 #define debug_print(...)
 #define debug_println(...)
 #define debug_printf(...)
#endif

// Variables globales principales
int debugLevel = DEBUG_LEVEL;
int currentThrottle = 0;
bool systemInitialized = false;

// Objetos de hardware
OLED_TYPE;
Keypad customKeypad = Keypad(makeKeymap(keypadKeys), keypadRowPins, keypadColPins, ROW_NUM, COLUMN_NUM);
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);
WiThrottleProtocol wiThrottleProtocol;

// ================================================================================
// SETUP - Inicialización del sistema
// ================================================================================
void setup() {
    Serial.begin(115200);
    debug_println("WiTcontroller Starting...");
    
    // Inicializar hardware básico
    initializeHardware();
    
    // Inicializar módulos
    initializeModules();
    
    // Mostrar pantalla de inicio
    showStartupScreen();
    
    systemInitialized = true;
    debug_println("WiTcontroller Ready!");
}

// ================================================================================
// LOOP PRINCIPAL - Bucle principal del sistema
// ================================================================================
void loop() {
    if (!systemInitialized) return;
    
    // Procesar entrada de usuario
    input_handler_loop();
    
    // Gestionar conexión WiFi
    wifi_manager_loop();
    
    // Gestionar conexión con servidor WiThrottle
    wit_server_loop();
    
    // Actualizar display
    display_manager_loop();
    
    // Monitorear batería
    battery_monitor_loop();
    
    // Procesar comandos WiThrottle
    processWiThrottleProtocol();
    
    // Pequeña pausa para evitar saturar el procesador
    delay(10);
}

// ================================================================================
// FUNCIONES DE INICIALIZACIÓN
// ================================================================================
void initializeHardware() {
    debug_println("Initializing hardware...");
    
    // Inicializar display OLED
    u8g2.begin();
    u8g2.clearBuffer();
    u8g2.setFont(FONT_DEFAULT);
    
    // Inicializar encoder rotativo
    rotaryEncoder.begin();
    rotaryEncoder.setup(readEncoderISR);
    
    // Inicializar teclado
    customKeypad.addEventListener(keypadEvent);
    customKeypad.setHoldTime(KEY_HOLD_TIME);
    
    // Configurar pines adicionales
    setupAdditionalButtons();
    
    debug_println("Hardware initialized");
}

void initializeModules() {
    debug_println("Initializing modules...");
    
    // Inicializar gestión de preferencias
    preferences_init();
    
    // Inicializar gestión WiFi
    wifi_init();
    
    // Inicializar gestión de servidor WiThrottle
    wit_server_init();
    
    // Inicializar gestión de display
    display_init();
    
    // Inicializar gestión de entrada
    input_handler_init();
    
    // Inicializar gestión de roster
    roster_init();
    
    // Inicializar monitoreo de batería
    battery_monitor_init();
    
    debug_println("Modules initialized");
}

void showStartupScreen() {
    display_showStartup(appName, appVersion);
    delay(2000);
}

// ================================================================================
// FUNCIONES DE PROTOCOLO WITHROTTLE
// ================================================================================
void processWiThrottleProtocol() {
    if (!wit_server_isConnected()) return;
    
    // Procesar mensajes entrantes del servidor
    wiThrottleProtocol.check();
    
    // Enviar comandos pendientes
    wit_server_processPendingCommands();
    
    // Actualizar estado de locomotoras
    roster_updateStatus();
}

// ================================================================================
// CALLBACKS DE EVENTOS
// ================================================================================
void keypadEvent(KeypadEvent key) {
    input_handler_processKeypad(key, customKeypad.getState());
}

void readEncoderISR() {
    rotaryEncoder.readEncoder_ISR();
}

void onWiThrottleConnected() {
    debug_println("WiThrottle connected callback");
    display_showMessage("Conectado al servidor", 2000);
    roster_requestRosterList();
}

void onWiThrottleDisconnected() {
    debug_println("WiThrottle disconnected callback");
    display_showMessage("Desconectado del servidor", 2000);
    wit_server_reset();
}

void onRosterReceived() {
    debug_println("Roster received callback");
    roster_processReceivedData();
    display_updateRoster();
}

// ================================================================================
// FUNCIONES DE UTILIDAD
// ================================================================================
void setupAdditionalButtons() {
    #if defined(USE_ADDITIONAL_BUTTONS) && USE_ADDITIONAL_BUTTONS
    for (int i = 0; i < MAX_ADDITIONAL_BUTTONS; i++) {
        if (additionalButtonPins[i] != -1) {
            pinMode(additionalButtonPins[i], additionalButtonTypes[i]);
        }
    }
    #endif
}

void handleSystemError(const String& errorMessage) {
    debug_println("System Error: " + errorMessage);
    display_showError(errorMessage);
    
    // Reiniciar sistema si es necesario
    if (errorMessage.indexOf("critical") != -1) {
        ESP.restart();
    }
}

void performSystemReset() {
    debug_println("Performing system reset...");
    
    // Resetear todos los módulos
    wifi_reset();
    wit_server_reset();
    display_reset();
    input_handler_reset();
    roster_reset();
    
    // Limpiar preferencias si es necesario
    #if defined(CLEAR_PREFERENCES_ON_RESET) && CLEAR_PREFERENCES_ON_RESET
    preferences_clear();
    #endif
    
    // Reiniciar ESP32
    ESP.restart();
}

// ================================================================================
// FUNCIONES DE INFORMACIÓN DEL SISTEMA
// ================================================================================
void printSystemInfo() {
    debug_println("=== WiTcontroller System Info ===");
    debug_println("Version: " + String(appVersion));
    debug_println("Free Heap: " + String(ESP.getFreeHeap()));
    debug_println("Chip ID: " + String((uint32_t)ESP.getEfuseMac()));
    debug_println("CPU Freq: " + String(ESP.getCpuFreqMHz()) + " MHz");
    debug_println("Flash Size: " + String(ESP.getFlashChipSize()));
    
    if (WiFi.status() == WL_CONNECTED) {
        debug_println("WiFi SSID: " + WiFi.SSID());
        debug_println("WiFi IP: " + WiFi.localIP().toString());
        debug_println("WiFi RSSI: " + String(WiFi.RSSI()));
    }
    
    debug_println("=== End System Info ===");
}