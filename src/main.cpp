#include <Arduino.h>
#include <Audio.h>
#include "OscillatorManager.h"

// === Instancias necesarias ===
AudioSynthWaveform waveform;            // Oscilador
AudioOutputI2S audioOutput;             // Salida al Audio Shield (I2S)
AudioConnection patchCord(waveform, 0, audioOutput, 0); // Canal izquierdo
AudioConnection patchCord2(waveform, 0, audioOutput, 1); // Canal derecho
AudioControlSGTL5000 audioShield;       // Controlador del codec de audio

// Instancia del oscilador
OscillatorManager osc; // Requiere pasarle el waveform

void setup() {
  Serial.begin(9600);

  // Inicializa el sistema de audio
  AudioMemory(12);  // Reserva bloques de memoria de audio
  audioShield.enable();            // Enciende el codec
  audioShield.volume(0.5);         // Volumen (0.0 a 1.0)

  // Prueba: reproduce una nota al inicio
  osc.playNote(60); // C4
}

void loop() {
  // Aquí irán tus controles
}