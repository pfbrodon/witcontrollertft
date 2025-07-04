# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Descripción del Proyecto

WiTcontroller es un controlador manual DIY basado en ESP32 para trenes de modelismo que se comunica con servidores WiThrottle (JMRI, DCC-EX, etc.) usando el protocolo WiThrottle. El proyecto está construido usando PlatformIO y el framework de Arduino.

## Entorno de Desarrollo

### Compilación y Carga
- **Plataforma**: PlatformIO con plataforma ESP32
- **Placa**: LOLIN32 Lite (ESP32)
- **Framework**: Arduino
- **Comando de compilación**: `pio run`
- **Comando de carga**: `pio run --target upload`
- **Monitor**: `pio device monitor` (115200 baud)

### Dependencias
El proyecto usa la gestión de librerías de PlatformIO. Las librerías clave están definidas en `platformio.ini`:
- Librería Keypad para entrada de teclado 3x4/4x4
- U8g2 para control de pantalla OLED
- AiEsp32RotaryEncoder para manejo del encoder rotativo
- WiThrottleProtocol para comunicación DCC
- Librerías integradas de WiFi y mDNS del ESP32

## Arquitectura del Código

### Componentes Principales
- **WiTcontroller.ino**: Archivo principal de aplicación con setup() y loop()
- **WiTcontroller.h**: Header principal con declaraciones globales y prototipos de funciones
- **actions.h**: Define constantes de acción para mapeo de botones/teclas
- **static.h**: Strings estáticos e información de versión
- **config_keypad_etc.h**: Definiciones de pines del hardware y configuración del teclado
- **Pangodream_18650_CL.h/.cpp**: Librería de monitoreo de batería

### Sistema de Configuración
El proyecto usa un enfoque de configuración de doble archivo:
- **config_network.h**: Configuración de redes WiFi (copiar de config_network_example.h)
- **config_buttons.h**: Mapeo de botones y configuración de características (copiar de config_buttons_example.h)

Estos archivos son creados por el usuario y no están rastreados en git para preservar configuraciones personalizadas durante actualizaciones.

### Características Principales
- Soporte multi-throttle (hasta 6 throttles)
- Encoder rotativo para control de velocidad con soporte opcional de potenciómetro
- Teclado 3x4 o 4x4 para entrada de comandos
- Pantalla OLED (0.96", 1.3", o 2.4" soportadas)
- Monitoreo opcional de batería
- Mapeo configurable de botones de función
- Soporte de múltiples idiomas (alemán, italiano incluidos)

### Arquitectura del Hardware
- Microcontrolador ESP32 con WiFi
- Pantalla OLED I2C en pines 22 (SCL) y 23 (SDA)
- Encoder rotativo en pines 12 (DT), 14 (CLK), 13 (SW)
- Teclado matricial en pines 19,18,17,16 (filas) y 4,0,2 (columnas)
- Botones adicionales opcionales en varios pines GPIO
- Monitoreo opcional de batería en pin 34

### Arquitectura Modular (Refactorizada)
El proyecto ha sido refactorizado en módulos separados para mejorar la mantenibilidad:

#### Módulos Principales:
- **wifi_manager.cpp/.h**: Gestión de conexiones WiFi y SSIDs (≈25 funciones)
- **wit_server.cpp/.h**: Comunicación con servidores WiThrottle (≈20 funciones)
- **display_manager.cpp/.h**: Control de pantalla OLED y UI (≈15 funciones)
- **input_handler.cpp/.h**: Gestión de entrada de usuario (≈20 funciones)
- **roster_manager.cpp/.h**: Gestión de locomotoras y funciones (≈10 funciones)
- **preferences_manager.cpp/.h**: Configuración persistente (≈5 funciones)
- **battery_monitor.cpp/.h**: Monitoreo de batería (≈3 funciones)

#### Beneficios de la Modularización:
- **WiTcontroller.ino** reducido de 3778 a ~200 líneas
- Separación clara de responsabilidades
- Facilita el mantenimiento y testing
- Permite desarrollo colaborativo
- Mejora la reutilización de código

## Flujo de Trabajo de Desarrollo

### Pruebas
- No hay framework de pruebas automatizadas configurado
- Las pruebas se realizan principalmente mediante verificación de hardware
- Usar monitor serie para depuración con el define `WITCONTROLLER_DEBUG`

### Cambios de Configuración
- Siempre modificar los archivos de configuración del usuario (config_network.h, config_buttons.h) no los archivos de ejemplo
- Las asignaciones de pines pueden ser sobreescritas en config_buttons.h
- Las banderas de características se controlan mediante #defines en config_buttons.h

### Gestión de Versiones
- La versión está definida en static.h como `appVersion`
- El registro de cambios se mantiene en change_log.md
- La compatibilidad de versiones de librerías está documentada en README.md

## Notas Importantes

- El proyecto soporta Arduino IDE pero PlatformIO es el entorno de desarrollo preferido
- Las asignaciones de pines del hardware son críticas - verificar que las conexiones coincidan con la configuración
- WiFi es solo 2.4GHz (limitación del ESP32)
- La polaridad de la batería debe ser verificada antes de la conexión
- El proyecto incluye características de seguridad como parada de emergencia y control de energía