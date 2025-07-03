// ********************************************************************************************
//
// config_network.h - Network Configuration for WiTcontroller TFT
// Copy this file to config_network.h and customize with your WiFi networks
//
// ********************************************************************************************

// wifi networks known to the controller
// you can list up to 5 different networks
// the device will attempt to connect to any one of these
// if none are available it will try the ones that are found

const String knownSsids[maxSsidCount] = {
  "YourWiFiNetwork",        // Replace with your WiFi network name
  "YourSecondNetwork",      // Optional second network
  "YourMobileHotspot",      // Optional mobile hotspot
  "",                       // Leave empty if not needed
  ""                        // Leave empty if not needed
};

const String knownPasswords[maxSsidCount] = {
  "YourWiFiPassword",       // Replace with your WiFi password
  "YourSecondPassword",     // Password for second network
  "YourHotspotPassword",    // Password for mobile hotspot  
  "",                       // Leave empty if not needed
  ""                        // Leave empty if not needed
};

// ********************************************************************************************
// WiThrottle Servers
// You can list up to 5 servers here

const String knownWitServers[maxWitServerCount] = {
  "",                       // e.g. "192.168.1.100:12090" for JMRI
  "",                       // e.g. "192.168.1.200:2560" for DCC-EX
  "",                       // Leave empty if not needed
  "",                       // Leave empty if not needed
  ""                        // Leave empty if not needed
};

// ********************************************************************************************
// Optional settings for TFT version

// Automatically connect to first available server
#define AUTO_CONNECT_TO_FIRST_WITHROTTLE_SERVER true

// Automatically connect to first defined server above
// #define AUTO_CONNECT_TO_FIRST_DEFINED_SERVER false

// Minimum delay between outbound commands (milliseconds)
// #define OUTBOUND_COMMANDS_MINIMUM_DELAY 50

// Add leading CR/LF to commands for some servers
// #define SEND_LEADING_CR_LF_FOR_COMMANDS true