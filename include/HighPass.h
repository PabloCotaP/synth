#pragma once
#include <Arduino.h>
#include <Audio.h>

/*
 * HighPassFilterManager
 * Encapsula un filtro de paso alto usando AudioFilterStateVariable de Teensy.
 * Permite configurar la frecuencia de corte y la resonancia.
 */
class HighPassFilterManager {
public:
    // Constructor con parámetros opcionales (valores por defecto)
    HighPassFilterManager(float cutoff = 500, float resonance = 0.7)
        : filter()
    {
        filter.frequency(cutoff);
        this->cutoff = cutoff;
        filter.resonance(resonance);
        this->resonance = resonance;
    }

    // Acceso al objeto filtro para conexiones de audio
    AudioFilterStateVariable* getFilter() { return &filter; }

    // Cambia los parámetros del filtro en tiempo real
    void setParams(float cutoff, float resonance) {
        filter.frequency(cutoff);
        this->cutoff = cutoff;
        filter.resonance(resonance);
        this->resonance = resonance;
    }

    float GetCutoff() {
        return cutoff;
    }

    float GetResonance() {
        return resonance;
    }

    // Llama a este método cuando se toca una nota (opcional, para automatización)
    void noteOn() {
        // Aquí puedes automatizar el filtro si lo deseas
    }

    // Llama a este método cuando se suelta la nota (opcional)
    void noteOff() {
        // Aquí puedes automatizar el filtro si lo deseas
    }

private:
    AudioFilterStateVariable filter;
    float cutoff;
    float resonance;
};