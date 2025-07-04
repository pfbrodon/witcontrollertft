#ifndef WIT_SERVER_H
#define WIT_SERVER_H

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiThrottleProtocol.h>
#include <String.h>
#include "static.h"
#include "config_keypad_etc.h"

// Estados de conexión del servidor WiThrottle
#define WIT_CONNECTION_STATE_DISCONNECTED 0
#define WIT_CONNECTION_STATE_SCANNING 1
#define WIT_CONNECTION_STATE_SELECTION_REQUIRED 2
#define WIT_CONNECTION_STATE_CONNECTING 3
#define WIT_CONNECTION_STATE_CONNECTED 4
#define WIT_CONNECTION_STATE_ENTRY_REQUIRED 5

// Constantes del protocolo
#define WIT_DEFAULT_PORT 12090
#define WIT_MAX_SERVERS 5
#define WIT_HEARTBEAT_INTERVAL 10000
#define WIT_RECONNECT_INTERVAL 5000

// Estructura para servidor WiThrottle encontrado
struct WiTServer {
    IPAddress ip;
    int port;
    String name;
    String type;
    bool isActive;
};

// Variables globales del servidor WiThrottle
extern IPAddress selectedWitServerIP;
extern int selectedWitServerPort;
extern String selectedWitServerName;
extern String serverType;
extern int noOfWitServices;
extern int witConnectionState;

// Variables de servidores encontrados
extern WiTServer foundWitServers[];
extern int foundWitServersCount;
extern bool autoConnectToFirstWiThrottleServer;
extern bool autoConnectToFirstDefinedServer;

// Variables de protocolo
extern int outboundCmdsMininumDelay;
extern bool commandsNeedLeadingCrLf;
extern String witServerIpAndPortEntered;
extern bool witServerIpAndPortChanged;

// Funciones principales del módulo
void wit_server_init();
void wit_server_loop();
void wit_server_reset();

// Funciones de gestión de conexión
void witServiceLoop();
void browseWitService();
void selectWitServer(int selection);
void connectWitServer();
void disconnectWitServer();
void reconnectWitServer();

// Funciones de entrada de servidor manual
void enterWitServer();
void witEntryAddChar(char key);
void witEntryDeleteChar(char key);
void buildWitEntry();
void parseWitServerEntry();

// Funciones del protocolo WiThrottle
void initWiThrottleProtocol();
void processWiThrottleMessages();
void sendWiThrottleCommand(const String& command);
void sendHeartbeat();

// Callbacks del protocolo WiThrottle
void onWiThrottleConnected();
void onWiThrottleDisconnected();
void onTrackPowerChange(TrackPower state);
void onRosterReceived();
void onTurnoutListReceived();
void onRouteListReceived();
void onLocoStateUpdate(int address);
void onDirectionChange(int throttle, Direction direction);
void onSpeedChange(int throttle, int speed);
void onFunctionChange(int throttle, int function, bool state);

// Funciones de gestión de estado
bool wit_server_isConnected();
bool wit_server_isScanning();
void wit_server_setConnectionState(int state);
int wit_server_getConnectionState();

// Funciones de descubrimiento mDNS
void scanForWiThrottleServers();
void processFoundServers();
void addFoundServer(const IPAddress& ip, int port, const String& name, const String& type);

// Funciones de comandos
void wit_server_processPendingCommands();
void wit_server_sendThrottleCommand(int throttle, const String& command);
void wit_server_sendGlobalCommand(const String& command);

// Funciones de utilidad
String formatServerAddress(const IPAddress& ip, int port);
bool isValidServerIP(const String& ip);
bool isValidServerPort(int port);
void parseServerString(const String& serverStr, IPAddress& ip, int& port);

// Funciones de configuración
void wit_server_setAutoConnect(bool enabled);
bool wit_server_getAutoConnect();
void wit_server_setHeartbeatEnabled(bool enabled);
bool wit_server_getHeartbeatEnabled();
void wit_server_setCommandDelay(int delay);
int wit_server_getCommandDelay();

#endif // WIT_SERVER_H