#pragma once
#include <Arduino.h>
#include "Voice.h"

template <int NUM_VOICES>
class VoiceManager {
public:
    VoiceManager(AudioMixer4& mixer) : mixer(mixer) {
        for (int i = 0; i < NUM_VOICES; ++i) {
            mixer.gain(i, 0.2);
            patch[i] = new AudioConnection(*voices[i].getEnvelope(), 0, mixer, i);
        }
    }

    void noteOn(uint8_t note) {
        // Si ya está activa, ignora
        for (int i = 0; i < NUM_VOICES; ++i) {
            if (voices[i].isActive() && voices[i].getNote() == note) return;
        }
        // Busca una voz libre
        for (int i = 0; i < NUM_VOICES; ++i) {
            if (!voices[i].isActive()) {
                voices[i].noteOn(note);
                return;
            }
        }
    }

    void noteOff(uint8_t note) {
        for (int i = 0; i < NUM_VOICES; ++i) {
            if (voices[i].isActive() && voices[i].getNote() == note) {
                voices[i].noteOff();
                return;
            }
        }
    }

private:
    Voice voices[NUM_VOICES];
    AudioMixer4& mixer;
    AudioConnection* patch[NUM_VOICES];
};