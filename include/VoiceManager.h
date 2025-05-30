#pragma once
#include <Arduino.h>
#include "Voice.h"

template <int NUM_VOICES>
class VoiceManager {
public:
    VoiceManager(AudioMixer4& mixer, AudioSynthWaveform* waves[], AudioEffectEnvelope* envs[])
        : mixer(mixer) {
        for (int i = 0; i < NUM_VOICES; ++i) {
            voices[i] = new Voice(waves[i], envs[i]);
            mixer.gain(i, 0.0);
            voiceStartTime[i] = 0;
        }
    }

    void noteOn(uint8_t note) {
        // Si la nota ya está activa, no hacer nada
        for (int i = 0; i < NUM_VOICES; ++i) {
            if (voices[i]->isActive() && voices[i]->getNote() == note) return;
        }
        // Busca una voz libre
        for (int i = 0; i < NUM_VOICES; ++i) {
            if (!voices[i]->isActive()) {
                voices[i]->noteOn(note);
                mixer.gain(i, 0.2);
                voiceStartTime[i] = millis();
                return;
            }
        }
        // Si no hay voces libres, roba la más antigua
        int oldest = 0;
        unsigned long oldestTime = voiceStartTime[0];
        for (int i = 1; i < NUM_VOICES; ++i) {
            if (voiceStartTime[i] < oldestTime) {
                oldest = i;
                oldestTime = voiceStartTime[i];
            }
        }
        voices[oldest]->noteOff(); // Apaga la voz antigua (opcional, por claridad)
        voices[oldest]->noteOn(note);
        mixer.gain(oldest, 0.2);
        voiceStartTime[oldest] = millis();
    }

    void noteOff(uint8_t note) {
        for (int i = 0; i < NUM_VOICES; ++i) {
            if (voices[i]->isActive() && voices[i]->getNote() == note) {
                voices[i]->noteOff();
                return;
            }
        }
    }

    void update() {
        for (int i = 0; i < NUM_VOICES; ++i) {
            voices[i]->update();
            if (!voices[i]->isActive()) {
                mixer.gain(i, 0.0);
            }
        }
    }

    int getActiveVoices() const {
        int count = 0;
        for (int i = 0; i < NUM_VOICES; ++i) {
            if (voices[i]->isActive()) ++count;
        }
        return count;
    }

    void debugStatus() {
        Serial.println("===== Estado de Voces =====");
        for (int i = 0; i < NUM_VOICES; ++i) {
            Voice* v = voices[i];
            Serial.print("Voz ");
            Serial.print(i);
            Serial.print(v->isActive() ? " [ACTIVA] " : " [apagada] ");
            Serial.print("Nota: ");
            Serial.println(v->getNote());
        }
    }

private:
    Voice* voices[NUM_VOICES];
    AudioMixer4& mixer;
    unsigned long voiceStartTime[NUM_VOICES]; // Marca de tiempo de activación de cada voz
};