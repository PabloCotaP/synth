#include "OscillatorManager.h"

OscillatorManager::OscillatorManager() {
    currentWaveformType = SINE; // Tipo de onda por defecto
}

void OscillatorManager::setOscillatorType(int type) {
    currentWaveformType = type; // Cambia el tipo de oscilador
}

void OscillatorManager::playNote(uint8_t midiNote) {
    float freq = midiToFreq(midiNote); // Convierte la nota MIDI a frecuencia
    waveform->frequency(freq); // Cambia la frecuencia del oscilador
    waveform->amplitude(0.5); // Establece la amplitud del oscilador. Esto puede venir de un potenciómetro más adelante
    waveform->begin(currentWaveformType); // Inicializa el oscilador con el tipo de onda actual
}

void OscillatorManager::stopNote() {
    waveform->amplitude(0.0); // Apaga el oscilador
}

AudioSynthWaveform* OscillatorManager::getWaveform() {
    return waveform; // Devuelve el objeto AudioSynthWaveform asociado al oscilador
}

float OscillatorManager::midiToFreq(uint8_t midiNote) {
    // Convierte la nota MIDI a frecuencia en Hz
    return 440.0 * pow(2.0, (midiNote - 69) / 12.0);
}