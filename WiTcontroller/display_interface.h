#ifndef DISPLAY_INTERFACE_H
#define DISPLAY_INTERFACE_H

// =====================================================
// Interfaz genérica para manejo de pantallas
// =====================================================

#include <Arduino.h>

// Tipos de pantalla soportados
enum DisplayType {
    DISPLAY_OLED_U8G2,
    DISPLAY_TFT_GENERIC,
    DISPLAY_LCD_GENERIC
    // Agregar más tipos según sea necesario
};

// Configuración de la pantalla
struct DisplayConfig {
    DisplayType type;
    int width;
    int height;
    int address;  // Para I2C
    int pinSDA;   // Para I2C
    int pinSCL;   // Para I2C
};

// Interfaz abstracta para pantallas
class DisplayInterface {
public:
    virtual ~DisplayInterface() {}
    
    // Funciones básicas de la pantalla
    virtual void begin() = 0;
    virtual void clear() = 0;
    virtual void display() = 0;
    virtual void powerSave(bool enable) = 0;
    
    // Funciones de dibujo básicas
    virtual void drawString(int x, int y, const String& text) = 0;
    virtual void drawLine(int x1, int y1, int x2, int y2) = 0;
    virtual void drawRectangle(int x, int y, int width, int height, bool filled = false) = 0;
    virtual void drawRoundRectangle(int x, int y, int width, int height, int radius, bool filled = false) = 0;
    virtual void drawGlyph(int x, int y, uint16_t glyph) = 0;
    
    // Funciones de texto
    virtual void setFont(const uint8_t* font) = 0;
    virtual void setFontSize(int size) = 0;
    virtual int getStringWidth(const String& text) = 0;
    
    // Funciones de color/inversión
    virtual void setDrawColor(int color) = 0;
    virtual void setInvertMode(bool invert) = 0;
    
    // Obtener dimensiones
    virtual int getWidth() = 0;
    virtual int getHeight() = 0;
};

// Factory para crear instancias de pantalla
class DisplayFactory {
public:
    static DisplayInterface* createDisplay(const DisplayConfig& config);
    
    // Función auxiliar para crear display OLED con instancia U8G2 existente
    static DisplayInterface* createOLEDDisplay(void* u8g2_instance);
};

#endif // DISPLAY_INTERFACE_H