#pragma once
#include <Arduino.h>
#include <Audio.h>
#include "WaveformType.h"

/*
    Clase OscillatorManager
    Esta clase se encarga de gestionar un oscilador de audio.

    Funciones principales:
    - Seleccionar el tipo de oscilador (seno, cuadrado, triángulo, diente de sierra)
    - Cambiar la frecuencia basada en la nota MIDI
    - Encender y apagar el oscilador
*/

class OscillatorManager {
    public:
        OscillatorManager();  // Constructor
        void setOscillatorType(int type); // Cambia el tipo de oscilador
        void playNote(uint8_t midiNote); // Cambia la frecuencia del oscilador según la nota MIDI
        void stopNote(); // Detiene la nota actual
        AudioSynthWaveform* getWaveform(); // Devuelve el objeto AudioSynthWaveform asociado al oscilador

    private:
        float midiToFreq(uint8_t midiNote); // Convierte una nota MIDI a frecuencia en Hz
        AudioSynthWaveform* waveform; // Oscilador de Teensy Audio
        int currentWaveformType; // Tipo de oscilador actual
};