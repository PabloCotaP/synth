#pragma once
#include <Arduino.h>
#include <Audio.h>

/*
 * BitCrusherManager
 * Encapsula un efecto bitcrusher usando AudioEffectBitcrusher de Teensy.
 * Permite configurar la profundidad de bits y la frecuencia de muestreo.
 */
class BitCrusherManager {
public:
    // Constructor con parámetros opcionales (valores por defecto)
    BitCrusherManager(uint8_t bits = 8, float sampleRate = 44100.0)
        : bitcrusher()
    {
        bitcrusher.bits(bits);
        bitcrusher.sampleRate(sampleRate);
    }

    // Acceso al objeto bitcrusher para conexiones de audio
    AudioEffectBitcrusher* getBitcrusher() { return &bitcrusher; }

    // Cambia los parámetros del bitcrusher en tiempo real
    void setParams(uint8_t bits, float sampleRate) {
        bitcrusher.bits(bits);
        bitcrusher.sampleRate(sampleRate);
    }

    uint8_t GetBits() {
        return bits;
    }

    float GetSampleRate() {
        return sampleRate;
    }

private:
    AudioEffectBitcrusher bitcrusher;
    uint8_t bits;
    float sampleRate;
};