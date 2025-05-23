#pragma once
#include <Arduino.h>
#include <Audio.h>

/*
 * FlangerManager
 * Encapsula un efecto flanger usando AudioEffectFlange de Teensy.
 * Permite configurar offset, depth y delayRate.
 */
class FlangerManager {
public:
    // Constructor con parámetros opcionales (valores por defecto)
    FlangerManager(float offset = 4.0f, float depth = 4.0f, float rate = 0.2f)
        : flanger(), offset(offset), depth(depth), rate(rate)
    {
        // El buffer debe ser suficientemente grande para el delay deseado
        flanger.begin(delayBuffer, bufferSize, offset, depth, rate);
    }

    // Cambia los parámetros del flanger en tiempo real
    void setParams(float newOffset, float newDepth, float newRate) {
        offset = newOffset;
        depth = newDepth;
        rate = newRate;
        flanger.voices(offset, depth, rate);
    }

    void setOffset(float o)   { offset = o; flanger.voices(offset, depth, rate); }
    void setDepth(float d)    { depth = d;  flanger.voices(offset, depth, rate); }
    void setRate(float r)     { rate = r;   flanger.voices(offset, depth, rate); }

    float getOffset() const { return offset; }
    float getDepth()  const { return depth; }
    float getRate()   const { return rate; }

    AudioEffectFlange* getFlanger() { return &flanger; }

private:
    static const int bufferSize = 128; // Ajusta según el máximo delay que quieras (múltiplo de AUDIO_BLOCK_SAMPLES)
    AudioEffectFlange flanger;
    float offset;
    float depth;
    float rate;
    short delayBuffer[bufferSize];
};