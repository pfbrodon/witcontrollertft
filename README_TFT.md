# WiTcontroller TFT Version

Esta es una versión modificada del WiTcontroller original que utiliza una pantalla TFT ST7735 RGB en lugar de la pantalla OLED monocromática, manteniendo el aspecto y funcionalidad de la pantalla original pero con mejoras visuales usando colores.

## Cambios Principales

### Hardware
- **Pantalla**: TFT ST7735 1.8" 128x160 RGB en lugar de OLED SSD1306 128x64
- **Conexiones**: SPI en lugar de I2C para la pantalla
- **Pinout**: Nuevos pines para CS, DC, RST de la pantalla TFT

### Software
- **Librerías**: Adafruit GFX y Adafruit ST7735 en lugar de U8g2
- **Wrapper**: Sistema de compatibilidad que traduce llamadas u8g2 a TFT
- **Colores**: Interfaz colorizada para mejor usabilidad
- **Layout**: Mantiene el layout original de 128x64 pero aprovecha la pantalla más grande

## Archivos Nuevos/Modificados

### Archivos de Configuración TFT
- `config_tft.h` - Configuración principal para TFT
- `config_buttons_tft.h` - Configuración de botones adaptada para TFT
- `tft_wrapper.cpp` - Wrapper que traduce funciones u8g2 a TFT

### Archivo Principal
- `WiTcontroller_TFT.ino` - Versión modificada del archivo principal

## Configuración de Hardware

### Conexiones TFT ST7735
```
ESP32    ->  TFT ST7735
GPIO 5   ->  CS  (Chip Select)
GPIO 2   ->  DC  (Data/Command)
GPIO 4   ->  RST (Reset)
GPIO 23  ->  SDA/MOSI (Data)
GPIO 18  ->  SCL/SCLK (Clock)
3.3V     ->  VCC
GND      ->  GND
```

### Otras Conexiones (sin cambios)
- Encoder rotativo: GPIO 12, 14, 13
- Teclado 3x4: Según configuración original
- Batería: GPIO 34 (opcional)

## Librerías Requeridas

Instalar mediante el Library Manager de Arduino IDE:

### Librerías Originales (sin cambios)
- `WiThrottleProtocol` by flash62au
- `AiEsp32RotaryEncoder` by Igor Antolic
- `Keypad` by Mark Stanley

### Librerías TFT (nuevas)
- `Adafruit GFX Library` by Adafruit
- `Adafruit ST7735 and ST7789 Library` by Adafruit

### Librerías ESP32 (incluidas con ESP32 board package)
- `WiFi`
- `ESPmDNS`
- `SPI`

## Configuración

### Archivo config_buttons_tft.h
```cpp
// Habilitar soporte TFT
#define USE_TFT_DISPLAY true

// Pines de hardware TFT
#define TFT_CS_PIN    5   // Chip select
#define TFT_RST_PIN   4   // Reset
#define TFT_DC_PIN    2   // Data/Command

// Tipo de pantalla
#define TFT_DISPLAY_TYPE    ST7735_BLACKTAB

// Tema de colores (personalizable)
#define TFT_THEME_BACKGROUND     0x0000  // Negro
#define TFT_THEME_TEXT          0xFFFF  // Blanco  
#define TFT_THEME_SPEED         0x07E0  // Verde
#define TFT_THEME_DIRECTION     0xFFE0  // Amarillo
#define TFT_THEME_FUNCTIONS     0x001F  // Azul

// Funciones mejoradas
#define TFT_ENHANCED_DISPLAY    true
#define TFT_SHOW_BATTERY_PERCENT true
#define TFT_LARGE_SPEED_DISPLAY true
```

### Archivo config_network.h
Usar el mismo que en la versión original con tus credenciales WiFi.

## Funciones Mejoradas

### Visualización en Color
- **Velocidad**: Verde en texto grande
- **Dirección**: Amarillo para adelante, naranja para atrás
- **Funciones**: Azul para funciones activas
- **Batería**: Verde/amarillo/rojo según nivel
- **Errores**: Rojo para mensajes de error
- **Menús**: Gris claro para mejor legibilidad

### Layout Optimizado
- Mantiene compatibilidad visual con la versión OLED
- Utiliza el área de 128x64 de la pantalla TFT de 128x160
- Tipografía más clara y legible
- Indicadores de estado más visibles

### Funciones Adicionales
- Porcentaje de batería visible
- Nombres de locomotoras (si disponible)
- Mayor número de throttles soportados
- Respuesta visual más rápida

## Compilación e Instalación

1. **Preparar Arduino IDE**:
   - Instalar soporte para ESP32
   - Instalar librerías requeridas

2. **Configurar archivos**:
   - Copiar `config_network.h` de la versión original
   - Usar `config_buttons_tft.h` como base de configuración
   - Modificar pines según tu hardware

3. **Compilar**:
   - Abrir `WiTcontroller_TFT.ino`
   - Seleccionar board: "ESP32 Dev Module" o compatible
   - Compilar y cargar

## Solución de Problemas

### Pantalla en Blanco
- Verificar conexiones SPI
- Verificar tipo de pantalla (BLACKTAB, GREENTAB, REDTAB)
- Verificar alimentación (3.3V)

### Colores Incorrectos
- Ajustar configuración de rotación
- Verificar tipo de pantalla ST7735
- Ajustar valores de color RGB565

### Funcionalidad Original No Funciona
- Verificar que `tft_wrapper.cpp` está incluido
- Verificar que las funciones u8g2 están siendo remapeadas correctamente

## Personalización

### Cambiar Colores
Modificar las definiciones en `config_buttons_tft.h`:
```cpp
#define TFT_THEME_SPEED         0x07E0  // Verde (RGB565)
#define TFT_THEME_DIRECTION     0xFFE0  // Amarillo
// ... etc
```

### Cambiar Layout
Modificar constantes en `config_tft.h`:
```cpp
#define SPEED_X             45  // Posición X de velocidad
#define SPEED_Y             28  // Posición Y de velocidad
// ... etc
```

### Añadir Funciones TFT
Usar funciones nativas de Adafruit GFX para funciones adicionales:
```cpp
tft.drawLine(x1, y1, x2, y2, color);
tft.drawCircle(x, y, radius, color);
tft.fillTriangle(x1, y1, x2, y2, x3, y3, color);
```

## Compatibilidad

- **Hardware**: Compatible con ESP32 y pantallas ST7735
- **Software**: Mantiene compatibilidad con servidores WiThrottle
- **Configuración**: La mayoría de opciones del WiTcontroller original funcionan
- **Actualización**: Se puede actualizar desde versión OLED cambiando hardware y software

## Créditos

- **WiTcontroller Original**: flash62au (Peter A)
- **Adaptación TFT**: Versión modificada que mantiene funcionalidad original
- **Librerías**: Adafruit (GFX, ST7735), Arduino Community