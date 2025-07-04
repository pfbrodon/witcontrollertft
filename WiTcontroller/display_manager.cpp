#include "display_manager.h"
#include "WiTcontroller.h"

// Variables globales de display
String oledText[OLED_LINES] = {"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""};
bool oledTextInvert[OLED_LINES] = {false, false, false, false, false, false, false, false, false, 
                                   false, false, false, false, false, false, false, false, false};

DisplayScreen currentScreen = SCREEN_STARTUP;
int lastOledScreen = 0;
String lastOledStringParameter = "";
int lastOledIntParameter = 0;
bool lastOledBoolParameter = false;
TurnoutAction lastOledTurnoutParameter = TurnoutToggle;

// Variables externas del archivo principal
extern OLED_TYPE;
extern String broadcastMessageText;
extern long broadcastMessageTime;
extern String menu_text[];
extern String directCommandText[][3];
extern bool oledDirectCommandsAreBeingDisplayed;

// Constantes de pantalla
const int last_oled_screen_speed = 1;
const int last_oled_screen_turnout_list = 2;
const int last_oled_screen_route_list = 3;
const int last_oled_screen_function_list = 4;
const int last_oled_screen_menu = 5;
const int last_oled_screen_extra_submenu = 6;
const int last_oled_screen_all_locos = 7;
const int last_oled_screen_edit_consist = 8;
const int last_oled_screen_direct_commands = 9;

void display_init() {
    debug_println("Initializing display...");
    
    u8g2.begin();
    u8g2.clearBuffer();
    u8g2.setFont(FONT_DEFAULT);
    
    clearOledArray();
    currentScreen = SCREEN_STARTUP;
    
    debug_println("Display initialized");
}

void display_loop() {
    // Actualizar display si es necesario
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 100) { // Actualizar cada 100ms
        display_update();
        lastUpdate = millis();
    }
}

void display_update() {
    // Lógica de actualización automática del display
    switch (currentScreen) {
        case SCREEN_THROTTLE:
            writeOledSpeed();
            break;
        case SCREEN_FUNCTIONS:
            writeOledFunctions();
            break;
        default:
            // No hacer nada para otras pantallas
            break;
    }
}

void clearOledArray() {
    for (int i = 0; i < OLED_LINES; i++) {
        oledText[i] = "";
        oledTextInvert[i] = false;
    }
}

void setAppnameForOled() {
    oledText[0] = appName + " " + appVersion;
}

void setMenuTextForOled(String menuText) {
    debug_print("setMenuTextForOled(): ");
    debug_println(menuText);
    oledText[5] = menuText;
    
    if (broadcastMessageText != "") {
        if (millis() - broadcastMessageTime < 10000) {
            oledText[5] = broadcastMessageText;
        } else {
            broadcastMessageText = "";
            broadcastMessageTime = 0;
        }
    }
}

void setMenuTextForOled(int menuTextIndex) {
    debug_print("setMenuTextForOled(): ");
    debug_println(menuTextIndex);
    oledText[5] = menu_text[menuTextIndex];
    
    if (broadcastMessageText != "") {
        if (millis() - broadcastMessageTime < 10000) {
            oledText[5] = broadcastMessageText;
        } else {
            broadcastMessageText = "";
            broadcastMessageTime = 0;
        }
    }
}

void writeOledArray(bool isThreeColumns, bool isPassword, bool sendBuffer, bool drawTopLine) {
    u8g2.clearBuffer();
    
    if (drawTopLine) {
        u8g2.drawLine(0, 8, 128, 8);
    }
    
    int lineHeight = 8;
    int startY = drawTopLine ? 16 : 8;
    
    if (isThreeColumns) {
        // Modo de tres columnas para menús
        for (int i = 0; i < 15; i++) {
            if (oledText[i] != "") {
                int x = 0;
                int y = startY + (i % 5) * lineHeight;
                
                if (i >= 5 && i < 10) x = 43;
                else if (i >= 10) x = 86;
                
                if (oledTextInvert[i]) {
                    u8g2.setDrawColor(1);
                    u8g2.drawBox(x, y - 7, u8g2.getStrWidth(oledText[i].c_str()), 8);
                    u8g2.setDrawColor(0);
                }
                
                u8g2.drawStr(x, y, oledText[i].c_str());
                
                if (oledTextInvert[i]) {
                    u8g2.setDrawColor(1);
                }
            }
        }
    } else {
        // Modo normal de una columna
        for (int i = 0; i < OLED_LINES; i++) {
            if (oledText[i] != "") {
                int y = startY + i * lineHeight;
                
                if (isPassword && i == 3) {
                    // Ocultar contraseña con asteriscos
                    String passwordMask = "";
                    for (unsigned int j = 0; j < oledText[i].length(); j++) {
                        passwordMask += "*";
                    }
                    u8g2.drawStr(0, y, passwordMask.c_str());
                } else {
                    if (oledTextInvert[i]) {
                        u8g2.setDrawColor(1);
                        u8g2.drawBox(0, y - 7, u8g2.getStrWidth(oledText[i].c_str()), 8);
                        u8g2.setDrawColor(0);
                    }
                    
                    u8g2.drawStr(0, y, oledText[i].c_str());
                    
                    if (oledTextInvert[i]) {
                        u8g2.setDrawColor(1);
                    }
                }
            }
        }
    }
    
    if (sendBuffer) {
        u8g2.sendBuffer();
    }
}

void writeOledFoundSSids(String soFar) {
    debug_println("writeOledFoundSSids()");
    clearOledArray();
    setAppnameForOled();
    
    if (foundSsidsCount == 0) {
        oledText[2] = MSG_NO_SSIDS_FOUND;
    } else {
        oledText[1] = MSG_SSIDS_FOUND + " (" + String(foundSsidsCount) + ")";
        
        for (int i = 0; i < min(foundSsidsCount, 10); i++) {
            String line = String(i) + ": ";
            if (foundSsids[i].length() < 15) {
                line += foundSsids[i];
            } else {
                line += foundSsids[i].substring(0, 12) + "...";
            }
            
            // Añadir indicador de seguridad
            if (foundSsidsOpen[i]) {
                line += " [OPEN]";
            }
            
            oledText[i + 2] = line;
        }
    }
    
    writeOledArray(false, false);
}

void writeOledSpeed() {
    debug_println("writeOledSpeed()");
    lastOledScreen = last_oled_screen_speed;
    
    clearOledArray();
    setAppnameForOled();
    
    // Mostrar información del throttle actual
    oledText[1] = "Throttle " + String(currentThrottle + 1);
    oledText[2] = "Speed: " + formatSpeedForDisplay(currentSpeed[currentThrottle]);
    oledText[3] = "Dir: " + formatDirectionForDisplay(currentDirection[currentThrottle]);
    
    // Mostrar locomotoras activas
    if (currentLocoAddress[currentThrottle] != 0) {
        oledText[4] = "Loco: " + String(currentLocoAddress[currentThrottle]);
    } else {
        oledText[4] = "No loco selected";
    }
    
    writeOledBattery();
    writeOledArray(false, false);
    currentScreen = SCREEN_THROTTLE;
}

void writeOledFunctions() {
    debug_println("writeOledFunctions()");
    
    clearOledArray();
    setAppnameForOled();
    
    oledText[1] = "Functions";
    
    // Mostrar estado de funciones F0-F9
    String funcLine1 = "F0-4: ";
    String funcLine2 = "F5-9: ";
    
    for (int i = 0; i <= 4; i++) {
        funcLine1 += functionStates[currentThrottle][i] ? String(i) : ".";
        funcLine1 += " ";
    }
    
    for (int i = 5; i <= 9; i++) {
        funcLine2 += functionStates[currentThrottle][i] ? String(i) : ".";
        funcLine2 += " ";
    }
    
    oledText[2] = funcLine1;
    oledText[3] = funcLine2;
    
    writeOledArray(false, false);
    currentScreen = SCREEN_FUNCTIONS;
}

void writeOledBattery() {
    #if USE_BATTERY_TEST
    if (showBatteryTest != NONE) {
        String batteryStr = "";
        
        if (showBatteryTest == ICON_ONLY || showBatteryTest == ICON_AND_PERCENT) {
            // Mostrar icono de batería basado en el nivel
            if (lastBatteryTestValue > 75) batteryStr += "████";
            else if (lastBatteryTestValue > 50) batteryStr += "███▒";
            else if (lastBatteryTestValue > 25) batteryStr += "██▒▒";
            else if (lastBatteryTestValue > 10) batteryStr += "█▒▒▒";
            else batteryStr += "▒▒▒▒";
        }
        
        if (showBatteryTest == ICON_AND_PERCENT) {
            batteryStr += " " + String(lastBatteryTestValue) + "%";
        }
        
        oledText[17] = batteryStr; // Última línea
    }
    #endif
}

void display_showStartup(const String& appName, const String& version) {
    clearOledArray();
    oledText[4] = appName;
    oledText[5] = version;
    oledText[7] = "Iniciando...";
    writeOledArray(false, false);
    currentScreen = SCREEN_STARTUP;
}

void display_showMessage(const String& message, int duration) {
    clearOledArray();
    setAppnameForOled();
    oledText[3] = message;
    writeOledArray(false, false);
    
    if (duration > 0) {
        delay(duration);
        display_refresh();
    }
}

void display_showError(const String& error) {
    clearOledArray();
    oledText[2] = "ERROR:";
    oledText[3] = error;
    writeOledArray(false, false);
    currentScreen = SCREEN_ERROR;
}

void display_refresh() {
    debug_print("display_refresh(): ");
    debug_println(lastOledScreen);
    
    switch (lastOledScreen) {
        case last_oled_screen_speed:
            writeOledSpeed();
            break;
        case last_oled_screen_turnout_list:
            writeOledTurnoutList(lastOledStringParameter, lastOledTurnoutParameter);
            break;
        case last_oled_screen_route_list:
            writeOledRouteList(lastOledStringParameter);
            break;
        case last_oled_screen_function_list:
            writeOledFunctionList(lastOledStringParameter);
            break;
        case last_oled_screen_menu:
            writeOledMenu(lastOledStringParameter, true);
            break;
        case last_oled_screen_extra_submenu:
            writeOledMenu(lastOledStringParameter, false);
            break;
        case last_oled_screen_all_locos:
            writeOledAllLocos(lastOledBoolParameter);
            break;
        case last_oled_screen_edit_consist:
            writeOledEditConsist();
            break;
        case last_oled_screen_direct_commands:
            writeOledDirectCommands();
            break;
        default:
            writeOledSpeed();
            break;
    }
}

void display_clear() {
    u8g2.clearBuffer();
    u8g2.sendBuffer();
    clearOledArray();
}

void display_reset() {
    display_clear();
    currentScreen = SCREEN_STARTUP;
    lastOledScreen = 0;
}

String getDots(int numDots) {
    String dots = "";
    for (int i = 0; i < numDots % 4; i++) {
        dots += ".";
    }
    return dots;
}

String formatSpeedForDisplay(int speed) {
    if (speed == 0) return "STOP";
    return String(speed) + "/126";
}

String formatDirectionForDisplay(int direction) {
    if (direction == 0) return "REV";
    else if (direction == 1) return "FWD";
    else return "UNK";
}

void display_setBrightness(int brightness) {
    u8g2.setContrast(brightness);
}

void display_setContrast(int contrast) {
    u8g2.setContrast(contrast);
}

void display_invertDisplay(bool invert) {
    u8g2.setFlipMode(invert ? 1 : 0);
}

// Funciones stub para funciones más complejas que requieren más context del archivo principal
void writeOledRoster(String soFar) {
    // Implementación simplificada - en el archivo real sería más compleja
    clearOledArray();
    setAppnameForOled();
    oledText[1] = "Roster";
    oledText[2] = soFar;
    writeOledArray(false, false);
}

void writeOledTurnoutList(String soFar, TurnoutAction action) {
    clearOledArray();
    setAppnameForOled();
    oledText[1] = "Turnouts";
    oledText[2] = soFar;
    writeOledArray(false, false);
}

void writeOledRouteList(String soFar) {
    clearOledArray();
    setAppnameForOled();
    oledText[1] = "Routes";
    oledText[2] = soFar;
    writeOledArray(false, false);
}

void writeOledFunctionList(String soFar) {
    clearOledArray();
    setAppnameForOled();
    oledText[1] = "Function List";
    oledText[2] = soFar;
    writeOledArray(false, false);
}

void writeOledEnterPassword() {
    clearOledArray();
    setAppnameForOled();
    oledText[1] = "Enter Password:";
    oledText[2] = ssidPasswordEntered;
    writeOledArray(false, true); // Password mode
}

void writeOledMenu(String soFar, bool primeMenu) {
    clearOledArray();
    setAppnameForOled();
    oledText[1] = primeMenu ? "Main Menu" : "Sub Menu";
    oledText[2] = soFar;
    writeOledArray(false, false);
}

void writeOledAllLocos(bool hideLeadLoco) {
    clearOledArray();
    setAppnameForOled();
    oledText[1] = "All Locos";
    writeOledArray(false, false);
}

void writeOledEditConsist() {
    clearOledArray();
    setAppnameForOled();
    oledText[1] = "Edit Consist";
    writeOledArray(false, false);
}

void writeOledSpeedStepMultiplier() {
    clearOledArray();
    setAppnameForOled();
    oledText[1] = "Speed Multiplier";
    oledText[2] = String(speedStepCurrentMultiplier) + "x";
    writeOledArray(false, false);
}

void writeOledDirectCommands() {
    lastOledScreen = last_oled_screen_direct_commands;
    
    oledDirectCommandsAreBeingDisplayed = true;
    clearOledArray();
    oledText[0] = "Direct Commands";
    
    for (int i = 0; i < 4; i++) {
        oledText[i + 1] = directCommandText[i][0];
    }
    
    int j = 0;
    for (int i = 6; i < 10; i++) {
        oledText[i + 1] = directCommandText[j][1];
        j++;
    }
    
    j = 0;
    for (int i = 12; i < 16; i++) {
        oledText[i + 1] = directCommandText[j][2];
        j++;
    }
    
    writeOledArray(true, false);
}