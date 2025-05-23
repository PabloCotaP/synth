#pragma once
#include <Arduino.h>
#include <Audio.h>

/*
 * EnvelopeManager
 * Encapsula un AudioEffectEnvelope de Teensy y permite configurar ADSR.
 */

class EnvelopeManager {
public:
    // Constructor con parámetros opcionales (valores por defecto)
    EnvelopeManager(float attack = 10.0, float decay = 50.0, float sustain = 0.7, float release = 200.0)
        : envelope()
    {
        envelope.attack(attack);
        envelope.decay(decay);
        envelope.sustain(sustain);
        envelope.release(release);
    }

    // Acceso al objeto envelope para conexiones de audio
    AudioEffectEnvelope* getEnvelope() { return &envelope; }

    // Métodos para cambiar parámetros en tiempo real
    void setADSR(float attack, float decay, float sustain, float release) {
        envelope.attack(attack);
        envelope.decay(decay);
        envelope.sustain(sustain);
        envelope.release(release);
    }

private:
    AudioEffectEnvelope envelope;
};