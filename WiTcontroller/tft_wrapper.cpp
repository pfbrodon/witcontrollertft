// ********************************************************************************************
//
// TFT ST7735 Wrapper Implementation for WiTcontroller
// This file implements TFT functions to replace u8g2 OLED functionality
//
// ********************************************************************************************

#include "config_tft.h"

// TFT instance is defined in TFT_Modifications.cpp
// extern Adafruit_ST7735 tft;

// Static member initialization
int TFTWrapper::currentFontSize = FONT_SIZE_SMALL;
uint16_t TFTWrapper::currentTextColor = TFT_WHITE;
uint16_t TFTWrapper::currentBgColor = TFT_BLACK;
int TFTWrapper::currentDrawColor = 1; // 1 = white, 0 = black

// Global wrapper instance
TFTWrapper tftWrapper;

// *******************************************************************************************************************
// Basic TFT wrapper functions to replace u8g2

void TFTWrapper::begin() {
    tft.initR(INITR_BLACKTAB);  // Initialize ST7735 with black tab
    tft.setRotation(0);         // Portrait orientation
    tft.fillScreen(COLOR_BACKGROUND);
    
    // Draw the OLED-compatible area border for debugging (optional)
    // drawOLEDCompatibleArea();
}

void TFTWrapper::clearBuffer() {
    // Clear only the OLED-compatible area to maintain layout
    tft.fillRect(0, 0, OLED_WIDTH, OLED_HEIGHT, COLOR_BACKGROUND);
}

void TFTWrapper::sendBuffer() {
    // In TFT, drawing is immediate, so this is essentially a no-op
    // But we can use it for any final screen updates if needed
}

void TFTWrapper::firstPage() {
    // Initialize first page - similar to u8g2's page loop start
    clearBuffer();
}

// *******************************************************************************************************************
// Font and drawing functions

void TFTWrapper::setFont(int fontSize) {
    currentFontSize = fontSize;
    tft.setTextSize(fontSize);
}

void TFTWrapper::setDrawColor(int color) {
    currentDrawColor = color;
    if (color == 0) {
        currentTextColor = COLOR_BACKGROUND; // Black
    } else {
        currentTextColor = COLOR_TEXT;       // White
    }
    tft.setTextColor(currentTextColor);
}

void TFTWrapper::drawStr(int x, int y, const char* str) {
    // Map OLED coordinates to TFT coordinates
    int tft_x = mapOLEDtoTFT_X(x);
    int tft_y = mapOLEDtoTFT_Y(y);
    
    tft.setCursor(tft_x, tft_y);
    tft.setTextColor(currentTextColor, COLOR_BACKGROUND);
    tft.print(str);
}

void TFTWrapper::drawBox(int x, int y, int w, int h) {
    // Map OLED coordinates to TFT coordinates
    int tft_x = mapOLEDtoTFT_X(x);
    int tft_y = mapOLEDtoTFT_Y(y);
    
    uint16_t fillColor = (currentDrawColor == 0) ? COLOR_BACKGROUND : COLOR_TEXT;
    tft.fillRect(tft_x, tft_y, w, h, fillColor);
}

void TFTWrapper::drawGlyph(int x, int y, int glyph) {
    // Map OLED coordinates to TFT coordinates
    int tft_x = mapOLEDtoTFT_X(x);
    int tft_y = mapOLEDtoTFT_Y(y);
    
    // For now, draw simple symbols instead of glyphs
    // This can be expanded to support specific glyphs as needed
    tft.setCursor(tft_x, tft_y);
    tft.setTextColor(currentTextColor, COLOR_BACKGROUND);
    
    switch(glyph) {
        case 0x00b7: // heartbeat off (bullet)
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

// *******************************************************************************************************************
// Additional TFT-specific functions

void TFTWrapper::setTextColor(uint16_t color) {
    currentTextColor = color;
    tft.setTextColor(color);
}

void TFTWrapper::setTextColor(uint16_t fg, uint16_t bg) {
    currentTextColor = fg;
    currentBgColor = bg;
    tft.setTextColor(fg, bg);
}

void TFTWrapper::setCursor(int x, int y) {
    tft.setCursor(x, y);
}

void TFTWrapper::print(const char* text) {
    tft.print(text);
}

void TFTWrapper::println(const char* text) {
    tft.println(text);
}

void TFTWrapper::fillRect(int x, int y, int w, int h, uint16_t color) {
    tft.fillRect(x, y, w, h, color);
}

void TFTWrapper::drawRect(int x, int y, int w, int h, uint16_t color) {
    tft.drawRect(x, y, w, h, color);
}

void TFTWrapper::fillScreen(uint16_t color) {
    tft.fillScreen(color);
}

// *******************************************************************************************************************
// Layout helper functions

void TFTWrapper::drawOLEDCompatibleArea() {
    // Draw a border around the OLED-compatible area for debugging
    tft.drawRect(0, 0, OLED_WIDTH, OLED_HEIGHT, TFT_RED);
}

int TFTWrapper::mapOLEDtoTFT_X(int oled_x) {
    // Direct mapping since we're using the same width
    return oled_x;
}

int TFTWrapper::mapOLEDtoTFT_Y(int oled_y) {
    // Direct mapping for the OLED-compatible area
    return oled_y;
}

// *******************************************************************************************************************
// Enhanced display functions for better TFT utilization

void TFTWrapper::drawSpeedDisplay(int speed, const char* direction, int throttleNum) {
    // Enhanced speed display using TFT colors
    
    // Clear speed area
    fillRect(SPEED_X, SPEED_Y, SPEED_WIDTH + DIRECTION_WIDTH, SPEED_HEIGHT, COLOR_BACKGROUND);
    
    // Draw speed with larger, colored text
    setTextColor(COLOR_SPEED, COLOR_BACKGROUND);
    setCursor(SPEED_X, SPEED_Y);
    tft.setTextSize(3);
    tft.print(speed);
    
    // Draw direction
    setTextColor(COLOR_DIRECTION, COLOR_BACKGROUND);
    setCursor(DIRECTION_X, DIRECTION_Y);
    tft.setTextSize(2);
    tft.print(direction);
    
    // Draw throttle number indicator
    setTextColor(COLOR_HIGHLIGHT, COLOR_BACKGROUND);
    setCursor(THROTTLE_NUM_X, THROTTLE_NUM_Y);
    tft.setTextSize(2);
    tft.print(throttleNum);
}

void TFTWrapper::drawFunctionIndicators(bool functions[32]) {
    // Draw function indicators in a colored grid
    int x = 0;
    int y = FUNCTIONS_Y;
    
    setTextColor(COLOR_FUNCTIONS, COLOR_BACKGROUND);
    tft.setTextSize(1);
    
    for (int i = 0; i < 32; i++) {
        if (i > 0 && i % 16 == 0) {
            x = 0;
            y += CHAR_HEIGHT_SMALL;
        }
        
        if (functions[i]) {
            setCursor(x, y);
            if (i < 10) {
                tft.print(i);
            } else {
                tft.print((char)('A' + i - 10));
            }
        }
        
        x += CHAR_WIDTH_SMALL;
    }
}

void TFTWrapper::drawBatteryIndicator(int batteryPercent) {
    // Draw battery indicator with color coding
    uint16_t batteryColor;
    if (batteryPercent > 50) {
        batteryColor = COLOR_BATTERY;
    } else if (batteryPercent > 20) {
        batteryColor = TFT_YELLOW;
    } else {
        batteryColor = COLOR_ERROR;
    }
    
    // Clear battery area
    fillRect(BATTERY_X, BATTERY_Y, BATTERY_WIDTH, BATTERY_HEIGHT, COLOR_BACKGROUND);
    
    // Draw battery outline
    drawRect(BATTERY_X, BATTERY_Y, BATTERY_WIDTH-2, BATTERY_HEIGHT, batteryColor);
    drawRect(BATTERY_X + BATTERY_WIDTH - 2, BATTERY_Y + 2, 2, BATTERY_HEIGHT - 4, batteryColor);
    
    // Fill battery based on percentage
    int fillWidth = ((BATTERY_WIDTH - 4) * batteryPercent) / 100;
    fillRect(BATTERY_X + 1, BATTERY_Y + 1, fillWidth, BATTERY_HEIGHT - 2, batteryColor);
    
    // Show percentage if enabled
    setTextColor(batteryColor, COLOR_BACKGROUND);
    setCursor(BATTERY_X, BATTERY_Y + BATTERY_HEIGHT + 2);
    tft.setTextSize(1);
    tft.print(batteryPercent);
    tft.print("%");
}

void TFTWrapper::drawMenuText(const char* menuText) {
    // Draw menu text at bottom with background
    fillRect(0, MENU_Y, OLED_WIDTH, MENU_HEIGHT, COLOR_MENU);
    setTextColor(TFT_BLACK, COLOR_MENU);
    setCursor(MARGIN_LEFT, MENU_Y);
    tft.setTextSize(1);
    tft.print(menuText);
}