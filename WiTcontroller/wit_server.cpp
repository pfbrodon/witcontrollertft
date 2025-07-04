#include "wit_server.h"
#include "WiTcontroller.h"
#include "display_manager.h"
#include "preferences_manager.h"

// Variables globales del servidor WiThrottle
IPAddress selectedWitServerIP;
int selectedWitServerPort = WIT_DEFAULT_PORT;
String selectedWitServerName = "";
String serverType = "";
int noOfWitServices = 0;
int witConnectionState = WIT_CONNECTION_STATE_DISCONNECTED;

// Variables de servidores encontrados
WiTServer foundWitServers[WIT_MAX_SERVERS];
int foundWitServersCount = 0;
bool autoConnectToFirstWiThrottleServer = AUTO_CONNECT_TO_FIRST_WITHROTTLE_SERVER;
bool autoConnectToFirstDefinedServer = AUTO_CONNECT_TO_FIRST_DEFINED_SERVER;

// Variables de protocolo
int outboundCmdsMininumDelay = OUTBOUND_COMMANDS_MINIMUM_DELAY;
bool commandsNeedLeadingCrLf = SEND_LEADING_CR_LF_FOR_COMMANDS;
String witServerIpAndPortEntered = DEFAULT_IP_AND_PORT;
bool witServerIpAndPortChanged = true;

// Variables internas
WiThrottleProtocol wiThrottleProtocol;
unsigned long lastHeartbeat = 0;
unsigned long lastReconnectAttempt = 0;
bool heartbeatEnabled = true;
Queue<String> pendingCommands;

// Variables externas
extern String oledText[];
extern int keypadUseType;
extern int encoderUseType;

void wit_server_init() {
    debug_println("Initializing WiThrottle server module...");
    
    // Inicializar protocolo WiThrottle
    initWiThrottleProtocol();
    
    // Resetear estado
    wit_server_reset();
    
    // Cargar configuración guardada
    selectedWitServerIP.fromString(getLastServerIP());
    selectedWitServerPort = getLastServerPort();
    
    debug_println("WiThrottle server module initialized");
}

void wit_server_loop() {
    // Procesar según el estado de conexión
    switch (witConnectionState) {
        case WIT_CONNECTION_STATE_DISCONNECTED:
            witServiceLoop();
            break;
        case WIT_CONNECTION_STATE_SCANNING:
            browseWitService();
            break;
        case WIT_CONNECTION_STATE_CONNECTING:
            // Manejar timeout de conexión
            handleConnectionTimeout();
            break;
        case WIT_CONNECTION_STATE_CONNECTED:
            processWiThrottleMessages();
            processPendingCommands();
            sendHeartbeatIfNeeded();
            break;
        case WIT_CONNECTION_STATE_ENTRY_REQUIRED:
            enterWitServer();
            break;
        default:
            break;
    }
    
    // Manejar reconexión automática si es necesario
    handleAutoReconnect();
}

void wit_server_reset() {
    witConnectionState = WIT_CONNECTION_STATE_DISCONNECTED;
    foundWitServersCount = 0;
    noOfWitServices = 0;
    selectedWitServerName = "";
    serverType = "";
    lastHeartbeat = 0;
    lastReconnectAttempt = 0;
    
    // Limpiar lista de servidores encontrados
    for (int i = 0; i < WIT_MAX_SERVERS; i++) {
        foundWitServers[i].isActive = false;
    }
    
    debug_println("WiThrottle server module reset");
}

void witServiceLoop() {
    if (WiFi.status() != WL_CONNECTED) {
        return; // No hacer nada si WiFi no está conectado
    }
    
    // Iniciar escaneo de servidores
    wit_server_setConnectionState(WIT_CONNECTION_STATE_SCANNING);
}

void browseWitService() {
    debug_println("browseWitService()");
    
    double startTime = millis();
    double nowTime = startTime;
    
    clearOledArray();
    setAppnameForOled();
    oledText[2] = MSG_BROWSING_FOR_SERVERS;
    writeOledBattery();
    writeOledArray(false, false, true, true);
    
    // Escanear servicios mDNS
    scanForWiThrottleServers();
    
    // Procesar resultados
    processFoundServers();
    
    if (foundWitServersCount == 0) {
        debug_println("No WiThrottle servers found");
        oledText[2] = MSG_NO_SERVERS_FOUND;
        oledText[3] = MSG_ENTER_SERVER_MANUALLY;
        writeOledArray(false, false, true, true);
        
        wit_server_setConnectionState(WIT_CONNECTION_STATE_ENTRY_REQUIRED);
    } else {
        debug_print("Found ");
        debug_print(foundWitServersCount);
        debug_println(" WiThrottle servers");
        
        // Mostrar servidores encontrados
        clearOledArray();
        oledText[10] = MSG_SERVERS_FOUND;
        
        for (int i = 0; i < foundWitServersCount && i < 5; i++) {
            String serverLine = String(i) + ": ";
            if (foundWitServers[i].name.length() > 0) {
                serverLine += foundWitServers[i].name;
            } else {
                serverLine += formatServerAddress(foundWitServers[i].ip, foundWitServers[i].port);
            }
            oledText[i + 1] = serverLine;
        }
        
        setMenuTextForOled(menu_select_wit_services);
        writeOledArray(false, false);
        
        keypadUseType = KEYPAD_USE_SELECT_WITHROTTLE_SERVER;
        wit_server_setConnectionState(WIT_CONNECTION_STATE_SELECTION_REQUIRED);
        
        // Auto-conectar si está habilitado
        if (autoConnectToFirstWiThrottleServer && foundWitServersCount > 0) {
            selectWitServer(0);
        }
    }
}

void selectWitServer(int selection) {
    debug_print("selectWitServer() ");
    debug_println(selection);
    
    if (selection >= 0 && selection < foundWitServersCount) {
        selectedWitServerIP = foundWitServers[selection].ip;
        selectedWitServerPort = foundWitServers[selection].port;
        selectedWitServerName = foundWitServers[selection].name;
        serverType = foundWitServers[selection].type;
        
        // Guardar en preferencias
        saveServerPreferences(selectedWitServerIP.toString(), selectedWitServerPort);
        
        wit_server_setConnectionState(WIT_CONNECTION_STATE_CONNECTING);
        connectWitServer();
    }
}

void connectWitServer() {
    debug_println("Connecting to WiThrottle server...");
    
    clearOledArray();
    setAppnameForOled();
    oledText[1] = selectedWitServerName.length() > 0 ? selectedWitServerName : formatServerAddress(selectedWitServerIP, selectedWitServerPort);
    oledText[2] = MSG_CONNECTING_TO_SERVER;
    writeOledBattery();
    writeOledArray(false, false, true, true);
    
    // Intentar conexión
    bool connected = wiThrottleProtocol.connect(selectedWitServerIP, selectedWitServerPort);
    
    if (connected) {
        debug_println("Connected to WiThrottle server");
        oledText[2] = MSG_CONNECTED_TO_SERVER;
        writeOledArray(false, false, true, true);
        
        wit_server_setConnectionState(WIT_CONNECTION_STATE_CONNECTED);
        keypadUseType = KEYPAD_USE_OPERATION;
        
        // Configurar callbacks
        setupWiThrottleCallbacks();
        
        // Ejecutar comandos de inicio si los hay
        executeStartupCommands();
        
        onWiThrottleConnected();
    } else {
        debug_println("Failed to connect to WiThrottle server");
        oledText[2] = MSG_CONNECTION_FAILED;
        oledText[3] = MSG_WILL_RETRY;
        writeOledArray(false, false, true, true);
        
        wit_server_setConnectionState(WIT_CONNECTION_STATE_DISCONNECTED);
        lastReconnectAttempt = millis();
    }
}

void disconnectWitServer() {
    debug_println("Disconnecting from WiThrottle server");
    
    if (wit_server_isConnected()) {
        wiThrottleProtocol.disconnect();
        onWiThrottleDisconnected();
    }
    
    wit_server_setConnectionState(WIT_CONNECTION_STATE_DISCONNECTED);
    
    clearOledArray();
    setAppnameForOled();
    oledText[2] = MSG_DISCONNECTED;
    writeOledArray(false, false, true, true);
}

void reconnectWitServer() {
    debug_println("Attempting to reconnect to WiThrottle server");
    
    if (selectedWitServerIP.toString() != "0.0.0.0" && selectedWitServerPort > 0) {
        wit_server_setConnectionState(WIT_CONNECTION_STATE_CONNECTING);
        connectWitServer();
    } else {
        wit_server_setConnectionState(WIT_CONNECTION_STATE_SCANNING);
    }
}

void enterWitServer() {
    keypadUseType = KEYPAD_USE_ENTER_WITHROTTLE_SERVER_IP;
    encoderUseType = ENCODER_USE_WITHROTTLE_SERVER_IP;
    
    if (witServerIpAndPortChanged) {
        debug_println("enterWitServer()");
        writeOledEnterServerIP();
        witServerIpAndPortChanged = false;
    }
}

void witEntryAddChar(char key) {
    debug_print("witEntryAddChar() ");
    debug_println(key);
    
    if (witServerIpAndPortEntered.length() < 21) { // ###.###.###.###:#####
        witServerIpAndPortEntered += String(key);
        buildWitEntry();
        witServerIpAndPortChanged = true;
    }
}

void witEntryDeleteChar(char key) {
    debug_println("witEntryDeleteChar()");
    
    if (witServerIpAndPortEntered.length() > 0) {
        witServerIpAndPortEntered = witServerIpAndPortEntered.substring(0, witServerIpAndPortEntered.length() - 1);
        buildWitEntry();
        witServerIpAndPortChanged = true;
    }
}

void buildWitEntry() {
    String constructed = "";
    int enteredLength = witServerIpAndPortEntered.length();
    
    for (int i = 0; i < 21; i++) {
        if (i < enteredLength) {
            constructed += witServerIpAndPortEntered.charAt(i);
        } else {
            if (i == 3 || i == 7 || i == 11) {
                constructed += ".";
            } else if (i == 15) {
                constructed += ":";
            } else {
                constructed += "#";
            }
        }
    }
    
    witServerIpAndPortConstructed = constructed;
}

void parseWitServerEntry() {
    // Extraer IP y puerto de la entrada
    String ipStr = "";
    String portStr = "";
    bool inPort = false;
    
    for (int i = 0; i < witServerIpAndPortEntered.length(); i++) {
        char c = witServerIpAndPortEntered.charAt(i);
        if (c == ':') {
            inPort = true;
        } else if (inPort) {
            portStr += c;
        } else {
            ipStr += c;
        }
    }
    
    // Validar y asignar
    if (isValidServerIP(ipStr) && isValidServerPort(portStr.toInt())) {
        selectedWitServerIP.fromString(ipStr);
        selectedWitServerPort = portStr.toInt();
        selectedWitServerName = "Manual Entry";
        
        wit_server_setConnectionState(WIT_CONNECTION_STATE_CONNECTING);
        connectWitServer();
    } else {
        debug_println("Invalid server IP or port entered");
        display_showError("Invalid server address");
    }
}

void initWiThrottleProtocol() {
    debug_println("Initializing WiThrottle protocol");
    
    // Configurar protocolo
    wiThrottleProtocol.setCommandDelay(outboundCmdsMininumDelay);
    wiThrottleProtocol.setHeartbeatPeriod(WIT_HEARTBEAT_INTERVAL);
    
    setupWiThrottleCallbacks();
}

void setupWiThrottleCallbacks() {
    // Configurar callbacks del protocolo
    wiThrottleProtocol.setDelegate([](String message) {
        debug_print("WiThrottle received: ");
        debug_println(message);
        
        // Procesar mensaje según tipo
        if (message.startsWith("PPA")) {
            // Track power state
            TrackPower power = (message.charAt(3) == '1') ? PowerOn : PowerOff;
            onTrackPowerChange(power);
        } else if (message.startsWith("RL")) {
            // Roster list
            onRosterReceived();
        } else if (message.startsWith("PTL")) {
            // Turnout list
            onTurnoutListReceived();
        } else if (message.startsWith("PRL")) {
            // Route list
            onRouteListReceived();
        }
        // Agregar más tipos de mensaje según sea necesario
    });
}

void processWiThrottleMessages() {
    if (wiThrottleProtocol.isConnected()) {
        wiThrottleProtocol.check();
    } else {
        // Conexión perdida
        debug_println("WiThrottle connection lost");
        wit_server_setConnectionState(WIT_CONNECTION_STATE_DISCONNECTED);
        onWiThrottleDisconnected();
    }
}

void sendWiThrottleCommand(const String& command) {
    if (wit_server_isConnected()) {
        debug_print("Sending WiThrottle command: ");
        debug_println(command);
        
        String fullCommand = command;
        if (commandsNeedLeadingCrLf) {
            fullCommand = "\r\n" + fullCommand;
        }
        
        wiThrottleProtocol.sendCommand(fullCommand);
    } else {
        debug_println("Cannot send command - not connected to server");
    }
}

void sendHeartbeat() {
    if (heartbeatEnabled && wit_server_isConnected()) {
        sendWiThrottleCommand("*");
        lastHeartbeat = millis();
    }
}

void sendHeartbeatIfNeeded() {
    if (heartbeatEnabled && (millis() - lastHeartbeat > WIT_HEARTBEAT_INTERVAL)) {
        sendHeartbeat();
    }
}

void processPendingCommands() {
    static unsigned long lastCommandSent = 0;
    
    if (!pendingCommands.isEmpty() && (millis() - lastCommandSent > outboundCmdsMininumDelay)) {
        String command = pendingCommands.dequeue();
        sendWiThrottleCommand(command);
        lastCommandSent = millis();
    }
}

void executeStartupCommands() {
    // Ejecutar comandos de inicio configurados
    for (int i = 0; i < 4; i++) {
        if (startupCommands[i].length() > 0) {
            pendingCommands.enqueue(startupCommands[i]);
        }
    }
}

bool wit_server_isConnected() {
    return witConnectionState == WIT_CONNECTION_STATE_CONNECTED && wiThrottleProtocol.isConnected();
}

bool wit_server_isScanning() {
    return witConnectionState == WIT_CONNECTION_STATE_SCANNING;
}

void wit_server_setConnectionState(int state) {
    if (witConnectionState != state) {
        debug_print("WiThrottle connection state changed: ");
        debug_print(witConnectionState);
        debug_print(" -> ");
        debug_println(state);
        
        witConnectionState = state;
    }
}

int wit_server_getConnectionState() {
    return witConnectionState;
}

void scanForWiThrottleServers() {
    debug_println("Scanning for WiThrottle servers via mDNS");
    
    foundWitServersCount = 0;
    
    // Buscar servicios _withrottle._tcp
    int n = MDNS.queryService("withrottle", "tcp");
    
    for (int i = 0; i < n && foundWitServersCount < WIT_MAX_SERVERS; i++) {
        IPAddress ip = MDNS.IP(i);
        int port = MDNS.port(i);
        String name = MDNS.hostname(i);
        
        addFoundServer(ip, port, name, "withrottle");
    }
    
    // También buscar servicios DCC-EX
    n = MDNS.queryService("dccex", "tcp");
    
    for (int i = 0; i < n && foundWitServersCount < WIT_MAX_SERVERS; i++) {
        IPAddress ip = MDNS.IP(i);
        int port = MDNS.port(i);
        String name = MDNS.hostname(i);
        
        addFoundServer(ip, port, name, "dccex");
    }
}

void addFoundServer(const IPAddress& ip, int port, const String& name, const String& type) {
    if (foundWitServersCount < WIT_MAX_SERVERS) {
        foundWitServers[foundWitServersCount].ip = ip;
        foundWitServers[foundWitServersCount].port = port;
        foundWitServers[foundWitServersCount].name = name;
        foundWitServers[foundWitServersCount].type = type;
        foundWitServers[foundWitServersCount].isActive = true;
        
        debug_print("Found server: ");
        debug_print(name);
        debug_print(" at ");
        debug_print(ip.toString());
        debug_print(":");
        debug_println(port);
        
        foundWitServersCount++;
    }
}

void processFoundServers() {
    // Ordenar servidores por nombre
    for (int i = 0; i < foundWitServersCount - 1; i++) {
        for (int j = i + 1; j < foundWitServersCount; j++) {
            if (foundWitServers[i].name > foundWitServers[j].name) {
                WiTServer temp = foundWitServers[i];
                foundWitServers[i] = foundWitServers[j];
                foundWitServers[j] = temp;
            }
        }
    }
}

String formatServerAddress(const IPAddress& ip, int port) {
    return ip.toString() + ":" + String(port);
}

bool isValidServerIP(const String& ip) {
    IPAddress testIP;
    return testIP.fromString(ip);
}

bool isValidServerPort(int port) {
    return port > 0 && port <= 65535;
}

void parseServerString(const String& serverStr, IPAddress& ip, int& port) {
    int colonIndex = serverStr.indexOf(':');
    if (colonIndex > 0) {
        String ipStr = serverStr.substring(0, colonIndex);
        String portStr = serverStr.substring(colonIndex + 1);
        
        ip.fromString(ipStr);
        port = portStr.toInt();
    }
}

void handleConnectionTimeout() {
    static unsigned long connectionStartTime = 0;
    
    if (connectionStartTime == 0) {
        connectionStartTime = millis();
    }
    
    if (millis() - connectionStartTime > 30000) { // 30 segundos timeout
        debug_println("Connection timeout");
        wit_server_setConnectionState(WIT_CONNECTION_STATE_DISCONNECTED);
        connectionStartTime = 0;
    }
}

void handleAutoReconnect() {
    if (witConnectionState == WIT_CONNECTION_STATE_DISCONNECTED && 
        selectedWitServerIP.toString() != "0.0.0.0" &&
        (millis() - lastReconnectAttempt > WIT_RECONNECT_INTERVAL)) {
        
        debug_println("Attempting auto-reconnect");
        reconnectWitServer();
        lastReconnectAttempt = millis();
    }
}

// Funciones de configuración
void wit_server_setAutoConnect(bool enabled) {
    autoConnectToFirstWiThrottleServer = enabled;
    saveAutoConnect(enabled);
}

bool wit_server_getAutoConnect() {
    return autoConnectToFirstWiThrottleServer;
}

void wit_server_setHeartbeatEnabled(bool enabled) {
    heartbeatEnabled = enabled;
    saveHeartbeatEnabled(enabled);
}

bool wit_server_getHeartbeatEnabled() {
    return heartbeatEnabled;
}

void wit_server_setCommandDelay(int delay) {
    outboundCmdsMininumDelay = delay;
    wiThrottleProtocol.setCommandDelay(delay);
}

int wit_server_getCommandDelay() {
    return outboundCmdsMininumDelay;
}

// Stubs para callbacks - implementar según necesidades específicas
void onWiThrottleConnected() {
    debug_println("WiThrottle connected callback");
    display_showMessage("Connected to server", 2000);
}

void onWiThrottleDisconnected() {
    debug_println("WiThrottle disconnected callback");
    display_showMessage("Disconnected from server", 2000);
}

void onTrackPowerChange(TrackPower state) {
    debug_print("Track power changed to: ");
    debug_println(state == PowerOn ? "ON" : "OFF");
}

void onRosterReceived() {
    debug_println("Roster received callback");
}

void onTurnoutListReceived() {
    debug_println("Turnout list received callback");
}

void onRouteListReceived() {
    debug_println("Route list received callback");
}

void onLocoStateUpdate(int address) {
    debug_print("Loco state update for address: ");
    debug_println(address);
}

void onDirectionChange(int throttle, Direction direction) {
    debug_print("Direction change for throttle ");
    debug_print(throttle);
    debug_print(": ");
    debug_println(direction == Forward ? "Forward" : "Reverse");
}

void onSpeedChange(int throttle, int speed) {
    debug_print("Speed change for throttle ");
    debug_print(throttle);
    debug_print(": ");
    debug_println(speed);
}

void onFunctionChange(int throttle, int function, bool state) {
    debug_print("Function ");
    debug_print(function);
    debug_print(" for throttle ");
    debug_print(throttle);
    debug_print(": ");
    debug_println(state ? "ON" : "OFF");
}

// Funciones stub para otras funciones necesarias
void writeOledEnterServerIP() {
    clearOledArray();
    setAppnameForOled();
    oledText[1] = "Enter Server IP:Port";
    oledText[2] = witServerIpAndPortConstructed;
    writeOledArray(false, false);
}