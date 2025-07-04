#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include <String.h>
#include "static.h"
#include "config_keypad_etc.h"

// Constantes de estado de conexión
#define CONNECTION_STATE_DISCONNECTED 0
#define CONNECTION_STATE_SELECTED 1
#define CONNECTION_STATE_PASSWORD_ENTRY 2
#define CONNECTION_STATE_CONNECTING 3
#define CONNECTION_STATE_CONNECTED 4

// Fuentes de selección de SSID
#define SSID_CONNECTION_SOURCE_LIST 0
#define SSID_CONNECTION_SOURCE_BROWSE 1

// Variables globales WiFi
extern String selectedSsid;
extern String selectedSsidPassword;
extern int ssidConnectionState;
extern int ssidSelectionSource;
extern double startWaitForSelection;

// Variables de entrada de contraseña SSID
extern String ssidPasswordEntered;
extern bool ssidPasswordChanged;
extern char ssidPasswordCurrentChar;

// Variables de SSIDs encontrados
extern String foundSsids[];
extern long foundSsidRssis[];
extern bool foundSsidsOpen[];
extern int foundSsidsCount;

// Funciones principales del módulo WiFi
void wifi_init();
void ssidsLoop();
void browseSsids();
void selectSsidFromFound(int selection);
void getSsidPasswordAndWitIpForFound();
void enterSsidPassword();
void showListOfSsids();
void selectSsid(int selection);
void connectSsid();

// Funciones auxiliares para entrada de contraseña
void ssidPasswordAddChar(char key);
void ssidPasswordDeleteChar(char key);

// Funciones de gestión de estado
bool isWifiConnected();
void disconnectWifi();
void resetWifiState();

#endif // WIFI_MANAGER_H