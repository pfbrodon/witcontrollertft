#include "display_interface.h"
#include "display_oled_u8g2.h"

// =====================================================
// Factory para crear diferentes tipos de pantalla
// =====================================================

DisplayInterface* DisplayFactory::createDisplay(const DisplayConfig& config) {
    switch (config.type) {
        case DISPLAY_OLED_U8G2: {
            // Este caso requiere que se pase la instancia de U8G2 desde el código principal
            // Por ahora retornamos nullptr, se manejará en el código principal
            return nullptr;
        }
        
        case DISPLAY_TFT_GENERIC:
            // Implementar cuando se agregue soporte TFT
            return nullptr;
            
        case DISPLAY_LCD_GENERIC:
            // Implementar cuando se agregue soporte LCD
            return nullptr;
            
        default:
            return nullptr;
    }
}

// Función auxiliar para crear display OLED con instancia U8G2 existente
DisplayInterface* DisplayFactory::createOLEDDisplay(void* u8g2_instance) {
    if (u8g2_instance) {
        return new DisplayOLED_U8G2(static_cast<U8G2*>(u8g2_instance));
    }
    return nullptr;
}