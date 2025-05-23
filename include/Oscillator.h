#pragma once
#include <Arduino.h>
#include <Audio.h>
#include "Envelope.h"
#include "WaveformType.h"

/*
    Clase OscillatorManager
    Gestiona un oscilador y su envolvente (una voz polifónica).
*/

class OscillatorManager {
public:
    // Constructor: recibe el puntero al objeto AudioSynthWaveform y EnvelopeManager
    OscillatorManager(AudioSynthWaveform* waveform, EnvelopeManager* env)
        : waveform(waveform), env(env), currentWaveformType(SINE) {}

    // Cambia el tipo de oscilador
    void setOscillatorType(int type) {
        currentWaveformType = type;
    }

    // Cambia la frecuencia del oscilador según la nota MIDI y lo enciende
    void playNote(uint8_t midiNote) {
        float freq = midiToFreq(midiNote);
        waveform->frequency(freq);
        waveform->amplitude(0.7); // Ajusta la amplitud según necesites
        waveform->begin(currentWaveformType);
        if (env) {
            env->getEnvelope()->noteOn();
        }
    }

    // Detiene la nota actual (apaga el oscilador)
    void stopNote() {
        if (env) {
            env->getEnvelope()->noteOff();
        }
        waveform->amplitude(0.0);
    }

    // Devuelve el objeto AudioSynthWaveform asociado al oscilador
    AudioSynthWaveform* getWaveform() {
        return waveform;
    }

    // Devuelve el EnvelopeManager asociado
    EnvelopeManager* getEnvelopeManager() {
        return env;
    }

private:
    // Convierte una nota MIDI a frecuencia en Hz
    float midiToFreq(uint8_t midiNote) {
        return 440.0 * pow(2.0, (midiNote - 69) / 12.0);
    }

    AudioSynthWaveform* waveform; // Oscilador de Teensy Audio
    EnvelopeManager* env;         // Envolvente asociada a esta voz
    int currentWaveformType;      // Tipo de oscilador actual
};