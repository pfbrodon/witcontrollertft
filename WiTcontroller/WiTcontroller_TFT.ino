/**
 * WiTcontroller TFT Version
 * This app turns an ESP32 into a dedicated Model Railway Controller for use
 * with JMRI or any wiThrottle server, using TFT ST7735 display instead of OLED.
 *
 * Instructions:
 * - Refer to the readme at https://github.com/flash62au/WiTcontroller
 * - This version adds TFT ST7735 support with enhanced color display
 */

#include <string>

// Use the Arduino IDE 'Boards' Manager to get these libraries
// They will be installed with the 'ESP32' Board library
// DO NOT DOWNLOAD THEM DIRECTLY!!!
#include <WiFi.h>                 // https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi     GPL 2.1
#include <ESPmDNS.h>              // https://github.com/espressif/arduino-esp32/blob/master/libraries/ESPmDNS  GPL 2.1
#include <esp_wifi.h>             // https://git.liberatedsystems.co.uk/jacob.eva/arduino-esp32/src/branch/master/tools/sdk/esp32s2/include/esp_wif  GPL 2.0

// ----------------------

#include <Preferences.h>

// use the Arduino IDE 'Library' Manager to get these libraries
#include <Keypad.h>               // https://www.arduinolibraries.info/libraries/keypad                        GPL 3.0
#include <WiThrottleProtocol.h>   // https://github.com/flash62au/WiThrottleProtocol                           Creative Commons 4.0  Attribution-ShareAlike
#include <AiEsp32RotaryEncoder.h> // https://github.com/igorantolic/ai-esp32-rotary-encoder                    GPL 2.0

// TFT libraries instead of U8g2
#include <Adafruit_GFX.h>         // Core graphics library
#include <Adafruit_ST7735.h>      // Hardware-specific library for ST7735
#include <SPI.h>

// this library is included with the WiTController code
#include "Pangodream_18650_CL.h"  // https://github.com/pangodream/18650CL                                     Copyright (c) 2019 Pangodream

// TFT-specific configuration files
#include "config_tft.h"           // TFT display configuration
#include "tft_wrapper.cpp"        // TFT wrapper implementation

// create these files by copying the example files and editing them as needed
#include "config_network.h"       // LAN networks (SSIDs and passwords)
#include "config_buttons_tft.h"   // TFT version of keypad buttons assignments

// DO NOT ALTER these files
#include "config_keypad_etc.h"
#include "static.h"
#include "actions.h"
#include "WiTcontroller.h"

#if WITCONTROLLER_DEBUG == 0
 #define debug_print(params...) Serial.print(params)
 #define debug_println(params...) Serial.print(params); Serial.print(" ("); Serial.print(millis()); Serial.println(")")
 #define debug_printf(params...) Serial.printf(params)
#else
 #define debug_print(...)
 #define debug_println(...)
 #define debug_printf(...)
#endif
int debugLevel = DEBUG_LEVEL;

// *********************************************************************************
// TFT-specific variables

bool tftInitialized = false;
uint16_t lastSpeedDisplayed = 999;  // Force initial update
String lastDirectionDisplayed = "";
bool functionsChanged = true;
int lastBatteryPercentDisplayed = -1;

// *********************************************************************************
// non-volatile storage

Preferences nvsPrefs;
bool nvsInit = false;
bool nvsPrefsSaved = false;
bool preferencesRead = false;

// *********************************************************************************

int keypadUseType = KEYPAD_USE_OPERATION;
int encoderUseType = ENCODER_USE_OPERATION;
int encoderButtonAction = ENCODER_BUTTON_ACTION;

bool menuCommandStarted = false;
String menuCommand = "";
bool menuIsShowing = false;

String startupCommands[4] = {STARTUP_COMMAND_1, STARTUP_COMMAND_2, STARTUP_COMMAND_3, STARTUP_COMMAND_4};

String oledText[18] = {"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""};
bool oledTextInvert[18] = {false, false, false, false, false, false, false, false, false, 
                           false, false, false, false, false, false, false, false, false};

int currentSpeed[6];   // set to maximum possible (6)
Direction currentDirection[6];   // set to maximum possible (6)
int speedStepCurrentMultiplier = 1;

TrackPower trackPower = PowerUnknown;
String turnoutPrefix = "";
String routePrefix = "";

// encoder variables
bool circleValues = true;
int encoderValue = 0;
int lastEncoderValue = 0;

// throttle pot values
bool useRotaryEncoderForThrottle = USE_ROTARY_ENCODER_FOR_THROTTLE;
int throttlePotPin = THROTTLE_POT_PIN;
bool throttlePotUseNotches = THROTTLE_POT_USE_NOTCHES;
int throttlePotNotchValues[] = THROTTLE_POT_NOTCH_VALUES; 
int throttlePotNotchSpeeds[] = THROTTLE_POT_NOTCH_SPEEDS;
int throttlePotNotch = 0;
int throttlePotTargetSpeed = 0;
int lastThrottlePotValue = 0;
int lastThrottlePotHighValue = 0;  // highest of the most recent
int lastThrottlePotValues[] = {0, 0, 0, 0, 0};

// Note: Rest of the original variables continue here...
// Due to space constraints, I'm showing the key modifications for TFT support

// *********************************************************************************
// TFT-enhanced display functions

void displayUpdateFromWit(int multiThrottleIndex) {
  debug_print("displayUpdateFromWit(): keypadUseType "); debug_print(keypadUseType); 
  debug_print(" multiThrottleIndex: "); debug_println(multiThrottleIndex);
  
  if (keypadUseType == KEYPAD_USE_OPERATION) {
    if (menuIsShowing) {
      refreshOled();
    } else {
      writeOledSpeed();
    }
  }
}

void writeOledSpeed() {
  lastOledScreen = last_oled_screen_speed;
  menuIsShowing = false;
  
  // Enhanced TFT speed display
  if (TFT_ENHANCED_DISPLAY && tftInitialized) {
    writeOledSpeedTFT();
    return;
  }
  
  // Original OLED-compatible display
  clearOledArray();
  
  // Set app name and version
  setAppnameForOled();
  
  // ... rest of original writeOledSpeed function
  writeOledArray(false, false, true, true);
}

void writeOledSpeedTFT() {
  // Enhanced TFT-specific speed display with colors
  if (!tftInitialized) return;
  
  // Clear the main display area
  tftWrapper.clearBuffer();
  
  // Draw throttle number indicator
  if (maxThrottles > 1) {
    tftWrapper.setTextColor(TFT_THEME_HIGHLIGHT, TFT_THEME_BACKGROUND);
    tftWrapper.setCursor(THROTTLE_NUM_X, THROTTLE_NUM_Y);
    tft.setTextSize(2);
    tft.print("T");
    tft.print(currentThrottleIndex + 1);
  }
  
  // Draw current speed with large, colored text
  String speedStr = String(currentSpeed[currentThrottleIndex]);
  if (speedDisplayAsPercent) {
    speedStr = String((currentSpeed[currentThrottleIndex] * 100) / 126) + "%";
  } else if (speedDisplayAs0to28) {
    speedStr = String((currentSpeed[currentThrottleIndex] * 28) / 126);
  }
  
  tftWrapper.setTextColor(TFT_THEME_SPEED, TFT_THEME_BACKGROUND);
  tftWrapper.setCursor(SPEED_X, SPEED_Y);
  tft.setTextSize(3);
  tft.print(speedStr);
  
  // Draw direction indicator
  String directionStr = (currentDirection[currentThrottleIndex] == Forward) ? "FWD" : "REV";
  uint16_t dirColor = (currentDirection[currentThrottleIndex] == Forward) ? TFT_THEME_DIRECTION : TFT_ORANGE;
  
  tftWrapper.setTextColor(dirColor, TFT_THEME_BACKGROUND);
  tftWrapper.setCursor(DIRECTION_X, DIRECTION_Y);
  tft.setTextSize(2);
  tft.print(directionStr);
  
  // Draw function indicators
  writeOledFunctionsTFT();
  
  // Draw battery indicator
  writeOledBatteryTFT();
  
  // Draw loco consist info
  writeOledLocoInfoTFT();
  
  // Draw menu text if needed
  if (broadcastMessageText != "") {
    tftWrapper.setTextColor(TFT_THEME_ERROR, TFT_THEME_BACKGROUND);
    tftWrapper.setCursor(0, MENU_Y);
    tft.setTextSize(1);
    tft.print(broadcastMessageText);
  } else {
    setMenuTextForOled(hashShowsFunctionsInsteadOfKeyDefs ? menu_menu_hash_is_functions : menu_menu);
    tftWrapper.setTextColor(TFT_THEME_MENU, TFT_THEME_BACKGROUND);
    tftWrapper.setCursor(0, MENU_Y);
    tft.setTextSize(1);
    tft.print(oledText[5]);
  }
}

void writeOledFunctionsTFT() {
  // Draw function indicators in a colored grid
  int x = 0;
  int y = FUNCTIONS_Y;
  
  tftWrapper.setTextColor(TFT_THEME_FUNCTIONS, TFT_THEME_BACKGROUND);
  tft.setTextSize(1);
  
  for (int i = 0; i < 32 && i < MAX_FUNCTIONS; i++) {
    if (i > 0 && i % 16 == 0) {
      x = 0;
      y += CHAR_HEIGHT_SMALL;
    }
    
    if (functionStates[currentThrottleIndex][i]) {
      tftWrapper.setCursor(x, y);
      if (i < 10) {
        tft.print(i);
      } else {
        // Use letters for functions 10+
        char funcChar = 'A' + (i - 10);
        if (funcChar <= 'Z') {
          tft.print(funcChar);
        } else {
          tft.print("?");
        }
      }
    }
    
    x += CHAR_WIDTH_SMALL;
  }
}

void writeOledBatteryTFT() {
  if (!useBatteryTest || lastBatteryCheckTime <= 0) return;
  
  int batteryPercent = batteryTestValue;
  
  // Determine battery color based on level
  uint16_t batteryColor;
  if (batteryPercent > 50) {
    batteryColor = TFT_THEME_BATTERY;
  } else if (batteryPercent > 20) {
    batteryColor = TFT_YELLOW;
  } else {
    batteryColor = TFT_THEME_ERROR;
  }
  
  // Draw battery outline
  tft.drawRect(BATTERY_X, BATTERY_Y, BATTERY_WIDTH-2, BATTERY_HEIGHT, batteryColor);
  tft.drawRect(BATTERY_X + BATTERY_WIDTH - 2, BATTERY_Y + 2, 2, BATTERY_HEIGHT - 4, batteryColor);
  
  // Fill battery based on percentage
  int fillWidth = ((BATTERY_WIDTH - 4) * batteryPercent) / 100;
  if (fillWidth > 0) {
    tft.fillRect(BATTERY_X + 1, BATTERY_Y + 1, fillWidth, BATTERY_HEIGHT - 2, batteryColor);
  }
  
  // Show percentage if enabled
  if (TFT_SHOW_BATTERY_PERCENT && showBatteryTest == ICON_AND_PERCENT) {
    tftWrapper.setTextColor(batteryColor, TFT_THEME_BACKGROUND);
    tftWrapper.setCursor(BATTERY_X, BATTERY_Y + BATTERY_HEIGHT + 2);
    tft.setTextSize(1);
    tft.print(batteryPercent);
    tft.print("%");
  }
}

void writeOledLocoInfoTFT() {
  // Display consist information with enhanced formatting
  String locoText = "";
  int locoCount = 0;
  
  for (int i = 0; i < MAX_LOCOS; i++) {
    if (locoThrottles[currentThrottleIndex][i] != NULL) {
      if (locoCount > 0) locoText += CONSIST_SPACE_BETWEEN_LOCOS;
      
      #ifdef DISPLAY_LOCO_NAME
      if (strlen(locoThrottles[currentThrottleIndex][i]->getName()) > 0) {
        locoText += locoThrottles[currentThrottleIndex][i]->getName();
      } else {
        locoText += String(locoThrottles[currentThrottleIndex][i]->getAddress());
      }
      #else
      locoText += String(locoThrottles[currentThrottleIndex][i]->getAddress());
      #endif
      
      if (locoThrottles[currentThrottleIndex][i]->getDirection() == Reverse) {
        locoText += DIRECTION_REVERSE_INDICATOR;
      }
      locoCount++;
    }
  }
  
  if (locoText.length() > 0) {
    tftWrapper.setTextColor(TFT_THEME_TEXT, TFT_THEME_BACKGROUND);
    tftWrapper.setCursor(2, 2);
    tft.setTextSize(1);
    tft.print(locoText);
  } else {
    tftWrapper.setTextColor(TFT_THEME_ERROR, TFT_THEME_BACKGROUND);
    tftWrapper.setCursor(2, 2);
    tft.setTextSize(1);
    tft.print(MSG_NO_LOCO_SELECTED);
  }
}

// *********************************************************************************
// Setup and initialization

void setup() {
  Serial.begin(115200);
  
  // Initialize TFT display
  debug_println("Initializing TFT display...");
  tftWrapper.begin();
  tftInitialized = true;
  
  // Show startup message
  tft.setTextColor(TFT_THEME_TEXT, TFT_THEME_BACKGROUND);
  tft.setCursor(10, 30);
  tft.setTextSize(2);
  tft.print("WiTcontroller");
  tft.setCursor(10, 50);
  tft.setTextSize(1);
  tft.print("TFT Edition");
  tft.setCursor(10, 70);
  tft.print("Starting...");
  
  delay(2000);
  
  // Initialize rotary encoder
  debug_println("Initializing rotary encoder...");
  rotaryEncoder.begin();
  rotaryEncoder.setup(readEncoderISR);
  
  // Initialize keypad
  debug_println("Initializing keypad...");
  // Keypad initialization code here
  
  // Initialize WiFi and other components
  debug_println("Setup complete. Starting main loop...");
  
  // Clear display and start normal operation
  tftWrapper.clearBuffer();
  setAppnameForOled();
  writeOledSpeed();
}

// *********************************************************************************
// Main loop - rest remains largely the same as original

void loop() {
  // Main loop implementation follows the original pattern
  // but uses enhanced TFT display functions when appropriate
  
  // Handle encoder rotation
  if (rotaryEncoder.encoderChanged()) {
    // Handle speed changes with enhanced display
    encoderValue = rotaryEncoder.readEncoder();
    
    if (keypadUseType == KEYPAD_USE_OPERATION) {
      if (!menuIsShowing) {
        // Speed control logic
        int change = encoderValue - lastEncoderValue;
        if (change != 0) {
          changeSpeed(currentThrottleIndex, change * speedStep * speedStepCurrentMultiplier);
          lastEncoderValue = encoderValue;
          
          // Update TFT display immediately for smooth response
          if (TFT_ENHANCED_DISPLAY && tftInitialized) {
            writeOledSpeedTFT();
          } else {
            displayUpdateFromWit(currentThrottleIndex);
          }
        }
      }
    }
  }
  
  // Handle keypad input
  char key = keypad.getKey();
  if (key) {
    handleKeypress(key);
  }
  
  // Handle encoder button press
  if (rotaryEncoder.isEncoderButtonClicked()) {
    handleEncoderButtonPress();
  }
  
  // Handle WiThrottle protocol
  if (wiThrottleProtocol.check()) {
    displayUpdateFromWit(-1);
  }
  
  // Other periodic tasks...
}

// Helper functions and remaining implementation would continue here...
// This template shows the key modifications needed for TFT support