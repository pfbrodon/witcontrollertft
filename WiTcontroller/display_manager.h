#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <U8g2lib.h>
#include <String.h>
#include "static.h"
#include "actions.h"
#include "config_keypad_etc.h"

// Constantes de display
#define OLED_LINES 18
#define OLED_CHARS_PER_LINE 21
#define FONT_DEFAULT u8g2_font_5x7_tr
#define FONT_THROTTLE u8g2_font_7x14_tr

// Estados de pantalla
enum DisplayScreen {
    SCREEN_STARTUP,
    SCREEN_WIFI_SCAN,
    SCREEN_WIFI_LIST,
    SCREEN_WIFI_PASSWORD,
    SCREEN_SERVER_SCAN,
    SCREEN_SERVER_LIST,
    SCREEN_THROTTLE,
    SCREEN_MENU,
    SCREEN_ROSTER,
    SCREEN_FUNCTIONS,
    SCREEN_TURNOUTS,
    SCREEN_ROUTES,
    SCREEN_ERROR
};

// Enumeración para acciones de turnout
enum TurnoutAction {
    TurnoutToggle,
    TurnoutThrow,
    TurnoutClose
};

// Variables globales de display
extern String oledText[];
extern bool oledTextInvert[];
extern DisplayScreen currentScreen;
extern int lastOledScreen;
extern String lastOledStringParameter;
extern int lastOledIntParameter;
extern bool lastOledBoolParameter;

// Funciones principales del módulo display
void display_init();
void display_loop();
void display_update();

// Funciones de gestión de pantalla
void clearOledArray();
void setAppnameForOled();
void setMenuTextForOled(String menuText);
void setMenuTextForOled(int menuTextIndex);

// Funciones de escritura específicas
void writeOledArray(bool isThreeColumns = false, bool isPassword = false, bool sendBuffer = true, bool drawTopLine = true);
void writeOledFoundSSids(String soFar);
void writeOledRoster(String soFar);
void writeOledTurnoutList(String soFar, TurnoutAction action);
void writeOledRouteList(String soFar);
void writeOledFunctionList(String soFar);
void writeOledEnterPassword();
void writeOledMenu(String soFar, bool primeMenu);
void writeOledAllLocos(bool hideLeadLoco);
void writeOledEditConsist();
void writeOledSpeed();
void writeOledSpeedStepMultiplier();
void writeOledBattery();
void writeOledFunctions();

// Funciones de display de alto nivel
void display_showStartup(const String& appName, const String& version);
void display_showMessage(const String& message, int duration = 0);
void display_showError(const String& error);
void display_showThrottle();
void display_showMenu();
void display_showRoster();
void display_showFunctions();
void display_showWifiScan();
void display_showWifiList();
void display_showServerScan();
void display_showServerList();

// Funciones de actualización específicas
void display_updateRoster();
void display_updateThrottle();
void display_updateBattery();
void display_updateFunctions();
void display_updateStatus(const String& status);

// Funciones de utilidad
void display_refresh();
void display_clear();
void display_reset();
String getDots(int numDots);
String formatSpeedForDisplay(int speed);
String formatDirectionForDisplay(int direction);

// Funciones de configuración
void display_setBrightness(int brightness);
void display_setContrast(int contrast);
void display_invertDisplay(bool invert);

#endif // DISPLAY_MANAGER_H