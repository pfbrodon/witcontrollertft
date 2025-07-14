#ifndef DISPLAY_INTEGRATION_EXAMPLE_H
#define DISPLAY_INTEGRATION_EXAMPLE_H

// =====================================================
// Ejemplo de cómo integrar el nuevo sistema de display
// =====================================================

/*
INSTRUCCIONES PARA INTEGRAR EL NUEVO SISTEMA DE DISPLAY:

1. En WiTcontroller.ino, agregar estos includes después de los includes existentes:

#include "display_interface.h"
#include "display_oled_u8g2.h"
#include "display_manager.h"
#include "display_factory.h"

2. Declarar las variables globales del display (después de las declaraciones existentes):

// Nuevas variables para el sistema de display modular
DisplayInterface* displayInterface = nullptr;
DisplayManager* displayManager = nullptr;

3. En la función setup(), después de la inicialización de u8g2, agregar:

// Configurar el sistema de display modular
displayInterface = DisplayFactory::createOLEDDisplay(&u8g2);
if (displayInterface) {
    displayManager = new DisplayManager(displayInterface);
    displayManager->begin();
}

4. Reemplazar las llamadas a las funciones antiguas por las nuevas:

FUNCIÓN ANTIGUA               →  FUNCIÓN NUEVA
writeOledSpeed()             →  displayManager->showThrottleScreen()
writeOledMenu(text, prime)   →  displayManager->showMenu(text, prime)
writeOledRoster(text)        →  displayManager->showRosterList(text)
writeOledTurnoutList(t, a)   →  displayManager->showTurnoutList(t, a)
writeOledRouteList(text)     →  displayManager->showRouteList(text)
writeOledFunctionList(text)  →  displayManager->showFunctionList(text)
writeOledFoundSSids(text)    →  displayManager->showSsidList(text)
writeOledEnterPassword()     →  displayManager->showPasswordEntry()
writeOledDirectCommands()    →  displayManager->showDirectCommands()
writeOledEditConsist()       →  displayManager->showEditConsist()
writeOledAllLocos(hide)      →  displayManager->showAllLocos(hide)

clearOledArray()             →  displayManager->clearTextArray() (método privado)
writeOledArray(...)          →  displayManager->drawTextArray(...) (método privado)

5. Para las funciones de configuración:

writeOledBattery()           →  displayManager->setBatteryDisplay(true, batteryValue)
writeOledSpeedStepMultiplier() → displayManager->setSpeedMultiplierDisplay(true, multiplier)

6. Para acceso directo a funciones específicas de OLED (si es necesario):

U8G2* u8g2_ptr = static_cast<DisplayOLED_U8G2*>(displayInterface)->getU8G2Instance();

7. En deepSleepStart(), reemplazar:

u8g2.setPowerSave(1);        →  displayManager->powerSave(true);

VENTAJAS DEL NUEVO SISTEMA:

✓ Separación clara entre lógica de pantalla y lógica de aplicación
✓ Fácil agregar soporte para otros tipos de pantalla (TFT, LCD, etc.)
✓ Código más limpio y mantenible
✓ Nombres de funciones más descriptivos y genéricos
✓ Facilita testing y debugging
✓ Permite futuras optimizaciones sin cambiar la interfaz

TIPOS DE PANTALLA FUTUROS SOPORTADOS:

- TFT displays (ST7735, ILI9341, etc.)
- LCD character displays (16x2, 20x4, etc.)
- E-paper displays
- Pantallas táctiles
- Displays SPI/I2C genéricos

CONFIGURACIÓN PERSONALIZADA:

Para diferentes configuraciones de hardware, solo necesitas modificar
la creación del DisplayInterface en setup() sin cambiar el resto del código.

*/

#endif // DISPLAY_INTEGRATION_EXAMPLE_H