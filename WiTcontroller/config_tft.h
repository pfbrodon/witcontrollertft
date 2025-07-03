// ********************************************************************************************
//
// TFT ST7735 Configuration for WiTcontroller
// This file replaces u8g2 OLED functionality with Adafruit TFT libraries
//
// ********************************************************************************************

#ifndef CONFIG_TFT_H
#define CONFIG_TFT_H

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>

// *******************************************************************************************************************
// TFT Hardware pins - Adjust according to your wiring
// For ESP32 with ST7735 1.8" TFT

#ifndef TFT_CS_PIN
  #define TFT_CS_PIN    5   // Chip select
#endif
#ifndef TFT_RST_PIN
  #define TFT_RST_PIN   4   // Reset
#endif
#ifndef TFT_DC_PIN
  #define TFT_DC_PIN    2   // Data/Command
#endif
#ifndef TFT_MOSI_PIN
  #define TFT_MOSI_PIN  23  // SPI MOSI
#endif
#ifndef TFT_SCLK_PIN
  #define TFT_SCLK_PIN  18  // SPI SCLK
#endif

// *******************************************************************************************************************
// Display specifications
#define TFT_WIDTH  128
#define TFT_HEIGHT 160

// Screen layout constants to match OLED layout
#define OLED_WIDTH  128
#define OLED_HEIGHT 64

// *******************************************************************************************************************
// Color definitions (16-bit RGB565)
#define TFT_BLACK       0x0000
#define TFT_WHITE       0xFFFF
#define TFT_RED         0xF800
#define TFT_GREEN       0x07E0
#define TFT_BLUE        0x001F
#define TFT_CYAN        0x07FF
#define TFT_MAGENTA     0xF81F
#define TFT_YELLOW      0xFFE0
#define TFT_ORANGE      0xFD20
#define TFT_DARKGREY    0x7BEF
#define TFT_LIGHTGREY   0xC618

// Theme colors for different screen elements
#define COLOR_BACKGROUND     TFT_BLACK
#define COLOR_TEXT          TFT_WHITE
#define COLOR_HIGHLIGHT     TFT_CYAN
#define COLOR_SPEED         TFT_GREEN
#define COLOR_DIRECTION     TFT_YELLOW
#define COLOR_FUNCTIONS     TFT_BLUE
#define COLOR_BATTERY       TFT_GREEN
#define COLOR_MENU          TFT_LIGHTGREY
#define COLOR_ERROR         TFT_RED

// Theme constants used in TFT_Modifications
#define TFT_ROTATION         0
#define TFT_THEME_BACKGROUND COLOR_BACKGROUND
#define TFT_THEME_TEXT       COLOR_TEXT
#define TFT_THEME_HIGHLIGHT  COLOR_HIGHLIGHT
#define TFT_THEME_SPEED      COLOR_SPEED
#define TFT_THEME_DIRECTION  COLOR_DIRECTION
#define TFT_THEME_FUNCTIONS  COLOR_FUNCTIONS
#define TFT_THEME_BATTERY    COLOR_BATTERY
#define TFT_THEME_MENU       COLOR_MENU
#define TFT_THEME_ERROR      COLOR_ERROR
#define TFT_SHOW_BATTERY_PERCENT true

// Constants used in original code that need to be defined
#define MAX_FUNCTIONS 32

// TFT Enhanced display settings (from config_buttons_tft.h)
#ifndef TFT_ENHANCED_DISPLAY
#define TFT_ENHANCED_DISPLAY true
#endif

// Font constants to replace U8G2 font definitions
#define u8g2_font_neuecraft_tr          FONT_SIZE_MEDIUM
#define u8g2_font_open_iconic_all_1x_t  FONT_SIZE_SMALL
#define u8g2_font_profont29_mr          FONT_SIZE_LARGE
#define u8g2_font_6x12_m_symbols        FONT_SIZE_SMALL
#define u8g2_font_9x15_tf               FONT_SIZE_MEDIUM
#define u8g2_font_tiny_simon_tr         FONT_SIZE_SMALL

// U8G2 compatibility constants
#define U8G2_R0 0

// *******************************************************************************************************************
// Font sizes to approximate u8g2 fonts
#define FONT_SIZE_SMALL     1
#define FONT_SIZE_MEDIUM    2
#define FONT_SIZE_LARGE     3

// Character dimensions for layout calculations
#define CHAR_WIDTH_SMALL    6
#define CHAR_HEIGHT_SMALL   8
#define CHAR_WIDTH_MEDIUM   12
#define CHAR_HEIGHT_MEDIUM  16
#define CHAR_WIDTH_LARGE    18
#define CHAR_HEIGHT_LARGE   24

// *******************************************************************************************************************
// Layout constants to match OLED display layout
#define LINES_MAX           8   // Maximum lines that fit in OLED height equivalent
#define LINE_HEIGHT         8   // Height per line to match OLED
#define MARGIN_LEFT         2
#define MARGIN_TOP          2

// Function indicators area
#define FUNCTIONS_Y         16
#define FUNCTIONS_HEIGHT    8

// Speed/direction area  
#define SPEED_X             45
#define SPEED_Y             28
#define SPEED_WIDTH         38
#define SPEED_HEIGHT        24

#define DIRECTION_X         85
#define DIRECTION_Y         28
#define DIRECTION_WIDTH     20
#define DIRECTION_HEIGHT    12

// Menu/message area
#define MENU_Y              56
#define MENU_HEIGHT         8

// Throttle number indicator
#define THROTTLE_NUM_X      2
#define THROTTLE_NUM_Y      15
#define THROTTLE_NUM_WIDTH  12
#define THROTTLE_NUM_HEIGHT 16

// Battery indicator
#define BATTERY_X           108
#define BATTERY_Y           2
#define BATTERY_WIDTH       18
#define BATTERY_HEIGHT      12

// *******************************************************************************************************************
// Create TFT instance
extern Adafruit_ST7735 tft;

// *******************************************************************************************************************
// TFT wrapper functions to replace u8g2 functions

class TFTWrapper {
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
    
    // Additional TFT-specific functions
    static void setTextColor(uint16_t color);
    static void setTextColor(uint16_t fg, uint16_t bg);
    static void setCursor(int x, int y);
    static void print(const char* text);
    static void println(const char* text);
    static void fillRect(int x, int y, int w, int h, uint16_t color);
    static void drawRect(int x, int y, int w, int h, uint16_t color);
    static void fillScreen(uint16_t color);
    
    // Layout helper functions
    static void drawOLEDCompatibleArea();
    static int mapOLEDtoTFT_X(int oled_x);
    static int mapOLEDtoTFT_Y(int oled_y);
    
    // Enhanced display functions - these were missing from the header
    static void drawSpeedDisplay(int speed, const char* direction, int throttleNum);
    static void drawFunctionIndicators(bool functions[32]);
    static void drawBatteryIndicator(int batteryPercent);
    static void drawMenuText(const char* menuText);
    
private:
    static int currentFontSize;
    static uint16_t currentTextColor;
    static uint16_t currentBgColor;
    static int currentDrawColor;
};

// Global TFT wrapper instance
extern TFTWrapper tftWrapper;

// Macros to replace u8g2 calls
#define u8g2 tftWrapper

#endif // CONFIG_TFT_H