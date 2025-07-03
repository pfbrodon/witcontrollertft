// ********************************************************************************************
//
// TFT Modifications Header - To be included in WiTcontroller.ino
// This file contains all the TFT-specific modifications needed
//
// ********************************************************************************************

#ifndef TFT_MODIFICATIONS_H
#define TFT_MODIFICATIONS_H

// Include TFT libraries and configuration
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include "config_tft.h"

// TFT instance - replaces u8g2
extern Adafruit_ST7735 tft;

// TFT-specific variables
extern bool tftInitialized;
extern uint16_t lastSpeedDisplayed[6];
extern String lastDirectionDisplayed[6];
extern bool functionsChanged;
extern int lastBatteryPercentDisplayed;

// *********************************************************************************
// TFT Wrapper Functions - Replace u8g2 functions

class TFTDisplay {
public:
    static void begin();
    static void clearBuffer();
    static void sendBuffer();
    static void firstPage();
    
    static void setFont(int fontSize);
    static void setDrawColor(int color);
    static void drawStr(int x, int y, const char* str);
    static void drawBox(int x, int y, int w, int h);
    static void drawGlyph(int x, int y, int glyph);
    
    // TFT-enhanced functions
    static void drawSpeedDisplay(int speed, Direction dir, int throttleNum);
    static void drawFunctionIndicators(bool functions[32]);
    static void drawBatteryIndicator(int batteryPercent);
    static void drawLocoInfo(int throttleIndex);
    static void drawMenuText(const char* menuText);
    static void drawTrackPower(TrackPower power);
    
private:
    static int currentFontSize;
    static uint16_t currentTextColor;
    static int currentDrawColor;
    static int mapOLEDtoTFT_X(int oled_x);
    static int mapOLEDtoTFT_Y(int oled_y);
};

// Global instance
extern TFTDisplay tftDisplay;

// Macros to replace u8g2 calls in original code
#define u8g2 tftDisplay

// *********************************************************************************
// Modified display functions for TFT

void writeOledSpeedTFT();
void writeOledFunctionsTFT();
void writeOledBatteryTFT();
void writeOledLocoInfoTFT();
void writeOledArrayTFT(bool isThreeColumns, bool isPassword, bool sendBuffer, bool drawTopLine);

// *********************************************************************************
// TFT Initialization function

void initializeTFT();

#endif // TFT_MODIFICATIONS_H