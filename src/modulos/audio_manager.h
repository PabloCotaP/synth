#pragma once
#include <Audio.h>

class AudioManager {
public:
    void begin() {
        AudioMemory(12);  // Reserva bloques de memoria
        // Configuración adicional del Audio Shield (si es necesario)
    }

    AudioOutputI2S audioOutput;  // Objeto de salida
};