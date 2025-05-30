#include <Arduino.h>
#include <Audio.h>
#include <USBHost_t36.h>
#include "MidiNote.h"
#include "HighPass.h"
#include "LowPass.h"
#include "BitCrusher.h"
#include "Flanger.h"
#include "Reverb.h"
#include "Voice.h"
#include "VoiceManager.h"

#define PIN_BTN_PASO_ALTO    0
#define PIN_BTN_PASO_BAJO    1
#define PIN_BTN_BITCRUSHER   2
#define PIN_BTN_FLANGER      3
#define PIN_BTN_REVERB       4

// Variables para guardar el último estado de cada botón
int lastBtnPasoAlto    = HIGH;
int lastBtnPasoBajo    = HIGH;
int lastBtnBitcrusher  = HIGH;
int lastBtnFlanger     = HIGH;
int lastBtnReverb      = HIGH;

USBHost myusb;
MIDIDevice midi1(myusb);

// Objetos de Audio para polifonía (4 voces)
AudioSynthWaveform       waveform1, waveform2, waveform3, waveform4;
AudioEffectEnvelope      envelope1, envelope2, envelope3, envelope4;

AudioSynthWaveform* wavePtr[4] = { &waveform1, &waveform2, &waveform3, &waveform4 };
AudioEffectEnvelope* env[4] = { &envelope1, &envelope2, &envelope3, &envelope4 };

AudioOutputI2S audioOutput;
AudioControlSGTL5000 audioShield;
AudioMixer4 mixer;
AudioMixer4 mixerOut;
VoiceManager<4> vManager(mixer, wavePtr, env);
MidiNoteController<VoiceManager<4>> midiController(&vManager, midi1);

// Efectos y filtros
HighPassFilterManager highPassFilter[4];
HighPassFilterManager highPassFilterParameters;
LowPassFilterManager lowPassFilter[4];
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
AudioConnection* patchCordEnvToMixer[4];

float currentVolume = 0.6f, targetVolume = 0.6f; // Volumen inicial
long envAttack = 1000, envDecay = 1000, envSustain = 5000, envRelease = 2000; // valores *10 para más precisión

void setupVoices() {
    for (int i = 0; i < 4; ++i) {
        patchCordInit[i]     = new AudioConnection(*wavePtr[i], 0, *env[i], 0);
        patchCordEnvToHP[i]  = new AudioConnection(*env[i], 0, *highPassFilter[i].getFilter(), 0);
        patchCordEnvToLP[i]  = new AudioConnection(*env[i], 0, *lowPassFilter[i].getFilter(), 0);
        patchCordHPToMixer[i]= new AudioConnection(*highPassFilter[i].getFilter(), 2, mixer, i);
        patchCordLPToMixer[i]= new AudioConnection(*lowPassFilter[i].getFilter(), 1, mixer, i);
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
bool pasoAltoActivo = false;
bool pasoBajoActivo = false;
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
    int potValue = analogRead(1); // Lee el pin A1 (15)
    float target = map(potValue, 0, 1023, 0, 80) / 100.0f; // 0.0 - 0.8
    const float alpha = 0.15f; // Suavidad EMA (0.1 muy suave, 0.3 más rápido)
    currentVolume = alpha * target + (1.0f - alpha) * currentVolume;
    if (currentVolume < 0.01f) currentVolume = 0.01f;
    if (currentVolume > 0.8f) currentVolume = 0.8f;
    audioShield.volume(currentVolume);
}

void updateEnvelopeFromPots() {
    int potAttack  = analogRead(0); // A0 (14)
    int potDecay   = analogRead(2); // A2 (16)
    int potSustain = analogRead(3); // A3 (17) 
    int potRelease = analogRead(6); // A6 (20)

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
        env[i]->attack(envAttack / 1000.0f);   // ms -> segundos
        env[i]->decay(envDecay / 1000.0f);
        env[i]->sustain(envSustain / 10000.0f); // 0.0 - 1.0
        env[i]->release(envRelease / 1000.0f);
    }
}

// Funciones para leer el pin y alternar el estado
void checkTogglePasoAlto() {
    int btn = digitalRead(PIN_BTN_PASO_ALTO);
    if (lastBtnPasoAlto == HIGH && btn == LOW) {
        pasoAltoActivo = !pasoAltoActivo;
        actualizarConexiones();
    }
    lastBtnPasoAlto = btn;
}

void checkTogglePasoBajo() {
    int btn = digitalRead(PIN_BTN_PASO_BAJO);
    if (lastBtnPasoBajo == HIGH && btn == LOW) {
        pasoBajoActivo = !pasoBajoActivo;
        actualizarConexiones();
    }
    lastBtnPasoBajo = btn;
}

void checkToggleBitcrusher() {
    int btn = digitalRead(PIN_BTN_BITCRUSHER);
    if (lastBtnBitcrusher == HIGH && btn == LOW) {
        bitcrusherActivo = !bitcrusherActivo;
        actualizarConexiones();
    }
    lastBtnBitcrusher = btn;
}

void checkToggleFlanger() {
    int btn = digitalRead(PIN_BTN_FLANGER);
    if (lastBtnFlanger == HIGH && btn == LOW) {
        flangerActivo = !flangerActivo;
        actualizarConexiones();
    }
    lastBtnFlanger = btn;
}

void checkToggleReverb() {
    int btn = digitalRead(PIN_BTN_REVERB);
    if (lastBtnReverb == HIGH && btn == LOW) {
        reverbActivo = !reverbActivo;
        actualizarConexiones();
    }
    lastBtnReverb = btn;
}

void printAudioMemoryUsage() {
  Serial.print("AudioMemoryUsage: ");
  Serial.print(AudioMemoryUsage());
  Serial.print(" / ");
  Serial.print(AudioMemoryUsageMax());
  Serial.println(" bloques usados");
}

void printBtnStatis() {
  Serial.print("Botones: ");
  Serial.print("PA: ");
  Serial.print(pasoAltoActivo ? "ON" : "OFF");
  Serial.print(", PB: ");
  Serial.print(pasoBajoActivo ? "ON" : "OFF");
  Serial.print(", BC: ");
  Serial.print(bitcrusherActivo ? "ON" : "OFF");
  Serial.print(", FL: ");
  Serial.print(flangerActivo ? "ON" : "OFF");
  Serial.print(", RV: ");
  Serial.print(reverbActivo ? "ON" : "OFF");
  Serial.print(", Vol: ");
  Serial.print(currentVolume, 2);
  Serial.print(" (");
  Serial.print((int)(currentVolume * 100));
  Serial.print(")");
  Serial.print(", Env: A=");
  Serial.print(envAttack / 10);    // ms
  Serial.print(" D=");
  Serial.print(envDecay / 10);     // ms
  Serial.print(" S=");
  Serial.print(envSustain / 100);  // %
  Serial.print(" R=");
  Serial.println(envRelease / 10); // ms
}

void setup() {
  pinMode(PIN_BTN_PASO_ALTO, INPUT_PULLUP);
  pinMode(PIN_BTN_PASO_BAJO, INPUT_PULLUP);
  pinMode(PIN_BTN_BITCRUSHER, INPUT_PULLUP);
  pinMode(PIN_BTN_FLANGER, INPUT_PULLUP);
  pinMode(PIN_BTN_REVERB, INPUT_PULLUP);

  Serial.begin(115200);
  AudioMemory(100);
  myusb.begin();
  audioShield.enable();
  audioShield.volume(currentVolume);
  mixerOut.gain(0, 0.5f);

  setupVoices();

  // Inicializa parámetros
  highPassFilterParameters.setParams(highPassFilter[0].GetCutoff(), highPassFilter[0].GetResonance());
  lowPassFilterParameters.setParams(lowPassFilter[0].GetCutoff(), lowPassFilter[0].GetResonance());
  bitcrusherParameters.setParams(bitcrusher.GetBits(), bitcrusher.GetSampleRate());
  flangerParameters.setParams(flanger.getOffset(), flanger.getDepth(), flanger.getRate());
  reverbParameters.setParams(reverb.getRoomSize(), reverb.getDamping());

  actualizarConexiones(); // Conecta todo según estado de efectos.
}

unsigned long lastDebug = 0;
void loop() {
  midiController.update();
  //updateVolumeFromPot();
  //updateEnvelopeFromPots();
  vManager.update();
  checkTogglePasoAlto();
  checkTogglePasoBajo();
  checkToggleBitcrusher();
  checkToggleFlanger();
  checkToggleReverb();

  printBtnStatis();

  /*  if (millis() - lastDebug > 2000) {  // Cada 2 segundos
        vManager.debugStatus();
        lastDebug = millis();
    }*/

  // Filtro paso alto (bypass modificando corte)
  for (int i = 0; i < 4; ++i) {
    if (pasoAltoActivo) {
      highPassFilter[i].setParams(highPassFilterParameters.GetCutoff(), highPassFilterParameters.GetResonance());
    } else {
      highPassFilter[i].setParams(0.0, 0.5);
    }
  }

  // Filtro paso bajo
  for (int i = 0; i < 4; ++i) {
    if (pasoBajoActivo) {
      lowPassFilter[i].setParams(lowPassFilterParameters.GetCutoff(), lowPassFilterParameters.GetResonance());
    } else {
      lowPassFilter[i].setParams(22050.0, 0.7);
    }
  }

  // Parámetros de efectos
  bitcrusher.setParams(bitcrusherParameters.GetBits(), bitcrusherParameters.GetSampleRate());
  flanger.setParams(flangerParameters.getOffset(), flangerParameters.getDepth(), flangerParameters.getRate());
  reverb.setParams(reverbParameters.getRoomSize(), reverbParameters.getDamping());
}