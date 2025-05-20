#pragma once
#include <Audio.h>

class Oscillator {
public:
    void begin() {
        sineWave.amplitude(0);  // Inicia silenciado
    }

    void noteOn(float freq, float velocity) {
        sineWave.frequency(freq);
        sineWave.amplitude(velocity);  // Velocidad MIDI (0-1)
    }

    void noteOff() {
        sineWave.amplitude(0);  // Silencia el oscilador
    }

private:
    AudioSynthWaveformSine sineWave;  // Objeto de audio
};