#include "display_manager.h"
#include "config_buttons.h"  // Para las constantes de fuente y configuración

// Variables externas que necesitamos (declaradas en el .ino principal)
extern String oledText[18];
extern bool oledTextInvert[18];
extern int currentThrottleIndex;
extern int lastBatteryTestValue;
extern int speedStepCurrentMultiplier;

// Fuentes y constantes (definidas en config_buttons.h)
extern const uint8_t* FONT_DEFAULT;
extern const uint8_t* FONT_PASSWORD;
extern const uint8_t* FONT_THROTTLE_NUMBER;
extern const uint8_t* FONT_SPEED;
extern const uint8_t* FONT_DIRECTION;
extern const uint8_t* FONT_NEXT_THROTTLE;
extern const uint8_t* FONT_FUNCTION_INDICATORS;
extern const uint8_t* FONT_GLYPHS;

// Glifos y textos (definidos en static.h)
extern uint16_t glyph_track_power;
extern uint16_t glyph_heartbeat_off;
extern uint16_t glyph_speed_step;

DisplayManager::DisplayManager(DisplayInterface* displayInterface) 
    : display(displayInterface), showBattery(false), showSpeedMultiplier(false), 
      lastBatteryValue(0), currentThrottleIndex(0) {
    clearTextArray();
}

DisplayManager::~DisplayManager() {
    // No eliminamos display ya que se maneja externamente
}

void DisplayManager::begin() {
    if (display) {
        display->begin();
    }
}

void DisplayManager::clear() {
    if (display) {
        display->clear();
    }
}

void DisplayManager::refresh() {
    if (display) {
        display->display();
    }
}

void DisplayManager::powerSave(bool enable) {
    if (display) {
        display->powerSave(enable);
    }
}

void DisplayManager::clearTextArray() {
    for (int i = 0; i < 18; i++) {
        oledText[i] = "";
        oledTextInvert[i] = false;
    }
}

void DisplayManager::showThrottleScreen() {
    if (!display) return;
    
    // Variables externas que necesitamos
    extern int lastOledScreen;
    extern int last_oled_screen_speed;
    extern bool menuIsShowing;
    extern WiThrottleProtocol wiThrottleProtocol;
    extern char currentThrottleIndexChar;
    extern int currentThrottleIndex;
    extern int maxThrottles;
    extern String getDisplayLocoString(int throttleIndex, int position);
    extern int getDisplaySpeed(int throttleIndex);
    extern Direction currentDirection[];
    extern String DIRECTION_FORWARD_TEXT;
    extern String DIRECTION_REVERSE_TEXT;
    extern String CONSIST_SPACE_BETWEEN_LOCOS;
    extern char getMultiThrottleChar(int throttleIndex);
    
    lastOledScreen = last_oled_screen_speed;
    menuIsShowing = false;
    
    String sLocos = "";
    String sSpeed = "";
    String sDirection = "";
    String sSpaceBetweenLocos = " ";
    
    bool foundNextThrottle = false;
    String sNextThrottleNo = "";
    String sNextThrottleSpeedAndDirection = "";
    
    clearTextArray();
    
    bool drawTopLine = false;
    
    if (wiThrottleProtocol.getNumberOfLocomotives(currentThrottleIndexChar) > 0 ) {
        // Construir lista de locos
        for (int i=0; i < wiThrottleProtocol.getNumberOfLocomotives(currentThrottleIndexChar); i++) {
            sLocos = sLocos + sSpaceBetweenLocos + getDisplayLocoString(currentThrottleIndex, i);
            sSpaceBetweenLocos = CONSIST_SPACE_BETWEEN_LOCOS;
        }
        sSpeed = String(getDisplaySpeed(currentThrottleIndex));
        sDirection = (currentDirection[currentThrottleIndex]==Forward) ? DIRECTION_FORWARD_TEXT : DIRECTION_REVERSE_TEXT;
        
        // Buscar próximo throttle con locos
        if (maxThrottles > 1) {
            int nextThrottleIndex = currentThrottleIndex + 1;
            
            for (int i = nextThrottleIndex; i<maxThrottles; i++) {
                if (wiThrottleProtocol.getNumberOfLocomotives(getMultiThrottleChar(i)) > 0 ) {
                    foundNextThrottle = true;
                    nextThrottleIndex = i;
                    break;
                }
            }
            if ( (!foundNextThrottle) && (currentThrottleIndex>0) ) {
                for (int i = 0; i<currentThrottleIndex; i++) {
                    if (wiThrottleProtocol.getNumberOfLocomotives(getMultiThrottleChar(i)) > 0 ) {
                        foundNextThrottle = true;
                        nextThrottleIndex = i;
                        break;
                    }
                }
            }
            
            if (foundNextThrottle) {
                sNextThrottleNo = String(nextThrottleIndex+1);
                String sNextDirection = (currentDirection[nextThrottleIndex]==Forward) ? ">" : "<";
                sNextThrottleSpeedAndDirection = sNextDirection + String(getDisplaySpeed(nextThrottleIndex));
            }
        }
        
        drawTopLine = true;
    }
    
    // Usar implementación personalizada de dibujado
    display->clear();
    drawThrottleHeader();
    
    if (foundNextThrottle) {
        display->setFont(FONT_NEXT_THROTTLE);
        display->drawString(85+34, 38, sNextThrottleNo);
        display->drawString(85+12, 48, sNextThrottleSpeedAndDirection);
    }
    
    // Dibujar velocidad y dirección 
    display->setFont(FONT_SPEED);
    int width = display->getStringWidth(sSpeed);
    display->drawString(22+(55-width), 45, sSpeed);
    
    display->setFont(FONT_DIRECTION);
    display->drawString(79, 36, sDirection);
    
    drawFunctionIndicators();
    
    if (showBattery) {
        drawBatteryIndicator();
    }
    
    if (showSpeedMultiplier) {
        drawSpeedMultiplierIndicator();
    }
    
    refresh();
}

void DisplayManager::showMenu(const String& soFar, bool primeMenu) {
    if (!display) return;
    
    // Variables externas que necesitamos
    extern String lastOledStringParameter;
    extern int lastOledScreen;
    extern int last_oled_screen_menu;
    extern int last_oled_screen_extra_submenu;
    extern bool menuIsShowing;
    extern String menuCommand;
    extern String menuText[20][2];  // Array de textos del menú
    extern String menu_cancel;
    extern char currentThrottleIndexChar;
    extern int currentThrottleIndex;
    extern String MSG_THROTTLE_NUMBER;
    extern String MSG_NO_LOCO_SELECTED;
    extern char MENU_ITEM_DROP_LOCO;
    extern char MENU_ITEM_FUNCTION;
    extern char MENU_ITEM_TOGGLE_DIRECTION;
    extern WiThrottleProtocol wiThrottleProtocol;
    extern void setMenuTextForOled(String text);
    
    debug_print("DisplayManager::showMenu() : "); debug_print(primeMenu); debug_print(" : "); debug_println(soFar);
    lastOledStringParameter = soFar;
    
    int offset = 0;
    lastOledScreen = last_oled_screen_menu;
    if (!primeMenu) {  //Extra Menu
        offset = 10;
        lastOledScreen = last_oled_screen_extra_submenu;
    }
    
    menuIsShowing = true;
    bool drawTopLine = false;
    
    if ((soFar == "") || ((!primeMenu) && (soFar.length()==1))) { 
        // Menú principal - mostrar opciones
        clearTextArray();
        int j = 0;
        for (int i=1+offset; i<10+offset; i++) {
            j = (i<6+offset) ? i-offset : i+1-offset;
            oledText[j-1] = String(i-offset) + ": " + menuText[i][0];
        }
        oledText[10] = "0: " + menuText[0+offset][0];
        setMenuTextForOled(menu_cancel);
        drawTextArray(false, false, true, false);
    } else {
        // Submenu - mostrar detalles del comando
        int cmd = menuCommand.substring(0, 1).toInt();
        
        clearTextArray();
        
        oledText[0] = ">> " + menuText[cmd][0] +":"; 
        oledText[6] = menuCommand.substring(1, menuCommand.length());
        oledText[5] = menuText[cmd+offset][1];
        
        switch (soFar.charAt(0)) {
            case MENU_ITEM_DROP_LOCO: {
                if (wiThrottleProtocol.getNumberOfLocomotives(currentThrottleIndexChar) > 0) {
                    showAllLocos(false);
                    drawTopLine = true;
                }
            } // fall through
            
            case MENU_ITEM_FUNCTION:
            case MENU_ITEM_TOGGLE_DIRECTION: {
                if (wiThrottleProtocol.getNumberOfLocomotives(currentThrottleIndexChar) <= 0 ) {
                    oledText[2] = MSG_THROTTLE_NUMBER + String(currentThrottleIndex+1);
                    oledText[3] = MSG_NO_LOCO_SELECTED;
                    setMenuTextForOled(menu_cancel);
                } 
                break;
            }
        }
        
        drawTextArray(false, false, true, drawTopLine);
    }
}

void DisplayManager::showTextList(const String lines[], int lineCount, const String& soFar, bool isThreeColumns) {
    if (!display) return;
    
    clearTextArray();
    display->clear();
    
    for (int i = 0; i < lineCount && i < 18; i++) {
        oledText[i] = lines[i];
    }
    
    drawTextArray(isThreeColumns, false, true, false);
}

void DisplayManager::showMessage(const String& message, bool sendBuffer) {
    if (!display) return;
    
    display->clear();
    display->setFont(FONT_DEFAULT);
    display->drawString(0, 10, message);
    
    if (sendBuffer) {
        refresh();
    }
}

void DisplayManager::drawTextArray(bool isThreeColumns, bool isPassword, bool sendBuffer, bool drawTopLine) {
    if (!display) return;
    
    display->setFont(FONT_DEFAULT);
    
    int x = 0;
    int y = 10;
    int xInc = 64;
    int maxLines = 12;
    
    if (isThreeColumns) {
        xInc = 42;
        maxLines = 18;
    }
    
    for (int i = 0; i < maxLines; i++) {
        if (isPassword && i == 2) {
            display->setFont(FONT_PASSWORD);
        }
        
        if (oledTextInvert[i]) {
            display->setDrawColor(1);
            display->drawRectangle(x, y-8, 62, 10, true);
            display->setDrawColor(0);
        }
        
        display->drawString(x, y, oledText[i]);
        display->setDrawColor(1);
        
        if (isPassword && i == 2) {
            display->setFont(FONT_DEFAULT);
        }
        
        y += 10;
        if (i == 5 || i == 11) {
            x += xInc;
            y = 10;
        }
    }
    
    if (drawTopLine) {
        display->drawLine(0, 11, 128, 11);
        if (showBattery) {
            drawBatteryIndicator();
        }
    }
    
    display->drawLine(0, 51, 128, 51);
    
    if (sendBuffer) {
        refresh();
    }
}

void DisplayManager::drawThrottleHeader() {
    if (!display) return;
    
    // Dibujar número de throttle actual
    display->setDrawColor(0);
    display->drawRectangle(0, 0, 12, 16, true);
    display->setDrawColor(1);
    display->setFont(FONT_THROTTLE_NUMBER);
    display->drawString(2, 15, String(currentThrottleIndex + 1));
    
    // Dibujar indicador de potencia de vía
    display->drawRoundRectangle(0, 40, 9, 9, 1, true);
    display->setDrawColor(0);
    display->setFont(FONT_GLYPHS);
    display->drawGlyph(1, 48, glyph_track_power);
    display->setDrawColor(1);
}

void DisplayManager::drawBatteryIndicator() {
    if (!display) return;
    
    int x = 90; // Posición de la batería
    int y = 38;
    
    display->setFont(FONT_GLYPHS);
    display->setDrawColor(1);
    
    // Dibujar símbolo de batería básico
    display->drawString(x, y, "Z");
    
    // Dibujar barras de nivel
    if (lastBatteryValue > 10) display->drawLine(x+1, y-6, x+1, y-3);
    if (lastBatteryValue > 25) display->drawLine(x+2, y-6, x+2, y-3);
    if (lastBatteryValue > 50) display->drawLine(x+3, y-6, x+3, y-3);
    if (lastBatteryValue > 75) display->drawLine(x+4, y-6, x+4, y-3);
    if (lastBatteryValue > 90) display->drawLine(x+5, y-6, x+5, y-3);
}

void DisplayManager::drawSpeedMultiplierIndicator() {
    if (!display) return;
    
    display->setDrawColor(1);
    display->setFont(FONT_GLYPHS);
    display->drawGlyph(1, 38, glyph_speed_step);
    display->setFont(FONT_DEFAULT);
    display->drawString(9, 37, String(speedStepCurrentMultiplier));
}

void DisplayManager::drawFunctionIndicators() {
    // Esta función se implementará según la lógica original de writeOledFunctions()
    // Por ahora, implementación básica
}

void DisplayManager::setBatteryDisplay(bool show, int batteryValue) {
    showBattery = show;
    if (batteryValue > 0) {
        lastBatteryValue = batteryValue;
    }
}

void DisplayManager::setSpeedMultiplierDisplay(bool show, int multiplier) {
    showSpeedMultiplier = show;
    if (multiplier > 0) {
        speedStepCurrentMultiplier = multiplier;
    }
}

void DisplayManager::setCurrentThrottle(int throttleIndex) {
    currentThrottleIndex = throttleIndex;
}

void DisplayManager::setTextLine(int line, const String& text, bool invert) {
    if (line >= 0 && line < 18) {
        oledText[line] = text;
        oledTextInvert[line] = invert;
    }
}

String DisplayManager::getTextLine(int line) {
    if (line >= 0 && line < 18) {
        return oledText[line];
    }
    return "";
}

void DisplayManager::clearTextLine(int line) {
    if (line >= 0 && line < 18) {
        oledText[line] = "";
        oledTextInvert[line] = false;
    }
}

// Implementaciones de las otras funciones de pantalla
void DisplayManager::showRosterList(const String& soFar) {
    // Variables externas que necesitamos
    extern int lastOledScreen;
    extern int last_oled_screen_roster;
    extern String lastOledStringParameter;
    extern bool menuIsShowing;
    extern int keypadUseType;
    extern int KEYPAD_USE_SELECT_ROSTER;
    extern int page;
    extern int rosterSize;
    extern int rosterSortedIndex[];
    extern int rosterAddress[];
    extern String rosterName[];
    extern String menu_text[];
    extern int menu_roster;
    
    lastOledScreen = last_oled_screen_roster;
    lastOledStringParameter = soFar;
    
    menuIsShowing = true;
    keypadUseType = KEYPAD_USE_SELECT_ROSTER;
    
    if (soFar == "") { // nothing entered yet
        clearTextArray();
        for (int i=0; i<5 && ((page*5)+i<rosterSize); i++) {
            int index = rosterSortedIndex[(page*5)+i];
            if (rosterAddress[index] != 0) {
                oledText[i] = String(i) + ": " + rosterName[index] + " (" + rosterAddress[index] + ")" ;
            }
        }
        oledText[5] = "(" + String(page+1) +  ") " + menu_text[menu_roster];
        drawTextArray(false, false, true, false);
    }
}

void DisplayManager::showTurnoutList(const String& soFar, TurnoutAction action) {
    // Variables externas que necesitamos
    extern int lastOledScreen;
    extern int last_oled_screen_turnout_list;
    extern String lastOledStringParameter;
    extern TurnoutAction lastOledTurnoutParameter;
    extern bool menuIsShowing;
    extern int keypadUseType;
    extern int KEYPAD_USE_SELECT_TURNOUTS_THROW;
    extern int KEYPAD_USE_SELECT_TURNOUTS_CLOSE;
    extern int page;
    extern int turnoutListSize;
    extern int turnoutListIndex[];
    extern String turnoutListUserName[];
    extern String menu_text[];
    extern int menu_turnout_list;
    extern int TurnoutThrow;
    extern int TurnoutClose;
    
    lastOledScreen = last_oled_screen_turnout_list;
    lastOledStringParameter = soFar;
    lastOledTurnoutParameter = action;
    
    menuIsShowing = true;
    if (action == TurnoutThrow) {
        keypadUseType = KEYPAD_USE_SELECT_TURNOUTS_THROW;
    } else {
        keypadUseType = KEYPAD_USE_SELECT_TURNOUTS_CLOSE;
    }
    
    if (soFar == "") { // nothing entered yet
        clearTextArray();
        int j = 0;
        for (int i=0; i<10 && i<turnoutListSize; i++) {
            j = (i<5) ? i : i+1;
            if (turnoutListUserName[(page*10)+i].length()>0) {
                oledText[j] = String(turnoutListIndex[i]) + ": " + turnoutListUserName[(page*10)+i].substring(0,10);
            }
        }
        oledText[5] = "(" + String(page+1) +  ") " + menu_text[menu_turnout_list];
        drawTextArray(false, false, true, false);
    }
}

void DisplayManager::showRouteList(const String& soFar) {
    // Variables externas que necesitamos
    extern int lastOledScreen;
    extern int last_oled_screen_route_list;
    extern String lastOledStringParameter;
    extern bool menuIsShowing;
    extern int keypadUseType;
    extern int KEYPAD_USE_SELECT_ROUTES;
    extern int page;
    extern int routeListSize;
    extern int routeListIndex[];
    extern String routeListUserName[];
    extern String menu_text[];
    extern int menu_route_list;
    
    lastOledScreen = last_oled_screen_route_list;
    lastOledStringParameter = soFar;
    
    menuIsShowing = true;
    keypadUseType = KEYPAD_USE_SELECT_ROUTES;
    
    if (soFar == "") { // nothing entered yet
        clearTextArray();
        int j = 0;
        for (int i=0; i<10 && i<routeListSize; i++) {
            j = (i<5) ? i : i+1;
            if (routeListUserName[(page*10)+i].length()>0) {
                oledText[j] = String(routeListIndex[i]) + ": " + routeListUserName[(page*10)+i].substring(0,10);
            }
        }
        oledText[5] = "(" + String(page+1) +  ") " + menu_text[menu_route_list];
        drawTextArray(false, false, true, false);
    }
}

void DisplayManager::showFunctionList(const String& soFar) {
    // Variables externas que necesitamos
    extern int lastOledScreen;
    extern int last_oled_screen_function_list;
    extern String lastOledStringParameter;
    extern bool menuIsShowing;
    extern int keypadUseType;
    extern int KEYPAD_USE_SELECT_FUNCTION;
    extern bool functionHasBeenSelected;
    extern WiThrottleProtocol wiThrottleProtocol;
    extern char currentThrottleIndexChar;
    extern int currentThrottleIndex;
    extern int functionPage;
    extern int MAX_FUNCTIONS;
    extern String functionLabels[][32];
    extern bool functionStates[][32];
    extern String menu_text[];
    extern int menu_function_list;
    extern String MSG_NO_FUNCTIONS;
    extern String MSG_THROTTLE_NUMBER;
    extern String MSG_NO_LOCO_SELECTED;
    extern String menu_cancel;
    extern void setMenuTextForOled(String text);
    
    lastOledScreen = last_oled_screen_function_list;
    lastOledStringParameter = soFar;
    
    menuIsShowing = true;
    keypadUseType = KEYPAD_USE_SELECT_FUNCTION;
    functionHasBeenSelected = false;
    
    if (soFar == "") { // nothing entered yet
        clearTextArray();
        if (wiThrottleProtocol.getNumberOfLocomotives(currentThrottleIndexChar) > 0 ) {
            int j = 0; int k = 0;
            for (int i=0; i<10; i++) {
                k = (functionPage*10) + i;
                if (k < MAX_FUNCTIONS) {
                    j = (i<5) ? i : i+1;
                    oledText[j] = String(i) + ": " 
                                + ((k<10) ? functionLabels[currentThrottleIndex][k].substring(0,10) : String(k) 
                                + "-" + functionLabels[currentThrottleIndex][k].substring(0,7)) ;
                    
                    if (functionStates[currentThrottleIndex][k]) {
                        oledTextInvert[j] = true;
                    }
                }
            }
            oledText[5] = "(" + String(functionPage) +  ") " + menu_text[menu_function_list];
        } else {
            oledText[0] = MSG_NO_FUNCTIONS;
            oledText[2] = MSG_THROTTLE_NUMBER + String(currentThrottleIndex+1);
            oledText[3] = MSG_NO_LOCO_SELECTED;
            setMenuTextForOled(menu_cancel);
        }
        drawTextArray(false, false, true, false);
    }
}

void DisplayManager::showSsidList(const String& soFar) {
    // Variables externas que necesitamos
    extern bool menuIsShowing;
    extern int keypadUseType;
    extern int KEYPAD_USE_SELECT_SSID_FROM_FOUND;
    extern int page;
    extern int foundSsidsCount;
    extern String foundSsids[];
    extern String foundSsidRssis[];
    extern String menu_text[];
    extern int menu_select_ssids_from_found;
    
    menuIsShowing = true;
    keypadUseType = KEYPAD_USE_SELECT_SSID_FROM_FOUND;
    
    if (soFar == "") { // nothing entered yet
        clearTextArray();
        for (int i=0; i<5 && i<foundSsidsCount; i++) {
            if (foundSsids[(page*5)+i].length()>0) {
                oledText[i] = String(i) + ": " + foundSsids[(page*5)+i] + "   (" + foundSsidRssis[(page*5)+i] + ")" ;
            }
        }
        oledText[5] = "(" + String(page+1) +  ") " + menu_text[menu_select_ssids_from_found];
        drawTextArray(false, false, true, false);
    }
}

void DisplayManager::showPasswordEntry() {
    clearTextArray();
    oledText[0] = "ENTER PASSWORD";
    drawTextArray(false, true, true, false);
}

void DisplayManager::showDirectCommands() {
    // Variables externas que necesitamos
    extern String DIRECT_COMMAND_LIST;
    extern String directCommandText[4][3];
    extern bool oledDirectCommandsAreBeingDisplayed;
    extern int lastOledScreen;
    extern int last_oled_screen_direct_commands;
    
    lastOledScreen = last_oled_screen_direct_commands;
    oledDirectCommandsAreBeingDisplayed = true;
    
    clearTextArray();
    oledText[0] = DIRECT_COMMAND_LIST;
    
    // Llenar las tres columnas con los comandos directos
    for (int i=0; i < 4; i++) {
        oledText[i+1] = directCommandText[i][0];
    }
    int j = 0;
    for (int i=6; i < 10; i++) {
        oledText[i+1] = directCommandText[j][1];
        j++;
    }
    j=0;
    for (int i=12; i < 16; i++) {
        oledText[i+1] = directCommandText[j][2];
        j++;
    }
    
    drawTextArray(true, false, true, false);
}

void DisplayManager::showEditConsist() {
    clearTextArray();
    oledText[0] = "EDIT CONSIST";
    drawTextArray(false, false, true, false);
}

void DisplayManager::showAllLocos(bool hideLeadLoco) {
    clearTextArray();
    oledText[0] = "ALL LOCOS";
    drawTextArray(false, false, true, false);
}