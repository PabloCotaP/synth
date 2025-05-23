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
AudioSynthWaveform waveform[4];
AudioEffectEnvelope envelope[4]; // Añadido: array de envelopes
AudioEffectEnvelope* env[4];     // Añadido: punteros a envelopes

AudioOutputI2S audioOutput;
AudioControlSGTL5000 audioShield;
AudioMixer4 mixer;
AudioMixer4 mixerOut;
VoiceManager<4> poly(mixer);
ButtonNoteController<VoiceManager<4>> btnController(&poly);

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
AudioConnection* patchCordInit[4];
AudioConnection* patchCordEnvToHP[4];
AudioConnection* patchCordEnvToLP[4];
AudioConnection* patchCordHPToMixer[4];
AudioConnection* patchCordLPToMixer[4];

float currentVolume = 0.5f, targetVolume = 0.5f; // Volumen inicial
long envAttack = 1000, envDecay = 1000, envSustain = 5000, envRelease = 2000; // valores *10 para más precisión

void setupVoices() {
    for (int i = 0; i < 4; ++i) {
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

void updateVolumeFromPot() {
    int potValue = analogRead(0); // Lee el pin A0
    float target = map(potValue, 0, 1023, 0, 80) / 100.0f; // 0.0 - 0.8
    const float alpha = 0.15f; // Suavidad EMA (0.1 muy suave, 0.3 más rápido)
    currentVolume = alpha * target + (1.0f - alpha) * currentVolume;
    if (currentVolume < 0.01f) currentVolume = 0.01f;
    if (currentVolume > 0.8f) currentVolume = 0.8f;
    audioShield.volume(currentVolume);

    /*
    Serial.print("Volumen: ");
    Serial.print(currentVolume, 2);
    Serial.print(" (");
    Serial.print((int)(currentVolume * 100));
    Serial.println(")");
    */
}

void updateEnvelopeFromPots() {
    int potAttack  = analogRead(1); // A1
    int potDecay   = analogRead(2); // A2
    int potSustain = analogRead(5); // A5
    int potRelease = analogRead(4); // A4

    long targetAttack  = map(potAttack,  0, 1023, 10, 30000);   // 1ms - 3s (x10)
    long targetDecay   = map(potDecay,   0, 1023, 10, 30000);   // 1ms - 3s (x10)
    long targetSustain = map(potSustain, 0, 1023, 0, 10000);    // 0 - 100% (x100)
    long targetRelease = map(potRelease, 0, 1023, 10, 60000);   // 1ms - 6s (x10)

    const float alpha = 0.15f; // Suavidad EMA

    envAttack  = alpha * targetAttack  + (1.0f - alpha) * envAttack;
    envDecay   = alpha * targetDecay   + (1.0f - alpha) * envDecay;
    envSustain = alpha * targetSustain + (1.0f - alpha) * envSustain;
    envRelease = alpha * targetRelease + (1.0f - alpha) * envRelease;

    for (int i = 0; i < 4; ++i) {
        envelope[i].attack(envAttack / 1000.0f);   // ms -> segundos
        envelope[i].decay(envDecay / 1000.0f);
        envelope[i].sustain(envSustain / 10000.0f); // 0.0 - 1.0
        envelope[i].release(envRelease / 1000.0f);
    }

    /*
    Serial.print("Env: A=");
    Serial.print(envAttack / 10);    // ms
    Serial.print(" D=");
    Serial.print(envDecay / 10);     // ms
    Serial.print(" S=");
    Serial.print(envSustain / 100);  // %
    Serial.print(" R=");
    Serial.println(envRelease / 10); // ms
    */
}

void printAudioMemoryUsage() {
  Serial.print("AudioMemoryUsage: ");
  Serial.print(AudioMemoryUsage());
  Serial.print(" / ");
  Serial.print(AudioMemoryUsageMax());
  Serial.println(" bloques usados");
}

void printButtonStates() {
    int stateDo  = digitalRead(2);
    int stateMi  = digitalRead(3);
    int stateSol = digitalRead(4);
    int stateLa  = digitalRead(5);

    Serial.print("Botones: DO=");
    Serial.print(stateDo == LOW ? "ON" : "OFF");
    Serial.print(" MI=");
    Serial.print(stateMi == LOW ? "ON" : "OFF");
    Serial.print(" SOL=");
    Serial.print(stateSol == LOW ? "ON" : "OFF");
    Serial.print(" LA=");
    Serial.println(stateLa == LOW ? "ON" : "OFF");
}

void setup() {
  Serial.begin(115200);
  AudioMemory(100);
  audioShield.enable();
  audioShield.volume(currentVolume);

  setupVoices();

  for (int i = 0; i < 4; ++i) {
    mixer.gain(i, 0.5f);
    mixerOut.gain(i, 0.5f);
  }

  // Inicializa parámetros
  highPassFilterParameters.setParams(highPassFilter.GetCutoff(), highPassFilter.GetResonance());
  lowPassFilterParameters.setParams(lowPassFilter.GetCutoff(), lowPassFilter.GetResonance());
  bitcrusherParameters.setParams(bitcrusher.GetBits(), bitcrusher.GetSampleRate());
  flangerParameters.setParams(flanger.getOffset(), flanger.getDepth(), flanger.getRate());
  reverbParameters.setParams(reverb.getRoomSize(), reverb.getDamping());

  actualizarConexiones(); // Conecta todo según estado de efectos.
}

void loop() {
  btnController.update();
  updateEnvelopeFromPots();
  updateVolumeFromPot();

  printButtonStates();

  // Filtro paso alto (bypass modificando corte)
  if (pasoAltoActivo) {
    highPassFilter.setParams(highPassFilterParameters.GetCutoff(), highPassFilterParameters.GetResonance());
  } else {
    highPassFilter.setParams(0.0, 0.5);
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

  // Imprime uso de memoria de audio cada segundo
  /*
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 1000) {
    printAudioMemoryUsage();
    lastPrint = millis();
  }
  */
}