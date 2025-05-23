#pragma once
#include <Arduino.h>
#include "Oscillator.h"

// Permite hasta 3 notas simultáneas
class PolyphonyManager {
public:
    PolyphonyManager(OscillatorManager* osc)
        : osc(osc) {}

    // Toca una nota solo si no está ya activa
    void noteOn(uint8_t note) {
        // ¿Ya está activa esta nota?
        for (int i = 0; i < 3; ++i) {
            if (active[i] && notes[i] == note) return;
        }
        // Busca una voz libre
        for (int i = 0; i < 3; ++i) {
            if (!active[i]) {
                osc[i].playNote(note);
                notes[i] = note;
                active[i] = true;
                return;
            }
        }
        // Si no hay voces libres, ignora la nota extra
    }

    // Apaga solo la voz que tiene esa nota
    void noteOff(uint8_t note) {
        for (int i = 0; i < 3; ++i) {
            if (active[i] && notes[i] == note) {
                osc[i].stopNote();
                active[i] = false;
                notes[i] = 0;
                return;
            }
        }
    }

private:
    OscillatorManager* osc;
    uint8_t notes[3] = {0, 0, 0};
    bool active[3] = {false, false, false};
};