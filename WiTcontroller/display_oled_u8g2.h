#ifndef DISPLAY_OLED_U8G2_H
#define DISPLAY_OLED_U8G2_H

#include "display_interface.h"
#include <U8g2lib.h>

// =====================================================
// Implementación específica para pantallas OLED con U8g2
// =====================================================

class DisplayOLED_U8G2 : public DisplayInterface {
private:
    U8G2* u8g2_instance;
    
public:
    DisplayOLED_U8G2(U8G2* u8g2_ptr);
    virtual ~DisplayOLED_U8G2();
    
    // Implementación de la interfaz
    void begin() override;
    void clear() override;
    void display() override;
    void powerSave(bool enable) override;
    
    void drawString(int x, int y, const String& text) override;
    void drawLine(int x1, int y1, int x2, int y2) override;
    void drawRectangle(int x, int y, int width, int height, bool filled = false) override;
    void drawRoundRectangle(int x, int y, int width, int height, int radius, bool filled = false) override;
    void drawGlyph(int x, int y, uint16_t glyph) override;
    
    void setFont(const uint8_t* font) override;
    void setFontSize(int size) override;
    int getStringWidth(const String& text) override;
    
    void setDrawColor(int color) override;
    void setInvertMode(bool invert) override;
    
    int getWidth() override;
    int getHeight() override;
    
    // Funciones específicas para U8G2 si son necesarias
    U8G2* getU8G2Instance() { return u8g2_instance; }
};

#endif // DISPLAY_OLED_U8G2_H