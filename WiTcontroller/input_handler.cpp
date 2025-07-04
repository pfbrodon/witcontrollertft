#include "input_handler.h"
#include "WiTcontroller.h"
#include "wifi_manager.h"
#include "display_manager.h"

// Variables globales de entrada
int keypadUseType = KEYPAD_USE_OPERATION;
int encoderUseType = ENCODER_USE_OPERATION;
int encoderButtonAction = ENCODER_BUTTON_ACTION;
bool circleValues = true;
int encoderValue = 0;
int lastEncoderValue = 0;

// Variables de potenciómetro throttle
bool useRotaryEncoderForThrottle = USE_ROTARY_ENCODER_FOR_THROTTLE;
int throttlePotPin = THROTTLE_POT_PIN;
bool throttlePotUseNotches = THROTTLE_POT_USE_NOTCHES;
int throttlePotNotchValues[] = THROTTLE_POT_NOTCH_VALUES;
int throttlePotNotchSpeeds[] = THROTTLE_POT_NOTCH_SPEEDS;
int throttlePotNotch = 0;
int throttlePotTargetSpeed = 0;
int lastThrottlePotValue = 0;
int lastThrottlePotReadTime = -1;

// Variables externas
extern AiEsp32RotaryEncoder rotaryEncoder;
extern Keypad customKeypad;
extern bool menuCommandStarted;
extern String menuCommand;
extern int currentThrottle;
extern int currentSpeed[];
extern Direction currentDirection[];
extern int speedStepCurrentMultiplier;

void input_handler_init() {
    debug_println("Initializing input handler...");
    
    // Configurar pines del teclado
    configureKeypadPins();
    
    // Configurar pines del encoder
    configureEncoderPins();
    
    // Configurar botones adicionales
    configureAdditionalButtonPins();
    
    // Configurar potenciómetro si está habilitado
    if (!useRotaryEncoderForThrottle) {
        pinMode(throttlePotPin, INPUT);
    }
    
    // Inicializar variables
    keypadUseType = KEYPAD_USE_OPERATION;
    encoderUseType = ENCODER_USE_OPERATION;
    encoderValue = 0;
    lastEncoderValue = 0;
    
    debug_println("Input handler initialized");
}

void input_handler_loop() {
    // Procesar teclado
    customKeypad.getKey();
    
    // Procesar encoder rotativo
    if (useRotaryEncoderForThrottle) {
        rotary_loop();
    } else {
        // Procesar potenciómetro throttle
        throttlePot_loop();
    }
    
    // Procesar botones adicionales
    processAdditionalButtons();
}

void input_handler_reset() {
    keypadUseType = KEYPAD_USE_OPERATION;
    encoderUseType = ENCODER_USE_OPERATION;
    encoderValue = 0;
    lastEncoderValue = 0;
    menuCommandStarted = false;
    menuCommand = "";
}

void input_handler_processKeypad(char key, KeypadState state) {
    switch (state) {
        case PRESSED:
            processKeypadPress(key);
            break;
        case RELEASED:
            processKeypadRelease(key);
            break;
        case HOLD:
            processKeypadHold(key);
            break;
        default:
            break;
    }
}

void processKeypadPress(char key) {
    debug_print("Key pressed: ");
    debug_println(key);
    
    switch (keypadUseType) {
        case KEYPAD_USE_OPERATION:
            processDirectCommand(key);
            break;
        case KEYPAD_USE_SELECT_SSID:
        case KEYPAD_USE_SELECT_SSID_FROM_FOUND:
            if (isdigit(key)) {
                int selection = charToInt(key);
                selectSsid(selection);
            }
            break;
        case KEYPAD_USE_ENTER_SSID_PASSWORD:
            processPasswordEntry(key);
            break;
        case KEYPAD_USE_SELECT_WITHROTTLE_SERVER:
            if (isdigit(key)) {
                int selection = charToInt(key);
                selectWitServer(selection);
            }
            break;
        case KEYPAD_USE_SELECT_ROSTER:
        case KEYPAD_USE_SELECT_TURNOUTS_THROW:
        case KEYPAD_USE_SELECT_TURNOUTS_CLOSE:
        case KEYPAD_USE_SELECT_ROUTES:
        case KEYPAD_USE_SELECT_FUNCTION:
            processSelection(charToInt(key));
            break;
        default:
            break;
    }
}

void processKeypadRelease(char key) {
    // Implementar lógica de liberación de tecla si es necesario
}

void processKeypadHold(char key) {
    // Implementar lógica de mantener tecla presionada si es necesario
    debug_print("Key held: ");
    debug_println(key);
}

void processDirectCommand(char key) {
    if (key == '*') {
        // Iniciar comando de menú
        menuCommandStarted = true;
        menuCommand = "";
        display_showMessage("Menu command...", 0);
    } else if (key == '#') {
        if (menuCommandStarted) {
            // Finalizar comando de menú
            processMenuCommand(menuCommand);
            menuCommandStarted = false;
            menuCommand = "";
        } else {
            // Mostrar comandos directos
            display_showDirectCommands();
        }
    } else if (menuCommandStarted) {
        // Agregar a comando de menú
        menuCommand += key;
        display_showMessage("Menu: " + menuCommand, 0);
    } else {
        // Comando directo
        executeDirectCommand(key);
    }
}

void processMenuCommand(String command) {
    debug_print("Processing menu command: ");
    debug_println(command);
    
    if (command == "1") {
        // Seleccionar locomotora
        display_showRoster();
        keypadUseType = KEYPAD_USE_SELECT_ROSTER;
    } else if (command == "2") {
        // Liberar locomotora
        releaseCurrentLoco();
    } else if (command == "3") {
        // Cambiar dirección
        toggleDirection();
    } else if (command == "4") {
        // Multiplicador de velocidad
        toggleSpeedMultiplier();
    } else if (command == "5") {
        // Activar turnout
        display_showTurnouts();
        keypadUseType = KEYPAD_USE_SELECT_TURNOUTS_THROW;
    } else if (command == "6") {
        // Cerrar turnout
        display_showTurnouts();
        keypadUseType = KEYPAD_USE_SELECT_TURNOUTS_CLOSE;
    } else if (command == "7") {
        // Activar ruta
        display_showRoutes();
        keypadUseType = KEYPAD_USE_SELECT_ROUTES;
    } else if (command == "8") {
        // Control de energía de la vía
        toggleTrackPower();
    } else if (command == "9") {
        // Menú de extras
        display_showExtrasMenu();
    } else if (command == "0") {
        // Funciones
        display_showFunctions();
        keypadUseType = KEYPAD_USE_SELECT_FUNCTION;
    }
}

void executeDirectCommand(char key) {
    debug_print("Executing direct command: ");
    debug_println(key);
    
    switch (key) {
        case '0':
            toggleFunction(0); // Luces
            break;
        case '1':
            toggleFunction(1); // Campana
            break;
        case '2':
            toggleFunction(2); // Bocina
            break;
        case '3':
            toggleFunction(3);
            break;
        case '4':
            toggleFunction(4);
            break;
        case '5':
            nextThrottle();
            break;
        case '6':
            toggleSpeedMultiplier();
            break;
        case '7':
            setDirection(DIRECTION_REVERSE);
            break;
        case '8':
            emergencyStop();
            break;
        case '9':
            setDirection(DIRECTION_FORWARD);
            break;
        default:
            break;
    }
}

void processPasswordEntry(char key) {
    if (key == '#') {
        // Finalizar entrada de contraseña
        selectedSsidPassword = ssidPasswordEntered;
        ssidConnectionState = CONNECTION_STATE_SELECTED;
        keypadUseType = KEYPAD_USE_OPERATION;
    } else if (key == '*') {
        // Borrar último carácter
        ssidPasswordDeleteChar(key);
    } else {
        // Agregar carácter
        ssidPasswordAddChar(key);
    }
}

void processSelection(int selection) {
    switch (keypadUseType) {
        case KEYPAD_USE_SELECT_ROSTER:
            selectLocoFromRoster(selection);
            break;
        case KEYPAD_USE_SELECT_TURNOUTS_THROW:
            throwTurnout(selection);
            break;
        case KEYPAD_USE_SELECT_TURNOUTS_CLOSE:
            closeTurnout(selection);
            break;
        case KEYPAD_USE_SELECT_ROUTES:
            activateRoute(selection);
            break;
        case KEYPAD_USE_SELECT_FUNCTION:
            toggleFunction(selection);
            break;
        default:
            break;
    }
    
    // Volver al modo de operación normal
    keypadUseType = KEYPAD_USE_OPERATION;
}

void rotary_loop() {
    if (rotaryEncoder.encoderChanged()) {
        int newValue = rotaryEncoder.readEncoder();
        bool clockwise = (newValue > encoderValue);
        
        switch (encoderUseType) {
            case ENCODER_USE_OPERATION:
                // Control de velocidad
                encoderSpeedChange(clockwise, abs(newValue - encoderValue));
                break;
            case ENCODER_USE_SSID_PASSWORD:
                // Selección de carácter para contraseña
                updatePasswordCharacter(clockwise);
                break;
            case ENCODER_USE_WITHROTTLE_SERVER_IP:
                // Entrada de IP del servidor
                updateServerIPEntry(clockwise);
                break;
            default:
                break;
        }
        
        encoderValue = newValue;
    }
    
    // Verificar si se presionó el botón del encoder
    if (rotaryEncoder.isEncoderButtonClicked()) {
        rotary_onButtonClick();
    }
}

void readEncoderISR() {
    rotaryEncoder.readEncoder_ISR();
}

void rotary_onButtonClick() {
    debug_println("Encoder button clicked");
    
    switch (encoderUseType) {
        case ENCODER_USE_OPERATION:
            // Parada de emergencia o acción por defecto
            emergencyStop();
            break;
        case ENCODER_USE_SSID_PASSWORD:
            // Confirmar carácter de contraseña
            confirmPasswordCharacter();
            break;
        case ENCODER_USE_WITHROTTLE_SERVER_IP:
            // Confirmar entrada de IP
            confirmServerIPEntry();
            break;
        default:
            break;
    }
}

void encoderSpeedChange(bool rotationIsClockwise, int speedChange) {
    int newSpeed = currentSpeed[currentThrottle];
    
    if (rotationIsClockwise) {
        newSpeed += speedChange * speedStepCurrentMultiplier;
        if (newSpeed > 126) newSpeed = 126;
    } else {
        newSpeed -= speedChange * speedStepCurrentMultiplier;
        if (newSpeed < 0) newSpeed = 0;
    }
    
    if (newSpeed != currentSpeed[currentThrottle]) {
        setThrottleSpeed(currentThrottle, newSpeed);
        display_updateThrottle();
    }
}

void throttlePot_loop() {
    throttlePot_loop(false);
}

void throttlePot_loop(bool forceRead) {
    static unsigned long lastRead = 0;
    
    if (!forceRead && (millis() - lastRead < 100)) {
        return; // Solo leer cada 100ms
    }
    
    int potValue = analogRead(throttlePotPin);
    
    if (abs(potValue - lastThrottlePotValue) > 10 || forceRead) { // Histéresis
        if (throttlePotUseNotches) {
            // Modo notches
            int newNotch = 0;
            for (int i = 0; i < 8; i++) {
                if (potValue >= throttlePotNotchValues[i]) {
                    newNotch = i;
                }
            }
            
            if (newNotch != throttlePotNotch) {
                throttlePotNotch = newNotch;
                int newSpeed = throttlePotNotchSpeeds[newNotch];
                setThrottleSpeed(currentThrottle, newSpeed);
                display_updateThrottle();
            }
        } else {
            // Modo lineal
            int newSpeed = map(potValue, throttlePotNotchValues[0], throttlePotNotchValues[7], 0, 126);
            newSpeed = constrain(newSpeed, 0, 126);
            
            if (abs(newSpeed - currentSpeed[currentThrottle]) > 2) { // Histéresis para velocidad
                setThrottleSpeed(currentThrottle, newSpeed);
                display_updateThrottle();
            }
        }
        
        lastThrottlePotValue = potValue;
    }
    
    lastRead = millis();
}

void processAdditionalButtons() {
    #if defined(USE_ADDITIONAL_BUTTONS) && USE_ADDITIONAL_BUTTONS
    static bool lastButtonStates[MAX_ADDITIONAL_BUTTONS] = {false};
    
    for (int i = 0; i < MAX_ADDITIONAL_BUTTONS; i++) {
        if (additionalButtonPins[i] != -1) {
            bool currentState = !digitalRead(additionalButtonPins[i]); // Asumiendo pull-up
            
            if (currentState && !lastButtonStates[i]) {
                // Botón presionado
                processButtonAction(i, additionalButtonActions[i]);
            }
            
            lastButtonStates[i] = currentState;
        }
    }
    #endif
}

void processButtonAction(int buttonIndex, int action) {
    debug_print("Additional button ");
    debug_print(buttonIndex);
    debug_print(" pressed, action: ");
    debug_println(action);
    
    if (action >= FUNCTION_0 && action <= FUNCTION_31) {
        toggleFunction(action);
    } else if (action == SPEED_STOP) {
        setThrottleSpeed(currentThrottle, 0);
    } else if (action == E_STOP) {
        emergencyStopAll();
    } else if (action == DIRECTION_TOGGLE) {
        toggleDirection();
    } else if (action == NEXT_THROTTLE) {
        nextThrottle();
    }
    // Agregar más acciones según sea necesario
}

void setKeypadUseType(int useType) {
    keypadUseType = useType;
    debug_print("Keypad use type set to: ");
    debug_println(useType);
}

void setEncoderUseType(int useType) {
    encoderUseType = useType;
    debug_print("Encoder use type set to: ");
    debug_println(useType);
}

bool isValidKeypadKey(char key) {
    return (key >= '0' && key <= '9') || key == '*' || key == '#';
}

int charToInt(char key) {
    if (key >= '0' && key <= '9') {
        return key - '0';
    }
    return -1;
}

void configureKeypadPins() {
    // Los pines del teclado se configuran automáticamente por la librería Keypad
    debug_println("Keypad pins configured");
}

void configureEncoderPins() {
    // Los pines del encoder se configuran en la inicialización del objeto
    debug_println("Encoder pins configured");
}

void configureAdditionalButtonPins() {
    #if defined(USE_ADDITIONAL_BUTTONS) && USE_ADDITIONAL_BUTTONS
    for (int i = 0; i < MAX_ADDITIONAL_BUTTONS; i++) {
        if (additionalButtonPins[i] != -1) {
            pinMode(additionalButtonPins[i], additionalButtonTypes[i]);
        }
    }
    debug_println("Additional button pins configured");
    #endif
}

// Funciones stub que requerirían implementación completa en el contexto real
void selectLocoFromRoster(int selection) {
    debug_print("Select loco from roster: ");
    debug_println(selection);
}

void releaseCurrentLoco() {
    debug_println("Release current loco");
}

void toggleDirection() {
    debug_println("Toggle direction");
}

void toggleSpeedMultiplier() {
    speedStepCurrentMultiplier = (speedStepCurrentMultiplier == 1) ? 2 : 1;
    debug_print("Speed multiplier: ");
    debug_println(speedStepCurrentMultiplier);
}

void toggleFunction(int function) {
    debug_print("Toggle function: ");
    debug_println(function);
}

void nextThrottle() {
    currentThrottle = (currentThrottle + 1) % MAX_THROTTLES;
    debug_print("Current throttle: ");
    debug_println(currentThrottle);
}

void setDirection(int direction) {
    currentDirection[currentThrottle] = (Direction)direction;
    debug_print("Set direction: ");
    debug_println(direction);
}

void emergencyStop() {
    currentSpeed[currentThrottle] = 0;
    debug_println("Emergency stop current throttle");
}

void emergencyStopAll() {
    for (int i = 0; i < MAX_THROTTLES; i++) {
        currentSpeed[i] = 0;
    }
    debug_println("Emergency stop all throttles");
}

void setThrottleSpeed(int throttle, int speed) {
    currentSpeed[throttle] = speed;
    debug_print("Set throttle ");
    debug_print(throttle);
    debug_print(" speed to: ");
    debug_println(speed);
}