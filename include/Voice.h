#pragma once
#include <Arduino.h>
#include <Audio.h>
#include "Envelope.h"
#include "WaveformType.h"

class Voice {
public:
    Voice(AudioSynthWaveform* wave, AudioEffectEnvelope* env)
        : waveform(wave), envelope(env), active(false), note(0) {
        waveform->begin(SINE);
        waveform->amplitude(0.0);
    }

    void noteOn(uint8_t midiNote, int waveformType = SINE) {
        waveform->begin(waveformType);
        waveform->frequency(midiToFreq(midiNote));
        waveform->amplitude(1.0);
        envelope->noteOn();
        note = midiNote;
        active = true;
    }

    void noteOff() {
        envelope->noteOff();
    }

    void update() {
        if (envelope->isActive()) {
            waveform->amplitude(1.0);
        } else {
            waveform->amplitude(0.0);
            active = false;
        }
    }

    bool isActive() const { return active; }
    uint8_t getNote() const { return note; }

private:
    AudioSynthWaveform* waveform;
    AudioEffectEnvelope* envelope;
    bool active;
    uint8_t note;

    float midiToFreq(uint8_t midiNote) {
        return 440.0 * pow(2.0, (midiNote - 69) / 12.0);
    }
};
