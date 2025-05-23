#pragma once
#include <Arduino.h>
#include <Audio.h>

/*
 * LowPassFilterManager
 * Encapsula un filtro de paso bajo usando AudioFilterStateVariable de Teensy.
 * Permite configurar la frecuencia de corte y la resonancia.
 */
class LowPassFilterManager {
public:
    // Constructor con parámetros opcionales (valores por defecto)
    LowPassFilterManager(float cutoff = 500.0, float resonance = 0.7)
        : filter()
    {
        filter.frequency(cutoff);
        cutoff = cutoff;
        filter.resonance(resonance);
        resonance = resonance;
    }

    // Acceso al objeto filtro para conexiones de audio
    AudioFilterStateVariable* getFilter() { return &filter; }

    // Cambia los parámetros del filtro en tiempo real
    void setParams(float cutoff, float resonance) {
        filter.frequency(cutoff);
        cutoff = cutoff;
        filter.resonance(resonance);
        resonance = resonance;
    }

    float GetCutoff() {
        return cutoff;
    }

    float GetResonance() {
        return resonance;
    }

    // Métodos opcionales para automatización con notas
    void noteOn() {}
    void noteOff() {}

private:
    AudioFilterStateVariable filter;
    float cutoff;
    float resonance;
};