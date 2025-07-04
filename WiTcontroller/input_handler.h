#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <Keypad.h>
#include <AiEsp32RotaryEncoder.h>
#include "actions.h"
#include "config_keypad_etc.h"

// Estados de uso del teclado
#define KEYPAD_USE_OPERATION 0
#define KEYPAD_USE_SELECT_SSID 1
#define KEYPAD_USE_SELECT_SSID_FROM_FOUND 2
#define KEYPAD_USE_ENTER_SSID_PASSWORD 3
#define KEYPAD_USE_SELECT_WITHROTTLE_SERVER 4
#define KEYPAD_USE_ENTER_WITHROTTLE_SERVER_IP 5
#define KEYPAD_USE_SELECT_ROSTER 6
#define KEYPAD_USE_SELECT_TURNOUTS_THROW 7
#define KEYPAD_USE_SELECT_TURNOUTS_CLOSE 8
#define KEYPAD_USE_SELECT_ROUTES 9
#define KEYPAD_USE_SELECT_FUNCTION 10

// Estados de uso del encoder
#define ENCODER_USE_OPERATION 0
#define ENCODER_USE_SSID_PASSWORD 1
#define ENCODER_USE_WITHROTTLE_SERVER_IP 2

// Acciones del botón del encoder
#define ENCODER_BUTTON_ACTION 0

// Tipos de eventos del teclado
typedef enum {
    KEY_PRESSED,
    KEY_RELEASED,
    KEY_HELD
} KeypadEventType;

// Variables globales de entrada
extern int keypadUseType;
extern int encoderUseType;
extern int encoderButtonAction;
extern bool circleValues;
extern int encoderValue;
extern int lastEncoderValue;

// Variables de potenciómetro throttle
extern bool useRotaryEncoderForThrottle;
extern int throttlePotPin;
extern bool throttlePotUseNotches;
extern int throttlePotNotchValues[];
extern int throttlePotNotchSpeeds[];
extern int throttlePotNotch;
extern int throttlePotTargetSpeed;
extern int lastThrottlePotValue;

// Funciones principales del módulo
void input_handler_init();
void input_handler_loop();
void input_handler_reset();

// Funciones de gestión del teclado
void input_handler_processKeypad(char key, KeypadState state);
void keypadEvent(KeypadEvent key);
void processKeypadPress(char key);
void processKeypadRelease(char key);
void processKeypadHold(char key);

// Funciones de gestión del encoder rotativo
void rotary_loop();
void readEncoderISR();
void rotary_onButtonClick();
void encoderSpeedChange(bool rotationIsClockwise, int speedChange);

// Funciones de gestión del potenciómetro throttle
void throttlePot_loop();
void throttlePot_loop(bool forceRead);
void processThrottlePotChange(int newValue);

// Funciones de botones adicionales
void processAdditionalButtons();
void processButtonAction(int buttonIndex, int action);

// Funciones de procesamiento de comandos
void processDirectCommand(char key);
void processMenuCommand(char key);
void processPasswordEntry(char key);
void processSelection(int selection);

// Funciones de utilidad
void setKeypadUseType(int useType);
void setEncoderUseType(int useType);
bool isValidKeypadKey(char key);
int charToInt(char key);
char getCurrentEncoderChar();

// Funciones de configuración
void configureKeypadPins();
void configureEncoderPins();
void configureAdditionalButtonPins();

#endif // INPUT_HANDLER_H