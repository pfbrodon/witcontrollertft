#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "display_interface.h"
#include <Arduino.h>

// =====================================================
// Manager de alto nivel para funciones de pantalla del WiTcontroller
// =====================================================

// Enums para diferentes tipos de contenido
enum DisplayContentType {
    CONTENT_THROTTLE_SPEED,
    CONTENT_MENU,
    CONTENT_ROSTER,
    CONTENT_TURNOUT_LIST,
    CONTENT_ROUTE_LIST,
    CONTENT_FUNCTION_LIST,
    CONTENT_SSID_LIST,
    CONTENT_PASSWORD_ENTRY,
    CONTENT_DIRECT_COMMANDS,
    CONTENT_EDIT_CONSIST
};

// Forward declarations
enum TurnoutAction;
extern const int TurnoutThrow;
extern const int TurnoutClose;

class DisplayManager {
private:
    DisplayInterface* display;
    String oledText[18];
    bool oledTextInvert[18];
    
    // Variables de estado para elementos de pantalla
    bool showBattery;
    bool showSpeedMultiplier;
    int lastBatteryValue;
    int currentThrottleIndex;
    
    // Funciones auxiliares privadas
    void clearTextArray();
    void drawTextArray(bool isThreeColumns, bool isPassword, bool sendBuffer, bool drawTopLine);
    void drawBatteryIndicator();
    void drawSpeedMultiplierIndicator();
    void drawFunctionIndicators();
    void drawThrottleHeader();
    void drawHorizontalLines();
    
public:
    DisplayManager(DisplayInterface* displayInterface);
    ~DisplayManager();
    
    // Funciones principales de la interfaz
    void begin();
    void clear();
    void refresh();
    void powerSave(bool enable);
    
    // Funciones de alto nivel para diferentes pantallas
    void showThrottleScreen();
    void showMenu(const String& soFar, bool primeMenu);
    void showRosterList(const String& soFar);
    void showTurnoutList(const String& soFar, TurnoutAction action);
    void showRouteList(const String& soFar);
    void showFunctionList(const String& soFar);
    void showSsidList(const String& soFar);
    void showPasswordEntry();
    void showDirectCommands();
    void showEditConsist();
    void showAllLocos(bool hideLeadLoco);
    
    // Funciones de configuración
    void setBatteryDisplay(bool show, int batteryValue = 0);
    void setSpeedMultiplierDisplay(bool show, int multiplier = 1);
    void setCurrentThrottle(int throttleIndex);
    
    // Funciones para mostrar texto genérico
    void showTextList(const String lines[], int lineCount, const String& soFar, bool isThreeColumns = false);
    void showMessage(const String& message, bool sendBuffer = true);
    
    // Funciones de utilidad
    void setTextLine(int line, const String& text, bool invert = false);
    String getTextLine(int line);
    void clearTextLine(int line);
    
    // Acceso directo a la pantalla (para casos especiales)
    DisplayInterface* getDisplay() { return display; }
};

#endif // DISPLAY_MANAGER_H