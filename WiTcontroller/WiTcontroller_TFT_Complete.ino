/**
 * WiTcontroller TFT Version - Complete Implementation
 * This app turns an ESP32 into a dedicated Model Railway Controller for use
 * with JMRI or any wiThrottle server, using TFT ST7735 display instead of OLED.
 *
 * Based on original WiTcontroller by flash62au (Peter A)
 * TFT adaptation maintains full compatibility while adding color display
 *
 * Instructions:
 * - Refer to the readme at https://github.com/flash62au/WiTcontroller
 * - TFT-specific instructions in README_TFT.md
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

// TFT libraries instead of U8g2
#include <Adafruit_GFX.h>         // Core graphics library
#include <Adafruit_ST7735.h>      // Hardware-specific library for ST7735
#include <SPI.h>

#include <WiThrottleProtocol.h>   // https://github.com/flash62au/WiThrottleProtocol                           Creative Commons 4.0  Attribution-ShareAlike
#include <AiEsp32RotaryEncoder.h> // https://github.com/igorantolic/ai-esp32-rotary-encoder                    GPL 2.0

// this library is included with the WiTController code
#include "Pangodream_18650_CL.h"  // https://github.com/pangodream/18650CL                                     Copyright (c) 2019 Pangodream

// TFT-specific configuration files
#include "config_tft.h"           // TFT display configuration and wrapper

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
// TFT-specific variables and initialization

bool tftInitialized = false;
uint16_t lastSpeedDisplayed[6] = {999, 999, 999, 999, 999, 999};  // Force initial update
String lastDirectionDisplayed[6] = {"", "", "", "", "", ""};
bool functionsChanged = true;
int lastBatteryPercentDisplayed = -1;
bool lastTrackPowerDisplayed = false;
String lastLocoDisplayed[6] = {"", "", "", "", "", ""};

// Create TFT instance - definition from config_tft.h
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS_PIN, TFT_DC_PIN, TFT_RST_PIN);

// *********************************************************************************
// All original variables from WiTcontroller.ino

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
int lastThrottlePotReadTime = -1;

// battery test values
bool useBatteryTest = USE_BATTERY_TEST;
#if USE_BATTERY_TEST
  #if USE_BATTERY_PERCENT_AS_WELL_AS_ICON
    ShowBattery showBatteryTest = ICON_AND_PERCENT;
  #else 
    ShowBattery showBatteryTest = ICON_ONLY;
  #endif
#else
  ShowBattery showBatteryTest = NONE;
#endif
bool useBatteryPercentAsWellAsIcon = USE_BATTERY_PERCENT_AS_WELL_AS_ICON;
int lastBatteryTestValue = 100; 
int batteryTestValue = 100;
int lastBatteryAnalogReadValue = 0;
double lastBatteryCheckTime = -10000;
#if USE_BATTERY_TEST
  Pangodream_18650_CL BL(BATTERY_TEST_PIN,BATTERY_CONVERSION_FACTOR);
#endif

// server variables
String selectedSsid = "";
String selectedSsidPassword = "";
int ssidConnectionState = CONNECTION_STATE_DISCONNECTED;

// ssid password entry
String ssidPasswordEntered = "";
bool ssidPasswordChanged = true;
char ssidPasswordCurrentChar = ssidPasswordBlankChar; 

IPAddress selectedWitServerIP;
int selectedWitServerPort = 0;
String selectedWitServerName ="";
int noOfWitServices = 0;
int witConnectionState = CONNECTION_STATE_DISCONNECTED;
String serverType = "";

//found wiThrottle servers
IPAddress foundWitServersIPs[maxFoundWitServers];
int foundWitServersPorts[maxFoundWitServers];
String foundWitServersNames[maxFoundWitServers];
int foundWitServersCount = 0;
bool autoConnectToFirstDefinedServer = AUTO_CONNECT_TO_FIRST_DEFINED_SERVER;
bool autoConnectToFirstWiThrottleServer = AUTO_CONNECT_TO_FIRST_WITHROTTLE_SERVER;
int outboundCmdsMininumDelay = OUTBOUND_COMMANDS_MINIMUM_DELAY;
bool commandsNeedLeadingCrLf = SEND_LEADING_CR_LF_FOR_COMMANDS;

//found ssids
String foundSsids[maxFoundSsids];
long foundSsidRssis[maxFoundSsids];
bool foundSsidsOpen[maxFoundSsids];
int foundSsidsCount = 0;
int ssidSelectionSource;
double startWaitForSelection;

// wit Server ip entry
String witServerIpAndPortConstructed = "###.###.###.###:#####";
String witServerIpAndPortEntered = DEFAULT_IP_AND_PORT;
bool witServerIpAndPortChanged = true;

// roster variables
int rosterSize = 0;
int rosterIndex[maxRoster]; 
String rosterName[maxRoster]; 
int rosterAddress[maxRoster];
char rosterLength[maxRoster];
char rosterSortStrings[maxRoster][14]; 
char* rosterSortPointers[maxRoster]; 
int rosterSortedIndex[maxRoster]; 

int page = 0;
int functionPage = 0;
bool functionHasBeenSelected = false;

// Broadcast message
String broadcastMessageText = "";
long broadcastMessageTime = 0;
long lastReceivingServerDetailsTime = 0;

// remember display state  
int lastOledScreen = 0;
String lastOledStringParameter = "";
int lastOledIntParameter = 0;
bool lastOledBoolParameter = false;
TurnoutAction lastOledTurnoutParameter = TurnoutToggle;

// turnout variables
int turnoutListSize = 0;
int turnoutListIndex[maxTurnoutList]; 
String turnoutListSysName[maxTurnoutList]; 
String turnoutListUserName[maxTurnoutList];
int turnoutListState[maxTurnoutList];

// route variables
int routeListSize = 0;
int routeListIndex[maxRouteList]; 
String routeListSysName[maxRouteList]; 
String routeListUserName[maxRouteList];
int routeListState[maxRouteList];

// function states
bool functionStates[6][MAX_FUNCTIONS];   // set to maximum possible (6 throttles)

// function labels
String functionLabels[6][MAX_FUNCTIONS];   // set to maximum possible (6 throttles)

// consist function follow
int functionFollow[6][MAX_FUNCTIONS];   // set to maximum possible (6 throttles)

// speedstep
int currentSpeedStep[6];   // set to maximum possible (6 throttles)

// throttle
int currentThrottleIndex = 0;
char currentThrottleIndexChar = '0';
int maxThrottles = MAX_THROTTLES;

int heartBeatPeriod = 10; // default to 10 seconds
long lastServerResponseTime;  // seconds since start of Arduino
bool heartbeatCheckEnabled = HEARTBEAT_ENABLED;

// used to stop speed bounces
long lastSpeedSentTime = 0;
int lastSpeedSent = 0;
int lastSpeedThrottleIndex = 0;

bool dropBeforeAcquire = DROP_BEFORE_ACQUIRE;

// device configuration
const char* deviceName = DEVICE_NAME;

static unsigned long rotaryEncoderButtonLastTimePressed = 0;
const int rotaryEncoderButtonEncoderDebounceTime = ROTARY_ENCODER_DEBOUNCE_TIME;

const bool encoderRotationClockwiseIsIncreaseSpeed = ENCODER_ROTATION_CLOCKWISE_IS_INCREASE_SPEED;
const bool toggleDirectionOnEncoderButtonPressWhenStationary = TOGGLE_DIRECTION_ON_ENCODER_BUTTON_PRESSED_WHEN_STATIONAY;

// keypad button actions
int buttonActions[14] = { CHOSEN_KEYPAD_0_FUNCTION,
                          CHOSEN_KEYPAD_1_FUNCTION,
                          CHOSEN_KEYPAD_2_FUNCTION,
                          CHOSEN_KEYPAD_3_FUNCTION,
                          CHOSEN_KEYPAD_4_FUNCTION,
                          CHOSEN_KEYPAD_5_FUNCTION,
                          CHOSEN_KEYPAD_6_FUNCTION,
                          CHOSEN_KEYPAD_7_FUNCTION,
                          CHOSEN_KEYPAD_8_FUNCTION,
                          CHOSEN_KEYPAD_9_FUNCTION,
                          CHOSEN_KEYPAD_A_FUNCTION,
                          CHOSEN_KEYPAD_B_FUNCTION,
                          CHOSEN_KEYPAD_C_FUNCTION,
                          CHOSEN_KEYPAD_D_FUNCTION
};

// text that will appear when you press #
const String directCommandText[4][3] = {
    {CHOSEN_KEYPAD_1_DISPLAY_NAME, CHOSEN_KEYPAD_2_DISPLAY_NAME, CHOSEN_KEYPAD_3_DISPLAY_NAME},
    {CHOSEN_KEYPAD_4_DISPLAY_NAME, CHOSEN_KEYPAD_5_DISPLAY_NAME, CHOSEN_KEYPAD_6_DISPLAY_NAME},
    {CHOSEN_KEYPAD_7_DISPLAY_NAME, CHOSEN_KEYPAD_8_DISPLAY_NAME, CHOSEN_KEYPAD_9_DISPLAY_NAME},
    {CHOSEN_KEYPAD_DISPLAY_PREFIX, CHOSEN_KEYPAD_0_DISPLAY_NAME, CHOSEN_KEYPAD_DISPLAY_SUFIX}
};

bool oledDirectCommandsAreBeingDisplayed = false;
#ifdef HASH_SHOWS_FUNCTIONS_INSTEAD_OF_KEY_DEFS
  bool hashShowsFunctionsInsteadOfKeyDefs = HASH_SHOWS_FUNCTIONS_INSTEAD_OF_KEY_DEFS;
#else
  bool hashShowsFunctionsInsteadOfKeyDefs = false;
#endif

// Additional buttons configuration
#if !USE_NEW_ADDITIONAL_BUTTONS_FORMAT
  int maxAdditionalButtons = MAX_ADDITIONAL_BUTTONS;
  int additionalButtonPin[MAX_ADDITIONAL_BUTTONS] =          ADDITIONAL_BUTTONS_PINS;
  int additionalButtonType[MAX_ADDITIONAL_BUTTONS] =         ADDITIONAL_BUTTONS_TYPE;
  int additionalButtonActions[MAX_ADDITIONAL_BUTTONS] = {
                            CHOSEN_ADDITIONAL_BUTTON_0_FUNCTION,
                            CHOSEN_ADDITIONAL_BUTTON_1_FUNCTION,
                            CHOSEN_ADDITIONAL_BUTTON_2_FUNCTION,
                            CHOSEN_ADDITIONAL_BUTTON_3_FUNCTION,
                            CHOSEN_ADDITIONAL_BUTTON_4_FUNCTION,
                            CHOSEN_ADDITIONAL_BUTTON_5_FUNCTION,
                            CHOSEN_ADDITIONAL_BUTTON_6_FUNCTION
  };
  int additionalButtonLatching[MAX_ADDITIONAL_BUTTONS] = {
                            ADDITIONAL_BUTTON_0_LATCHING,
                            ADDITIONAL_BUTTON_1_LATCHING,
                            ADDITIONAL_BUTTON_2_LATCHING,
                            ADDITIONAL_BUTTON_3_LATCHING,
                            ADDITIONAL_BUTTON_4_LATCHING,
                            ADDITIONAL_BUTTON_5_LATCHING,
                            ADDITIONAL_BUTTON_6_LATCHING
  };
  unsigned long lastAdditionalButtonDebounceTime[MAX_ADDITIONAL_BUTTONS];
  bool additionalButtonRead[MAX_ADDITIONAL_BUTTONS];
  bool additionalButtonLastRead[MAX_ADDITIONAL_BUTTONS];
#else
  #if NEW_MAX_ADDITIONAL_BUTTONS>0
    int maxAdditionalButtons = NEW_MAX_ADDITIONAL_BUTTONS;
    int additionalButtonPin[NEW_MAX_ADDITIONAL_BUTTONS] = NEW_ADDITIONAL_BUTTON_PIN;
    int additionalButtonType[NEW_MAX_ADDITIONAL_BUTTONS] = NEW_ADDITIONAL_BUTTON_TYPE;
    int additionalButtonActions[NEW_MAX_ADDITIONAL_BUTTONS] = NEW_ADDITIONAL_BUTTON_ACTIONS;
    int additionalButtonLatching[NEW_MAX_ADDITIONAL_BUTTONS] = NEW_ADDITIONAL_BUTTON_LATCHING;
    unsigned long lastAdditionalButtonDebounceTime[NEW_MAX_ADDITIONAL_BUTTONS];
    bool additionalButtonRead[NEW_MAX_ADDITIONAL_BUTTONS];
    bool additionalButtonLastRead[NEW_MAX_ADDITIONAL_BUTTONS];
  #else
    int maxAdditionalButtons = 0;
    int additionalButtonPin[1] = {-1};
    int additionalButtonType[1] = {INPUT_PULLUP};
    int additionalButtonActions[1] = {FUNCTION_NULL};
    int additionalButtonLatching[1] = {false};
    unsigned long lastAdditionalButtonDebounceTime[1];
    bool additionalButtonRead[1];
    bool additionalButtonLastRead[1];
  #endif
#endif

bool additionalButtonOverrideDefaultLatching = ADDITIONAL_BUTTON_OVERRIDE_DEFAULT_LATCHING;
unsigned long additionalButtonDebounceDelay = ADDITIONAL_BUTTON_DEBOUNCE_DELAY;

// *********************************************************************************
// non-volatile storage

Preferences nvsPrefs;
bool nvsInit = false;
bool nvsPrefsSaved = false;
bool preferencesRead = false;