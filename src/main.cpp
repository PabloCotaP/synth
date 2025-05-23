#include <Arduino.h>
#include <Audio.h>
#include "BtnNote.h"
#include "HighPass.h"
#include "LowPass.h"
#include "BitCrusher.h"
#include "Flanger.h"
#include "Reverb.h"
#include "Oscillator.h"
#include "Polyphony.h"
#include "Voice.h"
#include "VoiceManager.h"

// Objetos de Audio para polifonía (3 voces)
AudioSynthWaveform waveform[3];
AudioEffectEnvelope envelope[3]; // Añadido: array de envelopes
AudioEffectEnvelope* env[3];     // Añadido: punteros a envelopes

AudioOutputI2S audioOutput;
AudioControlSGTL5000 audioShield;
AudioMixer4 mixer;
AudioMixer4 mixerOut;
VoiceManager<3> poly(mixer);
ButtonNoteController<VoiceManager<3>> btnController(&poly);

// Efectos y filtros
HighPassFilterManager highPassFilter;
HighPassFilterManager highPassFilterParameters;
LowPassFilterManager lowPassFilter;
LowPassFilterManager lowPassFilterParameters;
BitCrusherManager bitcrusher;
BitCrusherManager bitcrusherParameters;
FlangerManager flanger;
FlangerManager flangerParameters;
ReverbManager reverb;
ReverbManager reverbParameters;

// Conexiones fijas: cada voz -> filtros -> mixer
AudioConnection* patchCordInit[3];
AudioConnection* patchCordEnvToHP[3];
AudioConnection* patchCordEnvToLP[3];
AudioConnection* patchCordHPToMixer[3];
AudioConnection* patchCordLPToMixer[3];

void setupVoices() {
    for (int i = 0; i < 3; ++i) {
        env[i] = &envelope[i];
        patchCordInit[i]     = new AudioConnection(waveform[i], 0, *env[i], 0);
        patchCordEnvToHP[i]  = new AudioConnection(*env[i], 0, *highPassFilter.getFilter(), 0);
        patchCordEnvToLP[i]  = new AudioConnection(*env[i], 0, *lowPassFilter.getFilter(), 0);
        patchCordHPToMixer[i]= new AudioConnection(*highPassFilter.getFilter(), 2, mixer, i);
        patchCordLPToMixer[i]= new AudioConnection(*lowPassFilter.getFilter(), 1, mixer, i);
    }
}

// Conexiones dinámicas
AudioConnection* patchMixerToCrusher = nullptr;
AudioConnection* patchCrusherToFlanger = nullptr; 
AudioConnection* patchFlangerToReverb = nullptr;
AudioConnection* patchMixerToFlanger = nullptr;
AudioConnection* patchCrusherToReverb = nullptr;
AudioConnection* patchMixerToReverb = nullptr;
AudioConnection* patchReverbToMixerOut = nullptr;
AudioConnection* patchFlangerToMixerOut = nullptr;
AudioConnection* patchMixerDirectToOut = nullptr;

// Conexión mixerOut a salida estéreo
AudioConnection patchMixerOutToOutL(mixerOut, 0, audioOutput, 0);
AudioConnection patchMixerOutToOutR(mixerOut, 0, audioOutput, 1);

// Estado de efectos
bool pasoAltoActivo = true;
bool pasoBajoActivo = true;
bool bitcrusherActivo = false;
bool flangerActivo = false; 
bool reverbActivo = false;

void actualizarConexiones() {
  // Limpieza previa
  if (patchMixerToCrusher)      { delete patchMixerToCrusher;      patchMixerToCrusher = nullptr; }
  if (patchCrusherToFlanger)    { delete patchCrusherToFlanger;    patchCrusherToFlanger = nullptr; }
  if (patchFlangerToReverb)     { delete patchFlangerToReverb;     patchFlangerToReverb = nullptr; }
  if (patchMixerToFlanger)      { delete patchMixerToFlanger;      patchMixerToFlanger = nullptr; }
  if (patchCrusherToReverb)     { delete patchCrusherToReverb;     patchCrusherToReverb = nullptr; }
  if (patchMixerToReverb)       { delete patchMixerToReverb;       patchMixerToReverb = nullptr; }
  if (patchReverbToMixerOut)    { delete patchReverbToMixerOut;    patchReverbToMixerOut = nullptr; }
  if (patchFlangerToMixerOut)   { delete patchFlangerToMixerOut;   patchFlangerToMixerOut = nullptr; }
  if (patchMixerDirectToOut)    { delete patchMixerDirectToOut;    patchMixerDirectToOut = nullptr; }

  // Combinaciones posibles
  if (bitcrusherActivo) {
    patchMixerToCrusher = new AudioConnection(mixer, 0, *bitcrusher.getBitcrusher(), 0);

    if (flangerActivo) {
      patchCrusherToFlanger = new AudioConnection(*bitcrusher.getBitcrusher(), 0, *flanger.getFlanger(), 0);

      if (reverbActivo) {
        patchFlangerToReverb = new AudioConnection(*flanger.getFlanger(), 0, *reverb.getReverb(), 0);
        patchReverbToMixerOut = new AudioConnection(*reverb.getReverb(), 0, mixerOut, 0);
      } else {
        patchFlangerToMixerOut = new AudioConnection(*flanger.getFlanger(), 0, mixerOut, 0);
      }

    } else if (reverbActivo) {
      patchCrusherToReverb = new AudioConnection(*bitcrusher.getBitcrusher(), 0, *reverb.getReverb(), 0);
      patchReverbToMixerOut = new AudioConnection(*reverb.getReverb(), 0, mixerOut, 0);
    } else {
      patchMixerDirectToOut = new AudioConnection(*bitcrusher.getBitcrusher(), 0, mixerOut, 0);
    }

  } else if (flangerActivo) {
    patchMixerToFlanger = new AudioConnection(mixer, 0, *flanger.getFlanger(), 0);

    if (reverbActivo) {
      patchFlangerToReverb = new AudioConnection(*flanger.getFlanger(), 0, *reverb.getReverb(), 0);
      patchReverbToMixerOut = new AudioConnection(*reverb.getReverb(), 0, mixerOut, 0);
    } else {
      patchFlangerToMixerOut = new AudioConnection(*flanger.getFlanger(), 0, mixerOut, 0);
    }

  } else if (reverbActivo) {
    patchMixerToReverb = new AudioConnection(mixer, 0, *reverb.getReverb(), 0);
    patchReverbToMixerOut = new AudioConnection(*reverb.getReverb(), 0, mixerOut, 0);

  } else {
    patchMixerDirectToOut = new AudioConnection(mixer, 0, mixerOut, 0);
  }
}

void setup() {
  Serial.begin(9600);
  AudioMemory(180);
  audioShield.enable();
  audioShield.volume(0.5);

  setupVoices();

  // Inicializa parámetros
  highPassFilterParameters.setParams(highPassFilter.GetCutoff(), highPassFilter.GetResonance());
  lowPassFilterParameters.setParams(lowPassFilter.GetCutoff(), lowPassFilter.GetResonance());
  bitcrusherParameters.setParams(bitcrusher.GetBits(), bitcrusher.GetSampleRate());
  flangerParameters.setParams(flanger.getOffset(), flanger.getDepth(), flanger.getRate());
  reverbParameters.setParams(reverb.getRoomSize(), reverb.getDamping());

  actualizarConexiones(); // Conecta todo según estado de efectos
}

void loop() {
  btnController.update();

  // Filtro paso alto (bypass modificando corte)
  if (pasoAltoActivo) {
    highPassFilter.setParams(highPassFilterParameters.GetCutoff(), highPassFilterParameters.GetResonance());
  } else {
    highPassFilter.setParams(0.0, 0.7);
  }

  // Filtro paso bajo
  if (pasoBajoActivo) {
    lowPassFilter.setParams(lowPassFilterParameters.GetCutoff(), lowPassFilterParameters.GetResonance());
  } else {
    lowPassFilter.setParams(22050.0, 0.7);
  }

  // Parámetros de efectos
  bitcrusher.setParams(bitcrusherParameters.GetBits(), bitcrusherParameters.GetSampleRate());
  flanger.setParams(flangerParameters.getOffset(), flangerParameters.getDepth(), flangerParameters.getRate());
  reverb.setParams(reverbParameters.getRoomSize(), reverbParameters.getDamping());
}