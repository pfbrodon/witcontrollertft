# Instrucciones para Integrar TFT en WiTcontroller Original

## Método Recomendado: Modificación del Archivo Original

En lugar de usar un archivo completamente nuevo, es mejor modificar el `WiTcontroller.ino` original para que sea compatible con TFT. Esto mantiene toda la funcionalidad y facilita futuras actualizaciones.

## Pasos de Integración

### 1. Backup del Original
```bash
cp WiTcontroller.ino WiTcontroller_OLED_backup.ino
```

### 2. Modificaciones en WiTcontroller.ino

#### A. Cambiar los includes (líneas 23-24)
**REEMPLAZAR:**
```cpp
#include <U8g2lib.h>              // https://github.com/olikraus/u8g2
```

**POR:**
```cpp
// TFT support instead of OLED
#include <Adafruit_GFX.h>         // Core graphics library
#include <Adafruit_ST7735.h>      // Hardware-specific library for ST7735
#include <SPI.h>
#include "TFT_Modifications.h"    // TFT wrapper and functions
```

#### B. Cambiar include de configuración (línea 33)
**REEMPLAZAR:**
```cpp
#include "config_buttons.h"      // keypad buttons assignments
```

**POR:**
```cpp
#include "config_buttons_tft.h"  // TFT version of keypad buttons assignments
```

#### C. Agregar variables TFT después de las variables existentes (después de línea ~330)
**AGREGAR:**
```cpp
// TFT-specific variables
bool tftInitialized = false;
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS_PIN, TFT_DC_PIN, TFT_RST_PIN);
```

### 3. Modificaciones en la función setup()

#### En la función setup() (alrededor de línea 1636)
**REEMPLAZAR:**
```cpp
u8g2.begin();
u8g2.firstPage();
```

**POR:**
```cpp
// Initialize TFT instead of OLED
initializeTFT();
```

### 4. Modificaciones en las funciones de display

#### A. Función writeOledSpeed() (alrededor de línea 3400)
**BUSCAR la función writeOledSpeed() y AGREGAR al inicio:**
```cpp
void writeOledSpeed() {
  lastOledScreen = last_oled_screen_speed;
  menuIsShowing = false;
  
  // Use TFT enhanced display if available
  if (tftInitialized && TFT_ENHANCED_DISPLAY) {
    writeOledSpeedTFT();
    return;
  }
  
  // Original OLED code continues below...
```

#### B. Función writeOledArray() (alrededor de línea 3720)
**BUSCAR la función writeOledArray() y AGREGAR al inicio:**
```cpp
void writeOledArray(bool isThreeColumns, bool isPassword, bool sendBuffer, bool drawTopLine) {
  // Use TFT version if available
  if (tftInitialized && TFT_ENHANCED_DISPLAY) {
    writeOledArrayTFT(isThreeColumns, isPassword, sendBuffer, drawTopLine);
    return;
  }
  
  // Original OLED code continues below...
```

### 5. Compilar y Instalar Librerías

#### Librerías requeridas (Arduino IDE Library Manager):
1. `Adafruit GFX Library` by Adafruit
2. `Adafruit ST7735 and ST7789 Library` by Adafruit
3. Todas las librerías originales del WiTcontroller

#### Board: 
- `ESP32 Dev Module` o compatible

### 6. Configuración de Hardware

#### Conexiones TFT ST7735:
```
ESP32 GPIO 5  -> TFT CS   (configurable en config_buttons_tft.h)
ESP32 GPIO 2  -> TFT DC   (configurable en config_buttons_tft.h)  
ESP32 GPIO 4  -> TFT RST  (configurable en config_buttons_tft.h)
ESP32 GPIO 23 -> TFT MOSI (SPI estándar)
ESP32 GPIO 18 -> TFT SCLK (SPI estándar)
3.3V -> TFT VCC
GND  -> TFT GND
```

#### Otras conexiones permanecen igual:
- Encoder rotativo: GPIO 12, 14, 13
- Teclado: Según configuración original
- Batería: GPIO 34 (opcional)

## Archivos Necesarios

Copiar estos archivos al directorio WiTcontroller/:

1. ✅ `config_tft.h` - Configuración TFT
2. ✅ `config_buttons_tft.h` - Configuración de botones para TFT  
3. ✅ `TFT_Modifications.h` - Header con definiciones TFT
4. ✅ `TFT_Modifications.cpp` - Implementación de funciones TFT
5. `config_network.h` - Usar el mismo del proyecto original

## Características TFT vs OLED

### Mantenido del Original:
- ✅ Misma funcionalidad WiThrottle
- ✅ Mismo layout visual de 128x64
- ✅ Mismos menús y navegación
- ✅ Compatibilidad con todos los servidores

### Mejoras TFT:
- 🎨 Display a color (verde para velocidad, amarillo para dirección, etc.)
- 🔋 Indicador de batería mejorado con colores y porcentaje
- 📊 Funciones activas más visibles
- ⚡ Respuesta visual más rápida
- 🖥️ Tipografía más clara en pantalla más grande

## Configuración Personalizable

En `config_buttons_tft.h` se pueden modificar:

```cpp
// Colores (formato RGB565)
#define TFT_THEME_SPEED         0x07E0  // Verde
#define TFT_THEME_DIRECTION     0xFFE0  // Amarillo
#define TFT_THEME_FUNCTIONS     0x001F  // Azul
#define TFT_THEME_ERROR         0xF800  // Rojo

// Funciones mejoradas
#define TFT_ENHANCED_DISPLAY    true   // Usar mejoras de color
#define TFT_SHOW_BATTERY_PERCENT true  // Mostrar % de batería
#define TFT_LARGE_SPEED_DISPLAY true   // Fuentes grandes para velocidad
```

## Solución de Problemas

### Pantalla en blanco:
1. Verificar conexiones SPI
2. Verificar tipo de pantalla en config (BLACKTAB/GREENTAB/REDTAB)
3. Verificar alimentación (3.3V)

### Compilación:
1. Verificar que todas las librerías estén instaladas
2. Verificar que todos los archivos .h y .cpp estén en el directorio
3. Seleccionar board ESP32 correcto

### Funcionalidad:
1. Si algo no funciona, verificar que las variables originales no hayan cambiado
2. El sistema fallback debe usar las funciones OLED originales

## Reversión

Para volver a OLED:
1. Restaurar backup: `cp WiTcontroller_OLED_backup.ino WiTcontroller.ino`
2. Usar `config_buttons.h` original en lugar de `config_buttons_tft.h`

## Ventajas de este Método

1. **Compatibilidad total**: Mantiene toda la funcionalidad original
2. **Actualizaciones**: Fácil integrar nuevas versiones del WiTcontroller
3. **Fallback**: Si TFT falla, usa funciones OLED originales
4. **Modular**: Los cambios están aislados en archivos separados
5. **Reversible**: Fácil volver a la versión OLED