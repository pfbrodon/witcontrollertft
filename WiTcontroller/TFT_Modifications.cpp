// ********************************************************************************************
//
// TFT Modifications Implementation
// This file implements all TFT-specific functions to replace OLED functionality
//
// ********************************************************************************************

#include "TFT_Modifications.h"

// TFT instance definition
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS_PIN, TFT_DC_PIN, TFT_RST_PIN);

// TFT-specific variables
bool tftInitialized = false;
uint16_t lastSpeedDisplayed[6] = {999, 999, 999, 999, 999, 999};
String lastDirectionDisplayed[6] = {"", "", "", "", "", ""};
bool functionsChanged = true;
int lastBatteryPercentDisplayed = -1;

// Static members
int TFTDisplay::currentFontSize = FONT_SIZE_SMALL;
uint16_t TFTDisplay::currentTextColor = TFT_WHITE;
int TFTDisplay::currentDrawColor = 1;

// Global instance
TFTDisplay tftDisplay;

// *********************************************************************************
// TFT Display Implementation

void TFTDisplay::begin() {
    tft.initR(INITR_BLACKTAB);
    tft.setRotation(TFT_ROTATION);
    tft.fillScreen(TFT_THEME_BACKGROUND);
    tftInitialized = true;
    
    // Show startup screen
    setFont(FONT_SIZE_MEDIUM);
    setDrawColor(1);
    drawStr(10, 30, "WiTcontroller");
    drawStr(10, 50, "TFT Edition");
    drawStr(10, 70, "Starting...");
    sendBuffer();
    delay(2000);
}

void TFTDisplay::clearBuffer() {
    if (!tftInitialized) return;
    // Clear only the OLED-compatible area
    tft.fillRect(0, 0, OLED_WIDTH, OLED_HEIGHT, TFT_THEME_BACKGROUND);
}

void TFTDisplay::sendBuffer() {
    // TFT draws immediately, so this is a no-op
    // But we can use it for any final updates
}

void TFTDisplay::firstPage() {
    clearBuffer();
}

void TFTDisplay::setFont(int fontSize) {
    currentFontSize = fontSize;
    tft.setTextSize(fontSize);
}

void TFTDisplay::setDrawColor(int color) {
    currentDrawColor = color;
    if (color == 0) {
        currentTextColor = TFT_THEME_BACKGROUND;
    } else {
        currentTextColor = TFT_THEME_TEXT;
    }
    tft.setTextColor(currentTextColor);
}

void TFTDisplay::drawStr(int x, int y, const char* str) {
    int tft_x = mapOLEDtoTFT_X(x);
    int tft_y = mapOLEDtoTFT_Y(y);
    
    tft.setCursor(tft_x, tft_y);
    tft.setTextColor(currentTextColor, TFT_THEME_BACKGROUND);
    tft.print(str);
}

void TFTDisplay::drawBox(int x, int y, int w, int h) {
    int tft_x = mapOLEDtoTFT_X(x);
    int tft_y = mapOLEDtoTFT_Y(y);
    
    uint16_t fillColor = (currentDrawColor == 0) ? TFT_THEME_BACKGROUND : currentTextColor;
    tft.fillRect(tft_x, tft_y, w, h, fillColor);
}

void TFTDisplay::drawGlyph(int x, int y, int glyph) {
    int tft_x = mapOLEDtoTFT_X(x);
    int tft_y = mapOLEDtoTFT_Y(y);
    
    tft.setCursor(tft_x, tft_y);
    tft.setTextColor(currentTextColor, TFT_THEME_BACKGROUND);
    
    // Map common glyphs to characters
    switch(glyph) {
        case 0x00b7: // heartbeat off
            tft.print("*");
            break;
        case 0x00eb: // track power
            tft.print("P");
            break;
        case 0x00d6: // speed step multiplier
            tft.print("X");
            break;
        default:
            tft.print("?");
            break;
    }
}

// *********************************************************************************
// Additional U8G2-compatible methods

void TFTDisplay::drawRBox(int x, int y, int w, int h, int r) {
    // For simplicity, draw a regular rectangle (TFT library doesn't have rounded rectangles easily)
    uint16_t fillColor = (currentDrawColor == 0) ? TFT_THEME_BACKGROUND : currentTextColor;
    tft.fillRect(x, y, w, h, fillColor);
}

void TFTDisplay::drawLine(int x1, int y1, int x2, int y2) {
    uint16_t lineColor = (currentDrawColor == 0) ? TFT_THEME_BACKGROUND : currentTextColor;
    tft.drawLine(x1, y1, x2, y2, lineColor);
}

void TFTDisplay::drawUTF8(int x, int y, const char* str) {
    // UTF8 handling simplified - just draw as regular string
    drawStr(x, y, str);
}

void TFTDisplay::drawHLine(int x, int y, int w) {
    uint16_t lineColor = (currentDrawColor == 0) ? TFT_THEME_BACKGROUND : currentTextColor;
    tft.drawFastHLine(x, y, w, lineColor);
}

int TFTDisplay::getStrWidth(const char* str) {
    // Approximate string width based on current font size
    int charWidth = 6 * currentFontSize; // Basic approximation
    return strlen(str) * charWidth;
}

void TFTDisplay::setPowerSave(int mode) {
    // TFT displays don't typically have power save - this is a no-op
    // Could potentially control backlight if available
}

// *********************************************************************************
// Enhanced TFT functions

void TFTDisplay::drawSpeedDisplay(int speed, Direction dir, int throttleNum) {
    // Clear speed area
    tft.fillRect(SPEED_X, SPEED_Y, SPEED_WIDTH + DIRECTION_WIDTH, SPEED_HEIGHT, TFT_THEME_BACKGROUND);
    
    // Draw speed with large, colored text
    tft.setTextColor(TFT_THEME_SPEED, TFT_THEME_BACKGROUND);
    tft.setCursor(SPEED_X, SPEED_Y);
    tft.setTextSize(3);
    tft.print(speed);
    
    // Draw direction
    String directionStr = (dir == Forward) ? "FWD" : "REV";
    uint16_t dirColor = (dir == Forward) ? TFT_THEME_DIRECTION : TFT_ORANGE;
    
    tft.setTextColor(dirColor, TFT_THEME_BACKGROUND);
    tft.setCursor(DIRECTION_X, DIRECTION_Y);
    tft.setTextSize(2);
    tft.print(directionStr);
    
    // Draw throttle number if multiple throttles
    if (throttleNum >= 0) {
        tft.setTextColor(TFT_THEME_HIGHLIGHT, TFT_THEME_BACKGROUND);
        tft.setCursor(THROTTLE_NUM_X, THROTTLE_NUM_Y);
        tft.setTextSize(2);
        tft.print("T");
        tft.print(throttleNum + 1);
    }
}

void TFTDisplay::drawFunctionIndicators(bool functions[32]) {
    int x = 0;
    int y = FUNCTIONS_Y;
    
    // Clear function area
    tft.fillRect(0, y, OLED_WIDTH, FUNCTIONS_HEIGHT * 2, TFT_THEME_BACKGROUND);
    
    tft.setTextColor(TFT_THEME_FUNCTIONS, TFT_THEME_BACKGROUND);
    tft.setTextSize(1);
    
    for (int i = 0; i < 32; i++) {
        if (i > 0 && i % 16 == 0) {
            x = 0;
            y += CHAR_HEIGHT_SMALL;
        }
        
        if (functions[i]) {
            tft.setCursor(x, y);
            if (i < 10) {
                tft.print(i);
            } else if (i < 36) {
                tft.print((char)('A' + i - 10));
            } else {
                tft.print("?");
            }
        }
        
        x += CHAR_WIDTH_SMALL;
    }
}

void TFTDisplay::drawBatteryIndicator(int batteryPercent) {
    // Determine battery color
    uint16_t batteryColor;
    if (batteryPercent > 50) {
        batteryColor = TFT_THEME_BATTERY;
    } else if (batteryPercent > 20) {
        batteryColor = TFT_YELLOW;
    } else {
        batteryColor = TFT_THEME_ERROR;
    }
    
    // Clear battery area
    tft.fillRect(BATTERY_X, BATTERY_Y, BATTERY_WIDTH, BATTERY_HEIGHT + 10, TFT_THEME_BACKGROUND);
    
    // Draw battery outline
    tft.drawRect(BATTERY_X, BATTERY_Y, BATTERY_WIDTH-2, BATTERY_HEIGHT, batteryColor);
    tft.drawRect(BATTERY_X + BATTERY_WIDTH - 2, BATTERY_Y + 2, 2, BATTERY_HEIGHT - 4, batteryColor);
    
    // Fill battery based on percentage
    int fillWidth = ((BATTERY_WIDTH - 4) * batteryPercent) / 100;
    if (fillWidth > 0) {
        tft.fillRect(BATTERY_X + 1, BATTERY_Y + 1, fillWidth, BATTERY_HEIGHT - 2, batteryColor);
    }
    
    // Show percentage if enabled
    if (TFT_SHOW_BATTERY_PERCENT) {
        tft.setTextColor(batteryColor, TFT_THEME_BACKGROUND);
        tft.setCursor(BATTERY_X, BATTERY_Y + BATTERY_HEIGHT + 2);
        tft.setTextSize(1);
        tft.print(batteryPercent);
        tft.print("%");
    }
}

void TFTDisplay::drawLocoInfo(int throttleIndex) {
    // This function would draw locomotive information
    // Implementation depends on access to locomotive data structures
    tft.setTextColor(TFT_THEME_TEXT, TFT_THEME_BACKGROUND);
    tft.setCursor(2, 2);
    tft.setTextSize(1);
    tft.print("Throttle ");
    tft.print(throttleIndex + 1);
}

void TFTDisplay::drawMenuText(const char* menuText) {
    // Draw menu text at bottom with background
    tft.fillRect(0, MENU_Y, OLED_WIDTH, MENU_HEIGHT, TFT_THEME_MENU);
    tft.setTextColor(TFT_BLACK, TFT_THEME_MENU);
    tft.setCursor(MARGIN_LEFT, MENU_Y);
    tft.setTextSize(1);
    tft.print(menuText);
}

void TFTDisplay::drawTrackPower(TrackPower power) {
    uint16_t powerColor = (power == PowerOn) ? TFT_THEME_BATTERY : TFT_THEME_ERROR;
    
    tft.setTextColor(powerColor, TFT_THEME_BACKGROUND);
    tft.setCursor(90, 2);
    tft.setTextSize(1);
    tft.print((power == PowerOn) ? "PWR" : "OFF");
}

// *********************************************************************************
// Helper functions

int TFTDisplay::mapOLEDtoTFT_X(int oled_x) {
    return oled_x; // Direct mapping for now
}

int TFTDisplay::mapOLEDtoTFT_Y(int oled_y) {
    return oled_y; // Direct mapping for now
}

// *********************************************************************************
// Modified display functions for integration

void writeOledSpeedTFT() {
    if (!tftInitialized) return;
    
    // Clear display
    tftDisplay.clearBuffer();
    
    // Draw enhanced speed display
    extern int currentSpeed[6];
    extern Direction currentDirection[6];
    extern int currentThrottleIndex;
    extern int maxThrottles;
    
    tftDisplay.drawSpeedDisplay(
        currentSpeed[currentThrottleIndex], 
        currentDirection[currentThrottleIndex], 
        (maxThrottles > 1) ? currentThrottleIndex : -1
    );
    
    // Draw function indicators
    extern bool functionStates[6][MAX_FUNCTIONS];
    tftDisplay.drawFunctionIndicators(functionStates[currentThrottleIndex]);
    
    // Draw battery if enabled
    extern bool useBatteryTest;
    extern int batteryTestValue;
    if (useBatteryTest) {
        tftDisplay.drawBatteryIndicator(batteryTestValue);
    }
    
    // Draw track power
    extern TrackPower trackPower;
    tftDisplay.drawTrackPower(trackPower);
}

void writeOledArrayTFT(bool isThreeColumns, bool isPassword, bool sendBuffer, bool drawTopLine) {
    if (!tftInitialized) {
        // Fallback to original function if TFT not available
        return;
    }
    
    tftDisplay.clearBuffer();
    
    // Draw the oledText array content with TFT formatting
    extern String oledText[18];
    extern bool oledTextInvert[18];
    
    int y = MARGIN_TOP;
    tft.setTextSize(1);
    
    for (int i = 0; i < 15 && y < OLED_HEIGHT - 8; i++) {
        if (oledText[i].length() > 0) {
            uint16_t textColor = oledTextInvert[i] ? TFT_THEME_BACKGROUND : TFT_THEME_TEXT;
            uint16_t bgColor = oledTextInvert[i] ? TFT_THEME_TEXT : TFT_THEME_BACKGROUND;
            
            if (oledTextInvert[i]) {
                tft.fillRect(0, y, OLED_WIDTH, LINE_HEIGHT, bgColor);
            }
            
            tft.setTextColor(textColor, bgColor);
            tft.setCursor(MARGIN_LEFT, y);
            
            if (isPassword) {
                // Show password characters as asterisks
                String maskedText = "";
                for (int j = 0; j < oledText[i].length(); j++) {
                    maskedText += "*";
                }
                tft.print(maskedText);
            } else {
                tft.print(oledText[i]);
            }
            
            y += LINE_HEIGHT;
        }
    }
    
    if (sendBuffer) {
        tftDisplay.sendBuffer();
    }
}

// *********************************************************************************
// Initialization function

void initializeTFT() {
    debug_println("Initializing TFT display...");
    tftDisplay.begin();
    
    // Test display
    tftDisplay.setFont(FONT_SIZE_MEDIUM);
    tftDisplay.setDrawColor(1);
    tftDisplay.drawStr(10, 80, "TFT Ready!");
    tftDisplay.sendBuffer();
    
    delay(1000);
    tftDisplay.clearBuffer();
    
    debug_println("TFT initialization complete");
}