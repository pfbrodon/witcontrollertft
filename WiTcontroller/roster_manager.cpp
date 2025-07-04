#include "roster_manager.h"
#include "WiTcontroller.h"
#include "wit_server.h"
#include "preferences_manager.h"
#include "display_manager.h"

// Variables globales del roster
RosterEntry rosterEntries[MAX_ROSTER_ENTRIES];
int rosterSize = 0;
int rosterIndex[MAX_ROSTER_ENTRIES];
String rosterName[MAX_ROSTER_ENTRIES];
int rosterAddress[MAX_ROSTER_ENTRIES];
char rosterLength[MAX_ROSTER_ENTRIES];

// Variables de turnouts
TurnoutEntry turnoutEntries[MAX_TURNOUT_ENTRIES];
int turnoutListSize = 0;

// Variables de rutas
RouteEntry routeEntries[MAX_ROUTE_ENTRIES];
int routeListSize = 0;

// Variables de throttles
ThrottleState throttleStates[MAX_THROTTLES];
int currentThrottle = 0;
int maxThrottles = 2;

// Variables de funciones
bool functionStates[MAX_THROTTLES][MAX_FUNCTION_LABELS];
String functionLabels[MAX_ROSTER_ENTRIES][MAX_FUNCTION_LABELS];
int currentLocoAddress[MAX_THROTTLES];

// Variables de estado
int page = 0;
int functionPage = 0;
bool functionHasBeenSelected = false;

void roster_init() {
    debug_println("Initializing roster manager...");
    
    // Inicializar estructuras
    roster_clear();
    
    // Inicializar throttles
    for (int i = 0; i < MAX_THROTTLES; i++) {
        roster_clearThrottle(i);
        currentLocoAddress[i] = 0;
    }
    
    // Cargar configuración
    maxThrottles = getMaxThrottles();
    currentThrottle = 0;
    
    debug_println("Roster manager initialized");
}

void roster_loop() {
    // Actualizar estado periódicamente
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 1000) { // Actualizar cada segundo
        roster_updateStatus();
        lastUpdate = millis();
    }
}

void roster_reset() {
    roster_clear();
    
    for (int i = 0; i < MAX_THROTTLES; i++) {
        roster_clearThrottle(i);
    }
    
    currentThrottle = 0;
    page = 0;
    functionPage = 0;
    functionHasBeenSelected = false;
    
    debug_println("Roster manager reset");
}

void roster_clear() {
    rosterSize = 0;
    turnoutListSize = 0;
    routeListSize = 0;
    
    // Limpiar entradas del roster
    for (int i = 0; i < MAX_ROSTER_ENTRIES; i++) {
        rosterEntries[i].isActive = false;
        rosterEntries[i].address = 0;
        rosterEntries[i].name = "";
        rosterEntries[i].length = 'S';
        
        for (int j = 0; j < MAX_FUNCTION_LABELS; j++) {
            rosterEntries[i].functionLabels[j] = "";
            rosterEntries[i].functionStates[j] = false;
        }
    }
    
    // Limpiar turnouts
    for (int i = 0; i < MAX_TURNOUT_ENTRIES; i++) {
        turnoutEntries[i].isActive = false;
        turnoutEntries[i].state = TurnoutUnknown;
    }
    
    // Limpiar rutas
    for (int i = 0; i < MAX_ROUTE_ENTRIES; i++) {
        routeEntries[i].isActive = false;
        routeEntries[i].state = RouteUnknown;
    }
    
    debug_println("Roster cleared");
}

void roster_addEntry(int address, const String& name, char length) {
    if (rosterSize >= MAX_ROSTER_ENTRIES) {
        debug_println("Roster full, cannot add entry");
        return;
    }
    
    // Verificar si ya existe
    RosterEntry* existing = roster_findByAddress(address);
    if (existing != nullptr) {
        // Actualizar entrada existente
        existing->name = name;
        existing->length = length;
        debug_print("Updated roster entry: ");
        debug_println(address);
        return;
    }
    
    // Agregar nueva entrada
    rosterEntries[rosterSize].index = rosterSize;
    rosterEntries[rosterSize].address = address;
    rosterEntries[rosterSize].name = name;
    rosterEntries[rosterSize].length = length;
    rosterEntries[rosterSize].isActive = true;
    
    // Inicializar funciones
    for (int i = 0; i < MAX_FUNCTION_LABELS; i++) {
        rosterEntries[rosterSize].functionStates[i] = false;
        rosterEntries[rosterSize].functionLabels[i] = "F" + String(i);
    }
    
    debug_print("Added roster entry: ");
    debug_print(address);
    debug_print(" - ");
    debug_println(name);
    
    rosterSize++;
}

void roster_removeEntry(int address) {
    for (int i = 0; i < rosterSize; i++) {
        if (rosterEntries[i].address == address && rosterEntries[i].isActive) {
            rosterEntries[i].isActive = false;
            debug_print("Removed roster entry: ");
            debug_println(address);
            break;
        }
    }
}

RosterEntry* roster_findByAddress(int address) {
    for (int i = 0; i < rosterSize; i++) {
        if (rosterEntries[i].address == address && rosterEntries[i].isActive) {
            return &rosterEntries[i];
        }
    }
    return nullptr;
}

RosterEntry* roster_findByIndex(int index) {
    if (index >= 0 && index < rosterSize && rosterEntries[index].isActive) {
        return &rosterEntries[index];
    }
    return nullptr;
}

void roster_sortByName() {
    // Ordenamiento burbuja simple por nombre
    for (int i = 0; i < rosterSize - 1; i++) {
        for (int j = 0; j < rosterSize - i - 1; j++) {
            if (rosterEntries[j].isActive && rosterEntries[j + 1].isActive &&
                rosterEntries[j].name > rosterEntries[j + 1].name) {
                
                RosterEntry temp = rosterEntries[j];
                rosterEntries[j] = rosterEntries[j + 1];
                rosterEntries[j + 1] = temp;
            }
        }
    }
}

void roster_sortByAddress() {
    // Ordenamiento burbuja simple por dirección
    for (int i = 0; i < rosterSize - 1; i++) {
        for (int j = 0; j < rosterSize - i - 1; j++) {
            if (rosterEntries[j].isActive && rosterEntries[j + 1].isActive &&
                rosterEntries[j].address > rosterEntries[j + 1].address) {
                
                RosterEntry temp = rosterEntries[j];
                rosterEntries[j] = rosterEntries[j + 1];
                rosterEntries[j + 1] = temp;
            }
        }
    }
}

void roster_processReceivedData() {
    debug_println("Processing received roster data");
    
    // En una implementación real, esto procesaría los datos del servidor
    // Por ahora, solo actualizar la pantalla
    display_updateRoster();
}

void roster_parseRosterList(const String& data) {
    debug_print("Parsing roster list: ");
    debug_println(data);
    
    // Formato esperado: RL###}|{dcc_address}|{name}|{length
    // Ejemplo: RL2688}|{3}|{Loco 3}|{S
    
    if (!data.startsWith("RL")) return;
    
    int startPos = data.indexOf("}|{") + 3;
    if (startPos < 3) return;
    
    // Extraer dirección
    int endPos = data.indexOf("}|{", startPos);
    if (endPos == -1) return;
    int address = data.substring(startPos, endPos).toInt();
    
    // Extraer nombre
    startPos = endPos + 3;
    endPos = data.indexOf("}|{", startPos);
    if (endPos == -1) return;
    String name = data.substring(startPos, endPos);
    
    // Extraer longitud
    startPos = endPos + 3;
    char length = data.charAt(startPos);
    
    // Agregar al roster
    roster_addEntry(address, name, length);
}

void roster_parseTurnoutList(const String& data) {
    debug_print("Parsing turnout list: ");
    debug_println(data);
    
    // Implementar parsing de turnouts según formato del servidor
}

void roster_parseRouteList(const String& data) {
    debug_print("Parsing route list: ");
    debug_println(data);
    
    // Implementar parsing de rutas según formato del servidor
}

void roster_requestRosterList() {
    if (wit_server_isConnected()) {
        sendWiThrottleCommand("RL");
        debug_println("Requested roster list");
    }
}

void roster_requestTurnoutList() {
    if (wit_server_isConnected()) {
        sendWiThrottleCommand("PTL");
        debug_println("Requested turnout list");
    }
}

void roster_requestRouteList() {
    if (wit_server_isConnected()) {
        sendWiThrottleCommand("PRL");
        debug_println("Requested route list");
    }
}

void roster_acquireLoco(int throttle, int address) {
    if (throttle < 0 || throttle >= MAX_THROTTLES) return;
    
    ThrottleState* state = &throttleStates[throttle];
    
    // Verificar si hay espacio
    if (state->locoCount >= MAX_LOCOS_PER_THROTTLE) {
        debug_println("Throttle full, cannot acquire loco");
        return;
    }
    
    // Verificar si ya está adquirida
    for (int i = 0; i < state->locoCount; i++) {
        if (state->locos[i].address == address) {
            debug_println("Loco already acquired on this throttle");
            return;
        }
    }
    
    // Agregar locomotora
    LocoState* loco = &state->locos[state->locoCount];
    loco->address = address;
    loco->name = roster_getLocoName(address);
    loco->direction = Forward;
    loco->speed = 0;
    loco->isActive = true;
    loco->isLeadLoco = (state->locoCount == 0);
    
    // Inicializar funciones
    for (int i = 0; i < MAX_FUNCTION_LABELS; i++) {
        loco->functionStates[i] = false;
    }
    
    state->locoCount++;
    state->isActive = true;
    
    // Actualizar dirección actual si es la primera locomotora
    if (loco->isLeadLoco) {
        currentLocoAddress[throttle] = address;
    }
    
    // Enviar comando al servidor
    if (wit_server_isConnected()) {
        String command = "MT" + String(throttle) + "+*<;>L" + String(address);
        sendWiThrottleCommand(command);
    }
    
    debug_print("Acquired loco ");
    debug_print(address);
    debug_print(" on throttle ");
    debug_println(throttle);
}

void roster_releaseLoco(int throttle, int address) {
    if (throttle < 0 || throttle >= MAX_THROTTLES) return;
    
    ThrottleState* state = &throttleStates[throttle];
    
    for (int i = 0; i < state->locoCount; i++) {
        if (state->locos[i].address == address) {
            // Enviar comando de liberación al servidor
            if (wit_server_isConnected()) {
                String command = "MT" + String(throttle) + "-*<;>r";
                sendWiThrottleCommand(command);
            }
            
            // Mover locomotoras restantes
            for (int j = i; j < state->locoCount - 1; j++) {
                state->locos[j] = state->locos[j + 1];
            }
            
            state->locoCount--;
            
            // Actualizar loco principal si es necesario
            if (state->locoCount > 0) {
                state->locos[0].isLeadLoco = true;
                currentLocoAddress[throttle] = state->locos[0].address;
            } else {
                state->isActive = false;
                currentLocoAddress[throttle] = 0;
            }
            
            debug_print("Released loco ");
            debug_print(address);
            debug_print(" from throttle ");
            debug_println(throttle);
            break;
        }
    }
}

void roster_releaseAllLocos(int throttle) {
    if (throttle < 0 || throttle >= MAX_THROTTLES) return;
    
    ThrottleState* state = &throttleStates[throttle];
    
    if (state->locoCount > 0) {
        // Enviar comando de liberación al servidor
        if (wit_server_isConnected()) {
            String command = "MT" + String(throttle) + "-*<;>r";
            sendWiThrottleCommand(command);
        }
        
        roster_clearThrottle(throttle);
        
        debug_print("Released all locos from throttle ");
        debug_println(throttle);
    }
}

void roster_setLocoDirection(int throttle, int address, Direction direction) {
    if (throttle < 0 || throttle >= MAX_THROTTLES) return;
    
    ThrottleState* state = &throttleStates[throttle];
    
    for (int i = 0; i < state->locoCount; i++) {
        if (state->locos[i].address == address) {
            state->locos[i].direction = direction;
            
            // Enviar comando al servidor
            if (wit_server_isConnected()) {
                String dirStr = (direction == Forward) ? "1" : "0";
                String command = "MT" + String(throttle) + "*<;>R" + dirStr;
                sendWiThrottleCommand(command);
            }
            
            debug_print("Set direction for loco ");
            debug_print(address);
            debug_print(" to ");
            debug_println(direction == Forward ? "Forward" : "Reverse");
            break;
        }
    }
}

void roster_setLocoSpeed(int throttle, int address, int speed) {
    if (throttle < 0 || throttle >= MAX_THROTTLES) return;
    
    speed = constrain(speed, 0, 126);
    
    ThrottleState* state = &throttleStates[throttle];
    state->currentSpeed = speed;
    
    for (int i = 0; i < state->locoCount; i++) {
        if (state->locos[i].address == address) {
            state->locos[i].speed = speed;
            break;
        }
    }
    
    // Enviar comando al servidor
    if (wit_server_isConnected()) {
        String command = "MT" + String(throttle) + "*<;>V" + String(speed);
        sendWiThrottleCommand(command);
    }
    
    debug_print("Set speed for throttle ");
    debug_print(throttle);
    debug_print(" to ");
    debug_println(speed);
}

void roster_setLocoFunction(int throttle, int address, int function, bool state) {
    if (throttle < 0 || throttle >= MAX_THROTTLES) return;
    if (function < 0 || function >= MAX_FUNCTION_LABELS) return;
    
    ThrottleState* throttleState = &throttleStates[throttle];
    
    for (int i = 0; i < throttleState->locoCount; i++) {
        if (throttleState->locos[i].address == address) {
            throttleState->locos[i].functionStates[function] = state;
            break;
        }
    }
    
    // Enviar comando al servidor
    if (wit_server_isConnected()) {
        String command = "MT" + String(throttle) + "*<;>F" + String(state ? "1" : "0") + String(function);
        sendWiThrottleCommand(command);
    }
    
    debug_print("Set function ");
    debug_print(function);
    debug_print(" for loco ");
    debug_print(address);
    debug_print(" to ");
    debug_println(state ? "ON" : "OFF");
}

void roster_setCurrentThrottle(int throttle) {
    if (throttle >= 0 && throttle < maxThrottles) {
        currentThrottle = throttle;
        debug_print("Current throttle set to: ");
        debug_println(throttle);
    }
}

int roster_getCurrentThrottle() {
    return currentThrottle;
}

void roster_setMaxThrottles(int max) {
    maxThrottles = constrain(max, 1, MAX_THROTTLES);
    saveMaxThrottles(maxThrottles);
    debug_print("Max throttles set to: ");
    debug_println(maxThrottles);
}

int roster_getMaxThrottles() {
    return maxThrottles;
}

ThrottleState* roster_getThrottleState(int throttle) {
    if (throttle >= 0 && throttle < MAX_THROTTLES) {
        return &throttleStates[throttle];
    }
    return nullptr;
}

void roster_clearThrottle(int throttle) {
    if (throttle < 0 || throttle >= MAX_THROTTLES) return;
    
    ThrottleState* state = &throttleStates[throttle];
    state->locoCount = 0;
    state->currentSpeed = 0;
    state->currentDirection = Forward;
    state->isActive = false;
    
    for (int i = 0; i < MAX_LOCOS_PER_THROTTLE; i++) {
        state->locos[i].isActive = false;
        state->locos[i].address = 0;
        state->locos[i].name = "";
        state->locos[i].direction = Forward;
        state->locos[i].speed = 0;
        state->locos[i].isLeadLoco = false;
        
        for (int j = 0; j < MAX_FUNCTION_LABELS; j++) {
            state->locos[i].functionStates[j] = false;
        }
    }
    
    currentLocoAddress[throttle] = 0;
}

void roster_updateStatus() {
    // Actualizar estado general del roster
    // En una implementación real, esto sincronizaría con el servidor
}

String roster_formatLocoInfo(int address) {
    RosterEntry* entry = roster_findByAddress(address);
    if (entry != nullptr) {
        return String(address) + " - " + entry->name;
    }
    return String(address);
}

String roster_formatThrottleInfo(int throttle) {
    if (throttle < 0 || throttle >= MAX_THROTTLES) return "";
    
    ThrottleState* state = &throttleStates[throttle];
    if (!state->isActive || state->locoCount == 0) {
        return "T" + String(throttle + 1) + ": No locos";
    }
    
    String info = "T" + String(throttle + 1) + ": ";
    for (int i = 0; i < state->locoCount; i++) {
        if (i > 0) info += "+";
        info += String(state->locos[i].address);
    }
    
    return info;
}

String roster_getLocoName(int address) {
    RosterEntry* entry = roster_findByAddress(address);
    if (entry != nullptr) {
        return entry->name;
    }
    return "Loco " + String(address);
}

int roster_getLocoAddress(const String& name) {
    for (int i = 0; i < rosterSize; i++) {
        if (rosterEntries[i].isActive && rosterEntries[i].name == name) {
            return rosterEntries[i].address;
        }
    }
    return 0;
}

bool roster_isLocoAcquired(int address) {
    for (int i = 0; i < MAX_THROTTLES; i++) {
        ThrottleState* state = &throttleStates[i];
        for (int j = 0; j < state->locoCount; j++) {
            if (state->locos[j].address == address && state->locos[j].isActive) {
                return true;
            }
        }
    }
    return false;
}

int roster_getThrottleForLoco(int address) {
    for (int i = 0; i < MAX_THROTTLES; i++) {
        ThrottleState* state = &throttleStates[i];
        for (int j = 0; j < state->locoCount; j++) {
            if (state->locos[j].address == address && state->locos[j].isActive) {
                return i;
            }
        }
    }
    return -1;
}

void roster_exportToSerial() {
    debug_println("=== ROSTER EXPORT ===");
    debug_print("Roster Size: ");
    debug_println(rosterSize);
    
    for (int i = 0; i < rosterSize; i++) {
        if (rosterEntries[i].isActive) {
            debug_print("Entry ");
            debug_print(i);
            debug_print(": Address=");
            debug_print(rosterEntries[i].address);
            debug_print(", Name=");
            debug_print(rosterEntries[i].name);
            debug_print(", Length=");
            debug_println(rosterEntries[i].length);
        }
    }
    
    debug_println("=== THROTTLE STATES ===");
    for (int i = 0; i < maxThrottles; i++) {
        debug_print("Throttle ");
        debug_print(i);
        debug_print(": ");
        debug_println(roster_formatThrottleInfo(i));
    }
    
    debug_println("=== END EXPORT ===");
}

// Funciones stub para turnouts y rutas
void roster_throwTurnout(const String& sysName) {
    debug_print("Throw turnout: ");
    debug_println(sysName);
    
    if (wit_server_isConnected()) {
        String command = "PTA" + turnoutPrefix + sysName + "T";
        sendWiThrottleCommand(command);
    }
}

void roster_closeTurnout(const String& sysName) {
    debug_print("Close turnout: ");
    debug_println(sysName);
    
    if (wit_server_isConnected()) {
        String command = "PTA" + turnoutPrefix + sysName + "C";
        sendWiThrottleCommand(command);
    }
}

void roster_activateRoute(const String& sysName) {
    debug_print("Activate route: ");
    debug_println(sysName);
    
    if (wit_server_isConnected()) {
        String command = "PRA" + routePrefix + sysName + "A";
        sendWiThrottleCommand(command);
    }
}

// Variables externas necesarias
extern String turnoutPrefix;
extern String routePrefix;
extern String startupCommands[];