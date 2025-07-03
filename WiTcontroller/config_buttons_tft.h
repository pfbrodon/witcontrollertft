// ********************************************************************************************
//
// TFT Configuration for WiTcontroller - Based on original config_buttons.h
// This version includes TFT-specific settings while maintaining compatibility
//
// ********************************************************************************************

// ********************************************************************************************
// TFT Display Configuration
// 
// Uncomment the line below to enable TFT ST7735 support instead of OLED
#define USE_TFT_DISPLAY true

// TFT Hardware pins - Adjust according to your wiring
#define TFT_CS_PIN    5   // Chip select
#define TFT_RST_PIN   4   // Reset (can be connected to ESP32 reset)
#define TFT_DC_PIN    2   // Data/Command
// MOSI and SCLK use default SPI pins (GPIO 23 and 18)

// TFT Display type and options
#define TFT_DISPLAY_TYPE    ST7735_BLACKTAB    // or ST7735_GREENTAB, ST7735_REDTAB
#define TFT_ROTATION        0                  // 0=Portrait, 1=Landscape, 2=Portrait_flipped, 3=Landscape_flipped

// TFT Color theme (you can customize these)
#define TFT_THEME_BACKGROUND     0x0000  // Black
#define TFT_THEME_TEXT          0xFFFF  // White  
#define TFT_THEME_SPEED         0x07E0  // Green
#define TFT_THEME_DIRECTION     0xFFE0  // Yellow
#define TFT_THEME_FUNCTIONS     0x001F  // Blue
#define TFT_THEME_BATTERY       0x07E0  // Green
#define TFT_THEME_MENU          0xC618  // Light Grey
#define TFT_THEME_ERROR         0xF800  // Red
#define TFT_THEME_HIGHLIGHT     0x07FF  // Cyan

// Enable enhanced TFT features
#define TFT_ENHANCED_DISPLAY    true   // Use colors and better layout
#define TFT_SHOW_BATTERY_PERCENT true  // Show battery percentage with icon
#define TFT_LARGE_SPEED_DISPLAY true   // Use larger fonts for speed

// ********************************************************************************************

// Encoder Direction and Actions
#define ENCODER_ROTATION_CLOCKWISE_IS_INCREASE_SPEED                     false

// Define what the rotary encoder button does.   (Pushing down on the knob)
#define ENCODER_BUTTON_ACTION SPEED_STOP_THEN_TOGGLE_DIRECTION 

// Rotary Encoder Debounce Time
#define ROTARY_ENCODER_DEBOUNCE_TIME     200

// *******************************************************************************************************************
// Keypad Definitions / Actions

// Define what each button will do as direct press (not in a menu)   * and # cannot be remapped

#define CHOSEN_KEYPAD_0_FUNCTION     FUNCTION_0
#define CHOSEN_KEYPAD_1_FUNCTION     FUNCTION_1
#define CHOSEN_KEYPAD_2_FUNCTION     FUNCTION_2
#define CHOSEN_KEYPAD_3_FUNCTION     FUNCTION_3
#define CHOSEN_KEYPAD_4_FUNCTION     FUNCTION_4
#define CHOSEN_KEYPAD_5_FUNCTION     NEXT_THROTTLE
#define CHOSEN_KEYPAD_6_FUNCTION     SPEED_MULTIPLIER
#define CHOSEN_KEYPAD_7_FUNCTION     DIRECTION_REVERSE
#define CHOSEN_KEYPAD_8_FUNCTION     E_STOP
#define CHOSEN_KEYPAD_9_FUNCTION     DIRECTION_FORWARD

// text that will appear when you press #
#define CHOSEN_KEYPAD_0_DISPLAY_NAME     "0 Lights"
#define CHOSEN_KEYPAD_1_DISPLAY_NAME     "1 Bell"
#define CHOSEN_KEYPAD_2_DISPLAY_NAME     "2 Horn"
#define CHOSEN_KEYPAD_3_DISPLAY_NAME     "3 F3"
#define CHOSEN_KEYPAD_4_DISPLAY_NAME     "4 F4"
#define CHOSEN_KEYPAD_5_DISPLAY_NAME     "5 Nxt Ttl"
#define CHOSEN_KEYPAD_6_DISPLAY_NAME     "6 X Spd"
#define CHOSEN_KEYPAD_7_DISPLAY_NAME     "7 Rev"
#define CHOSEN_KEYPAD_8_DISPLAY_NAME     "8 Estop"
#define CHOSEN_KEYPAD_9_DISPLAY_NAME     "9 Fwd"

// *******************************************************************************************************************
// Encoder Hardware
// defaults are for the Small ESP32

// Small ESP32
#define ROTARY_ENCODER_A_PIN 12
#define ROTARY_ENCODER_B_PIN 14
#define ROTARY_ENCODER_BUTTON_PIN 13

// *******************************************************************************************************************
// Keypad Hardware
// Defaults are for the Small 4x3 keypad

// 4x3 keypad - default
#define ROW_NUM     4
#define COLUMN_NUM  3
#define KEYPAD_KEYS  {'1', '2', '3'}, {'4', '5', '6'}, {'7', '8', '9'}, {'*', '0', '#'}
#define KEYPAD_ROW_PINS    {32, 33, 21, 22}
#define KEYPAD_COLUMN_PINS { 15, 16, 27}

// *******************************************************************************************************************
// TFT Display Definition (replaces oLED section)

#ifndef USE_TFT_DISPLAY
   // Original OLED configuration would go here
   // #define OLED_TYPE U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 22, /* data=*/ 23);
#else
   // TFT configuration is handled in config_tft.h and tft_wrapper.cpp
   #define DISPLAY_TYPE_TFT_ST7735
#endif

// *******************************************************************************************************************
// Battery Test with enhanced TFT display

#define USE_BATTERY_TEST true
#define BATTERY_TEST_PIN 34
#define BATTERY_CONVERSION_FACTOR 1.7
#define USE_BATTERY_PERCENT_AS_WELL_AS_ICON true
#define USE_BATTERY_SLEEP_AT_PERCENT 3

// *******************************************************************************************************************
// Debugging

#define WITCONTROLLER_DEBUG    0
// #define WITHROTTLE_PROTOCOL_DEBUG    1
// #define DEBUG_LEVEL    1

// *******************************************************************************************************************
// Additional TFT-specific settings

#ifdef USE_TFT_DISPLAY
    // TFT allows for more detailed displays
    #define DISPLAY_LOCO_NAME              // Show loco names instead of just addresses
    #define MAX_THROTTLES                  3  // TFT can handle more throttles display
    #define HASH_SHOWS_FUNCTIONS_INSTEAD_OF_KEY_DEFS    true  // Show functions directly
    
    // Enhanced display options
    #define SPEED_STEP                     2   // Smaller steps for smoother display
    #define DISPLAY_SPEED_AS_PERCENT       false
    #define DISPLAY_SPEED_AS_0_TO_28       false
    
    // Use TFT-optimized font sizes (these are handled in tft_wrapper.cpp)
    #define FONT_DEFAULT                   1  // Small font
    #define FONT_SPEED                     3  // Large font for speed
    #define FONT_DIRECTION                 2  // Medium font for direction
    #define FONT_THROTTLE_NUMBER           2  // Medium font for throttle number
#endif

// *******************************************************************************************************************
// Network Configuration
// You still need to create config_network.h with your WiFi settings