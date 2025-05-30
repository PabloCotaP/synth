#pragma once
#include <Audio.h>

class VoiceRouter {
public:
    VoiceRouter(AudioSynthWaveform* waveforms[], AudioEffectEnvelope* envelopes[],
                AudioFilterStateVariable* hpFilters[], AudioFilterStateVariable* lpFilters[],
                AudioMixer4& mixer, int numVoices)
        : mixer(mixer), numVoices(numVoices), filtersEnabled(false) {
        for (int i = 0; i < numVoices; ++i) {
            osc[i] = waveforms[i];
            env[i] = envelopes[i];
            hp[i] = hpFilters[i];
            lp[i] = lpFilters[i];

            patchOscToEnv[i] = nullptr;
            patchEnvToHP[i] = nullptr;
            patchHPToLP[i] = nullptr;
            patchLPToMixer[i] = nullptr;
            patchEnvToMixer[i] = nullptr;
        }
    }

    void connectVoices() {
        for (int i = 0; i < numVoices; ++i) {
            patchOscToEnv[i] = new AudioConnection(*osc[i], 0, *env[i], 0);
        }
    }

    void enableFilters() {
        if (filtersEnabled) return;

        disconnectDirect();

        for (int i = 0; i < numVoices; ++i) {
            patchEnvToHP[i] = new AudioConnection(*env[i], 0, *hp[i], 0);
            patchHPToLP[i] = new AudioConnection(*hp[i], 0, *lp[i], 0);
            patchLPToMixer[i] = new AudioConnection(*lp[i], 0, mixer, i);
        }

        filtersEnabled = true;
    }

    void disableFilters() {
        if (!filtersEnabled) return;

        for (int i = 0; i < numVoices; ++i) {
            delete patchEnvToHP[i];  patchEnvToHP[i] = nullptr;
            delete patchHPToLP[i];   patchHPToLP[i] = nullptr;
            delete patchLPToMixer[i]; patchLPToMixer[i] = nullptr;
        }

        connectDirect();
        filtersEnabled = false;
    }

    bool areFiltersEnabled() const {
        return filtersEnabled;
    }

private:
    AudioSynthWaveform* osc[4];
    AudioEffectEnvelope* env[4];
    AudioFilterStateVariable* hp[4];
    AudioFilterStateVariable* lp[4];
    AudioMixer4& mixer;
    int numVoices;
    bool filtersEnabled;

    // Conexiones dinámicas
    AudioConnection* patchOscToEnv[4];
    AudioConnection* patchEnvToHP[4];
    AudioConnection* patchHPToLP[4];
    AudioConnection* patchLPToMixer[4];
    AudioConnection* patchEnvToMixer[4];

    void connectDirect() {
        for (int i = 0; i < numVoices; ++i) {
            patchEnvToMixer[i] = new AudioConnection(*env[i], 0, mixer, i);
        }
    }

    void disconnectDirect() {
        for (int i = 0; i < numVoices; ++i) {
            delete patchEnvToMixer[i];
            patchEnvToMixer[i] = nullptr;
        }
    }
};
