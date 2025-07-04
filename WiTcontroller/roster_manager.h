#ifndef ROSTER_MANAGER_H
#define ROSTER_MANAGER_H

#include <String.h>
#include "static.h"
#include "actions.h"

// Constantes del roster
#define MAX_ROSTER_ENTRIES 70
#define MAX_TURNOUT_ENTRIES 60
#define MAX_ROUTE_ENTRIES 60
#define MAX_THROTTLES 6
#define MAX_LOCOS_PER_THROTTLE 6
#define MAX_FUNCTION_LABELS 32

// Enumeraciones
enum Direction {
    Reverse = 0,
    Forward = 1
};

enum TrackPower {
    PowerOff = 0,
    PowerOn = 1,
    PowerUnknown = 2
};

enum TurnoutState {
    TurnoutClosed = 0,
    TurnoutThrown = 1,
    TurnoutUnknown = 2
};

enum RouteState {
    RouteInactive = 0,
    RouteActive = 1,
    RouteUnknown = 2
};

// Estructuras de datos
struct RosterEntry {
    int index;
    int address;
    String name;
    char length;
    String functionLabels[MAX_FUNCTION_LABELS];
    bool functionStates[MAX_FUNCTION_LABELS];
    bool isActive;
};

struct TurnoutEntry {
    int index;
    String sysName;
    String userName;
    TurnoutState state;
    bool isActive;
};

struct RouteEntry {
    int index;
    String sysName;
    String userName;
    RouteState state;
    bool isActive;
};

struct LocoState {
    int address;
    String name;
    Direction direction;
    int speed;
    bool functionStates[MAX_FUNCTION_LABELS];
    bool isActive;
    bool isLeadLoco;
};

struct ThrottleState {
    LocoState locos[MAX_LOCOS_PER_THROTTLE];
    int locoCount;
    int currentSpeed;
    Direction currentDirection;
    bool isActive;
};

// Variables globales del roster
extern RosterEntry rosterEntries[];
extern int rosterSize;
extern int rosterIndex[];
extern String rosterName[];
extern int rosterAddress[];
extern char rosterLength[];

// Variables de turnouts
extern TurnoutEntry turnoutEntries[];
extern int turnoutListSize;

// Variables de rutas
extern RouteEntry routeEntries[];
extern int routeListSize;

// Variables de throttles
extern ThrottleState throttleStates[];
extern int currentThrottle;
extern int maxThrottles;

// Variables de funciones
extern bool functionStates[][MAX_FUNCTION_LABELS];
extern String functionLabels[][];
extern int currentLocoAddress[];

// Funciones principales del módulo
void roster_init();
void roster_loop();
void roster_reset();

// Funciones de gestión del roster
void roster_clear();
void roster_addEntry(int address, const String& name, char length);
void roster_removeEntry(int address);
void roster_updateEntry(int address, const String& name);
RosterEntry* roster_findByAddress(int address);
RosterEntry* roster_findByIndex(int index);
void roster_sortByName();
void roster_sortByAddress();

// Funciones de procesamiento de datos del servidor
void roster_processReceivedData();
void roster_parseRosterList(const String& data);
void roster_parseTurnoutList(const String& data);
void roster_parseRouteList(const String& data);
void roster_requestRosterList();
void roster_requestTurnoutList();
void roster_requestRouteList();

// Funciones de gestión de locomotoras
void roster_acquireLoco(int throttle, int address);
void roster_releaseLoco(int throttle, int address);
void roster_releaseAllLocos(int throttle);
void roster_setLocoDirection(int throttle, int address, Direction direction);
void roster_setLocoSpeed(int throttle, int address, int speed);
void roster_setLocoFunction(int throttle, int address, int function, bool state);

// Funciones de gestión de throttles
void roster_setCurrentThrottle(int throttle);
int roster_getCurrentThrottle();
void roster_setMaxThrottles(int max);
int roster_getMaxThrottles();
ThrottleState* roster_getThrottleState(int throttle);
void roster_clearThrottle(int throttle);

// Funciones de gestión de consists
void roster_createConsist(int throttle, int* addresses, int count);
void roster_addToConsist(int throttle, int address);
void roster_removeFromConsist(int throttle, int address);
void roster_setConsistDirection(int throttle, int address, Direction direction);

// Funciones de turnouts
void roster_throwTurnout(const String& sysName);
void roster_closeTurnout(const String& sysName);
void roster_toggleTurnout(const String& sysName);
TurnoutEntry* roster_findTurnoutBySysName(const String& sysName);
TurnoutEntry* roster_findTurnoutByUserName(const String& userName);
void roster_updateTurnoutState(const String& sysName, TurnoutState state);

// Funciones de rutas
void roster_activateRoute(const String& sysName);
void roster_deactivateRoute(const String& sysName);
RouteEntry* roster_findRouteBySysName(const String& sysName);
RouteEntry* roster_findRouteByUserName(const String& userName);
void roster_updateRouteState(const String& sysName, RouteState state);

// Funciones de búsqueda y filtrado
int roster_searchByName(const String& searchTerm, int results[], int maxResults);
int roster_searchByAddress(int addressPattern, int results[], int maxResults);
void roster_filterByLength(char length, int results[], int maxResults);

// Funciones de actualización de estado
void roster_updateStatus();
void roster_updateFromServer(const String& message);
void roster_syncWithServer();

// Funciones de utilidad
String roster_formatLocoInfo(int address);
String roster_formatThrottleInfo(int throttle);
String roster_getLocoName(int address);
int roster_getLocoAddress(const String& name);
bool roster_isLocoAcquired(int address);
int roster_getThrottleForLoco(int address);

// Funciones de configuración
void roster_setFunctionLabels(int address, const String labels[]);
void roster_getFunctionLabels(int address, String labels[]);
void roster_saveFunctionStates(int throttle);
void roster_loadFunctionStates(int throttle);

// Funciones de exportación/importación
void roster_exportToSerial();
void roster_importFromSerial();
void roster_backupState();
void roster_restoreState();

#endif // ROSTER_MANAGER_H