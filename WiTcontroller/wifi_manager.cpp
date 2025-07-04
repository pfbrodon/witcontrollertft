#include "wifi_manager.h"
#include "WiTcontroller.h"
#include "config_network.h"

// Variables globales WiFi
String selectedSsid = "";
String selectedSsidPassword = "";
int ssidConnectionState = CONNECTION_STATE_DISCONNECTED;
int ssidSelectionSource = SSID_CONNECTION_SOURCE_BROWSE;
double startWaitForSelection = 0;

// Variables de entrada de contraseña SSID
String ssidPasswordEntered = "";
bool ssidPasswordChanged = true;
char ssidPasswordCurrentChar = ssidPasswordBlankChar;

// Variables de SSIDs encontrados
String foundSsids[maxFoundSsids];
long foundSsidRssis[maxFoundSsids];
bool foundSsidsOpen[maxFoundSsids];
int foundSsidsCount = 0;

// Declaraciones de funciones externas del archivo principal
extern void clearOledArray();
extern void setAppnameForOled();
extern void writeOledBattery();
extern void writeOledArray(bool invert = false, bool force = false, bool waitForHalfSecond = false, bool waitForFullSecond = false);
extern void writeOledFoundSSids(String extraText);
extern void setMenuTextForOled(String menuText);
extern void writeOledEnterPassword();
extern String getDots(int numDots);

// Variables externas del archivo principal
extern String oledText[];
extern int keypadUseType;
extern int encoderUseType;
extern String turnoutPrefix;
extern String routePrefix;
extern String witServerIpAndPortEntered;
extern bool autoConnectToFirstDefinedServer;
extern bool commandsNeedLeadingCrLf;

void wifi_init() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    ssidConnectionState = CONNECTION_STATE_DISCONNECTED;
    foundSsidsCount = 0;
}

void ssidsLoop() {
    if (ssidConnectionState == CONNECTION_STATE_DISCONNECTED) {
        if (ssidSelectionSource == SSID_CONNECTION_SOURCE_LIST) {
            showListOfSsids(); 
        } else {
            browseSsids();
        }
    }
    
    if (ssidConnectionState == CONNECTION_STATE_PASSWORD_ENTRY) {
        enterSsidPassword();
    }

    if (ssidConnectionState == CONNECTION_STATE_SELECTED) {
        connectSsid();
    }
}

void browseSsids() {
    debug_println("browseSsids()");

    double startTime = millis();
    double nowTime = startTime;

    debug_println("Browsing for ssids");
    clearOledArray(); 
    setAppnameForOled();
    oledText[2] = MSG_BROWSING_FOR_SSIDS;
    writeOledBattery();
    writeOledArray(false, false, true, true);

    WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
    WiFi.setSortMethod(WIFI_CONNECT_AP_BY_SIGNAL);
    
    int numSsids = WiFi.scanNetworks();
    while ((numSsids == -1) && ((nowTime-startTime) <= 10000)) {
        delay(250);
        debug_print(".");
        nowTime = millis();
    }

    startWaitForSelection = millis();
    foundSsidsCount = 0;
    
    if (numSsids == -1) {
        debug_println("Couldn't get a wifi connection");
    } else {
        for (int thisSsid = 0; thisSsid < numSsids; thisSsid++) {
            // Remove duplicates (repeaters and mesh networks)
            bool found = false;
            for (int i = 0; i < foundSsidsCount && i < maxFoundSsids; i++) {
                if (WiFi.SSID(thisSsid) == foundSsids[i]) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                foundSsids[foundSsidsCount] = WiFi.SSID(thisSsid);
                foundSsidRssis[foundSsidsCount] = WiFi.RSSI(thisSsid);
                foundSsidsOpen[foundSsidsCount] = (WiFi.encryptionType(thisSsid) == 7) ? true : false;
                foundSsidsCount++;
            }
        }
        
        for (int i = 0; i < foundSsidsCount; i++) {
            debug_println(foundSsids[i]);      
        }

        clearOledArray(); 
        oledText[10] = MSG_SSIDS_FOUND;
        writeOledFoundSSids("");
        setMenuTextForOled(menu_select_ssids_from_found);
        writeOledArray(false, false);

        keypadUseType = KEYPAD_USE_SELECT_SSID_FROM_FOUND;
        ssidConnectionState = CONNECTION_STATE_SELECTION_REQUIRED;

        if ((foundSsidsCount > 0) && (autoConnectToFirstDefinedServer)) {
            for (int i = 0; i < foundSsidsCount; i++) { 
                if (foundSsids[i] == ssids[0]) {
                    ssidConnectionState = CONNECTION_STATE_SELECTED;
                    selectedSsid = foundSsids[i];
                    getSsidPasswordAndWitIpForFound();
                }
            }
        }
    }
}

void selectSsidFromFound(int selection) {
    debug_print("selectSsid() "); debug_println(selection);

    if ((selection >= 0) && (selection < maxFoundSsids)) {
        ssidConnectionState = CONNECTION_STATE_SELECTED;
        selectedSsid = foundSsids[selection];
        getSsidPasswordAndWitIpForFound();
    }
    if (selectedSsidPassword == "") {
        ssidConnectionState = CONNECTION_STATE_PASSWORD_ENTRY;
    }
}

void getSsidPasswordAndWitIpForFound() {
    bool found = false;

    selectedSsidPassword = "";
    turnoutPrefix = "";
    routePrefix = "";

    for (int i = 0; i < maxSsids; ++i) {
        if (selectedSsid == ssids[i]) {
            selectedSsidPassword = passwords[i];
            turnoutPrefix = turnoutPrefixes[i];
            routePrefix = routePrefixes[i];
            found = true;
            debug_println("getSsidPasswordAndWitIpForFound() Using configured password");
            break;
        }
    }

    if (!found) {
        if ((selectedSsid.substring(0, 6) == "DCCEX_") && (selectedSsid.length() == 12)) {
            selectedSsidPassword = "PASS_" + selectedSsid.substring(6);
            witServerIpAndPortEntered = "19216800400102560";
            turnoutPrefix = DCC_EX_TURNOUT_PREFIX;
            routePrefix = DCC_EX_ROUTE_PREFIX;
            debug_println("getSsidPasswordAndWitIpForFound() Using guessed DCC-EX password");
        } 
    }
}

void enterSsidPassword() {
    keypadUseType = KEYPAD_USE_ENTER_SSID_PASSWORD;
    encoderUseType = ENCODER_USE_SSID_PASSWORD;
    if (ssidPasswordChanged) {
        debug_println("enterSsidPassword()");
        writeOledEnterPassword();
        ssidPasswordChanged = false;
    }
}

void showListOfSsids() {
    debug_println("showListOfSsids()");
    startWaitForSelection = millis();

    clearOledArray(); 
    setAppnameForOled(); 
    writeOledBattery();
    writeOledArray(false, false);

    if (maxSsids == 0) {
        oledText[1] = MSG_NO_SSIDS_FOUND;
        writeOledBattery();
        writeOledArray(false, false, true, true);
        debug_println(oledText[1]);
    } else {
        debug_print(maxSsids); debug_println(MSG_SSIDS_LISTED);
        clearOledArray(); 
        oledText[10] = MSG_SSIDS_LISTED;

        for (int i = 0; i < maxSsids; ++i) {
            debug_print(i+1); debug_print(": "); debug_println(ssids[i]);
            int j = i;
            if (i >= 5) { 
                j = i + 1;
            } 
            if (i <= 10) {
                oledText[j] = String(i) + ": ";
                if (ssids[i].length() < 9) {
                    oledText[j] = oledText[j] + ssids[i];
                } else {
                    oledText[j] = oledText[j] + ssids[i].substring(0, 9) + "..";
                }
            }
        }

        if (maxSsids > 0) {
            setMenuTextForOled(menu_select_ssids);
        }
        writeOledArray(false, false);

        if (maxSsids == 1) {
            selectedSsid = ssids[0];
            selectedSsidPassword = passwords[0];
            ssidConnectionState = CONNECTION_STATE_SELECTED;
            turnoutPrefix = turnoutPrefixes[0];
            routePrefix = routePrefixes[0];
        } else {
            ssidConnectionState = CONNECTION_STATE_SELECTION_REQUIRED;
            keypadUseType = KEYPAD_USE_SELECT_SSID;
        }
    }
}

void selectSsid(int selection) {
    debug_print("selectSsid() "); debug_println(selection);

    if ((selection >= 0) && (selection < maxSsids)) {
        ssidConnectionState = CONNECTION_STATE_SELECTED;
        selectedSsid = ssids[selection];
        selectedSsidPassword = passwords[selection];
        turnoutPrefix = turnoutPrefixes[selection];
        routePrefix = routePrefixes[selection];
    }
}

void connectSsid() {
    debug_println("Connecting to ssid...");
    clearOledArray(); 
    setAppnameForOled();
    oledText[1] = selectedSsid; 
    oledText[2] = "connecting...";
    writeOledBattery();
    writeOledArray(false, false, true, true);

    double startTime = millis();
    double nowTime = startTime;

    const char *cSsid = selectedSsid.c_str();
    const char *cPassword = selectedSsidPassword.c_str();

    if (selectedSsid.length() > 0) {
        debug_print("Trying Network "); debug_println(cSsid);
        clearOledArray(); 
        setAppnameForOled(); 
        
        for (int i = 0; i < 3; ++i) {
            oledText[1] = selectedSsid; 
            oledText[2] = String(MSG_TRYING_TO_CONNECT) + " (" + String(i) + ")";
            writeOledBattery();
            writeOledArray(false, false, true, true);

            nowTime = startTime;
            debug_print("hostname "); debug_println(WiFi.getHostname());
            WiFi.begin(cSsid, cPassword); 

            int j = 0;
            int tempTimer = millis();
            debug_print("Trying Network ... Checking status "); 
            debug_print(cSsid); debug_print(" :"); debug_print(cPassword); debug_println(":");
            
            while ((WiFi.status() != WL_CONNECTED) && ((nowTime-startTime) <= SSID_CONNECTION_TIMEOUT)) {
                if (millis() > tempTimer + 250) {
                    oledText[3] = getDots(j);
                    writeOledBattery();
                    writeOledArray(false, false, true, true);
                    j++;
                    debug_print(".");
                    tempTimer = millis();
                }
                nowTime = millis();
            }

            if (WiFi.status() == WL_CONNECTED) {
                if (!commandsNeedLeadingCrLf) { 
                    debug_println("Leading CRLF will not be sent for commands"); 
                }
                break; 
            } else {
                debug_println("");
            }
        }

        debug_println("");
        if (WiFi.status() == WL_CONNECTED) {
            debug_print("Connected. IP address: "); debug_println(WiFi.localIP());
            oledText[2] = MSG_CONNECTED; 
            oledText[3] = MSG_ADDRESS_LABEL + String(WiFi.localIP());
            writeOledBattery();
            writeOledArray(false, false, true, true);
            ssidConnectionState = CONNECTION_STATE_CONNECTED;
            keypadUseType = KEYPAD_USE_SELECT_WITHROTTLE_SERVER;

            // Setup mDNS responder
            if (!MDNS.begin("WiTcontroller")) {
                debug_println("Error setting up MDNS responder!");
                oledText[2] = MSG_BOUNJOUR_SETUP_FAILED;
                writeOledBattery();
                writeOledArray(false, false, true, true);
            }
        } else {
            debug_println("Connection failed");
            oledText[2] = MSG_CONNECTION_FAILED;
            writeOledBattery();
            writeOledArray(false, false, true, true);
            ssidConnectionState = CONNECTION_STATE_DISCONNECTED;
        }
    }
}

void ssidPasswordAddChar(char key) {
    debug_print("ssidPasswordAddChar() "); debug_println(key);
    if (ssidPasswordEntered.length() < 63) {
        ssidPasswordEntered = ssidPasswordEntered + String(key);
        ssidPasswordChanged = true;
    }
}

void ssidPasswordDeleteChar(char key) {
    debug_println("ssidPasswordDeleteChar()");
    if (ssidPasswordEntered.length() > 0) {
        ssidPasswordEntered = ssidPasswordEntered.substring(0, ssidPasswordEntered.length() - 1);
        ssidPasswordChanged = true;
    }
}

bool isWifiConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void disconnectWifi() {
    WiFi.disconnect();
    ssidConnectionState = CONNECTION_STATE_DISCONNECTED;
}

void resetWifiState() {
    ssidConnectionState = CONNECTION_STATE_DISCONNECTED;
    selectedSsid = "";
    selectedSsidPassword = "";
    ssidPasswordEntered = "";
    foundSsidsCount = 0;
}