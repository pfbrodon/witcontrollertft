#include "display_oled_u8g2.h"

// =====================================================
// Implementación de DisplayOLED_U8G2
// =====================================================

DisplayOLED_U8G2::DisplayOLED_U8G2(U8G2* u8g2_ptr) : u8g2_instance(u8g2_ptr) {
}

DisplayOLED_U8G2::~DisplayOLED_U8G2() {
    // No eliminamos u8g2_instance ya que se maneja externamente
}

void DisplayOLED_U8G2::begin() {
    if (u8g2_instance) {
        u8g2_instance->begin();
        u8g2_instance->firstPage();
    }
}

void DisplayOLED_U8G2::clear() {
    if (u8g2_instance) {
        u8g2_instance->clearBuffer();
    }
}

void DisplayOLED_U8G2::display() {
    if (u8g2_instance) {
        u8g2_instance->sendBuffer();
    }
}

void DisplayOLED_U8G2::powerSave(bool enable) {
    if (u8g2_instance) {
        u8g2_instance->setPowerSave(enable ? 1 : 0);
    }
}

void DisplayOLED_U8G2::drawString(int x, int y, const String& text) {
    if (u8g2_instance) {
        u8g2_instance->drawUTF8(x, y, text.c_str());
    }
}

void DisplayOLED_U8G2::drawLine(int x1, int y1, int x2, int y2) {
    if (u8g2_instance) {
        u8g2_instance->drawLine(x1, y1, x2, y2);
    }
}

void DisplayOLED_U8G2::drawRectangle(int x, int y, int width, int height, bool filled) {
    if (u8g2_instance) {
        if (filled) {
            u8g2_instance->drawBox(x, y, width, height);
        } else {
            u8g2_instance->drawFrame(x, y, width, height);
        }
    }
}

void DisplayOLED_U8G2::drawRoundRectangle(int x, int y, int width, int height, int radius, bool filled) {
    if (u8g2_instance) {
        if (filled) {
            u8g2_instance->drawRBox(x, y, width, height, radius);
        } else {
            u8g2_instance->drawRFrame(x, y, width, height, radius);
        }
    }
}

void DisplayOLED_U8G2::drawGlyph(int x, int y, uint16_t glyph) {
    if (u8g2_instance) {
        u8g2_instance->drawGlyph(x, y, glyph);
    }
}

void DisplayOLED_U8G2::setFont(const uint8_t* font) {
    if (u8g2_instance) {
        u8g2_instance->setFont(font);
    }
}

void DisplayOLED_U8G2::setFontSize(int size) {
    // U8G2 no tiene setFontSize directo, se maneja por setFont
    // Esta función se puede extender para mapear tamaños a fuentes específicas
}

int DisplayOLED_U8G2::getStringWidth(const String& text) {
    if (u8g2_instance) {
        return u8g2_instance->getStrWidth(text.c_str());
    }
    return 0;
}

void DisplayOLED_U8G2::setDrawColor(int color) {
    if (u8g2_instance) {
        u8g2_instance->setDrawColor(color);
    }
}

void DisplayOLED_U8G2::setInvertMode(bool invert) {
    // U8G2 maneja esto a través de setDrawColor y drawBox
    // Se puede implementar una lógica más específica aquí
}

int DisplayOLED_U8G2::getWidth() {
    if (u8g2_instance) {
        return u8g2_instance->getDisplayWidth();
    }
    return 128; // valor por defecto
}

int DisplayOLED_U8G2::getHeight() {
    if (u8g2_instance) {
        return u8g2_instance->getDisplayHeight();
    }
    return 64; // valor por defecto
}