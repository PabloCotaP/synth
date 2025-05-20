// #include <USBHost_t36.h>
// #include <Audio.h>

// // Prototipos de funciones
// void printDeviceInfo();
// void OnNoteOn(byte channel, byte note, byte velocity);
// void OnNoteOff(byte channel, byte note, byte velocity);

// // Objetos para el USB Host
// USBHost myusb;
// USBHub hub1(myusb);
// MIDIDevice_BigBuffer midi1(myusb);

// // Variables para monitorear actividad MIDI
// bool midiActivity = false;
// unsigned long lastMidiActivity = 0;

// void printDeviceInfo(); // Prototipo de función para evitar error de declaración
// void OnNoteOn(byte channel, byte note, byte velocity);
// void OnNoteOff(byte channel, byte note, byte velocity);

// void setup() {
//   // Iniciar comunicación serial con la computadora
//   Serial.begin(115200);
  
//   // Esperar a que el puerto serial esté disponible o hasta 1.5 segundos
//   for (int i = 0; i < 15 && !Serial; i++) {
//     delay(100);
//   }
  
//   Serial.println("Iniciando Monitor MIDI para Alesis Recital...");
//   Serial.println("Monitorizando solo eventos de Nota ON/OFF");
  
//   // Iniciar USB Host
//   myusb.begin();
  
//   // Configurar callbacks solo para los eventos de Nota ON/OFF
//   midi1.setHandleNoteOn(OnNoteOn);
//   midi1.setHandleNoteOff(OnNoteOff);
// }
// // 

// void loop() {
//   // Actualizar USB Host (necesario para recibir datos)
//   myusb.Task();
  
//   // Leer los datos MIDI disponibles
//   midi1.read();
  
//   // Mostrar indicador de actividad MIDI
//   if (midiActivity && millis() - lastMidiActivity > 250) {
//     midiActivity = false;
//     Serial.println("------------");
//   }
  
//   // Verificar si hay un dispositivo conectado
//   static bool deviceChecked = false;
//   if (!deviceChecked && midi1.idVendor() != 0) {
//     printDeviceInfo();
//     deviceChecked = true;
//   }
// }

// // Función para imprimir información sobre el dispositivo conectado
// void printDeviceInfo() {
//   Serial.println("------------------------------");
//   Serial.println("Dispositivo MIDI conectado:");
  
//   if (*midi1.product()) {
//     Serial.printf("Fabricante: %s\n", midi1.manufacturer());
//     Serial.printf("Producto: %s\n", midi1.product());
//     Serial.printf("VID: %04X, PID: %04X\n", midi1.idVendor(), midi1.idProduct());
//   } else {
//     Serial.println("No se detectó nombre de dispositivo");
//   }
  
//   Serial.println("------------------------------");
// }

// // Funciones de callback para eventos MIDI de Nota ON/OFF
// void OnNoteOn(byte channel, byte note, byte velocity) {
//   midiActivity = true;
//   lastMidiActivity = millis();
//   Serial.printf("Nota ON - Canal: %d, Nota: %d, Velocidad: %d\n", channel, note, velocity);
// }

// void OnNoteOff(byte channel, byte note, byte velocity) {
//   midiActivity = true;
//   lastMidiActivity = millis();
//   Serial.printf("Nota OFF - Canal: %d, Nota: %d, Velocidad: %d\n", channel, note, velocity);
// }

// msin adaptado/

/*
#include <Audio.h>
#include <USBHost_t36.h>
#include "modulos/midi_handler.h"
#include "modulos/Oscillator.h"
#include "modulos/audio_manager.h"

// Instancias globales
Oscillator osc;
AudioManager audio;
MidiHandler midi;

// Callbacks MIDI
void handleNoteOn(byte channel, byte note, byte velocity) {
    float freq = 440.0 * pow(2, (note - 69) / 12.0);
    osc.noteOn(freq, velocity / 127.0f);
    Serial.printf("NoteOn - Ch: %d, Note: %d, Vel: %d\n", channel, note, velocity);
}

void handleNoteOff(byte channel, byte note, byte velocity) {
    osc.noteOff();
    Serial.printf("NoteOff - Ch: %d, Note: %d\n", channel, note);
}

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 1500); // Espera 1.5s máximo

    audio.begin();      // Inicia Audio Shield
    osc.begin();        // Configura oscilador
    midi.begin();       // Inicia USB Host
    midi.setCallbacks(handleNoteOn, handleNoteOff);

    Serial.println("Sintetizador MIDI listo");
}

void loop() {
    midi.update();      // Escanea eventos MIDI
}
*/
#include <USBHost_t36.h>
#include <Audio.h>

// Configuración de audio
AudioSynthWaveform       waveform;
AudioOutputI2S           audioOutput;
AudioConnection          patchCord1(waveform, 0, audioOutput, 0);  // Canal izquierdo
AudioConnection          patchCord2(waveform, 0, audioOutput, 1);  // Canal derecho
AudioControlSGTL5000     audioShield;  // Para el Audio Shield

// Configuración MIDI USB
USBHost myusb;
USBHub hub1(myusb);
MIDIDevice midi1(myusb);

void OnNoteOn(byte channel, byte note, byte velocity) {
    // Convertir nota MIDI a frecuencia (A4 = 440Hz)
    float freq = 440.0 * pow(2.0, (note - 69) / 12.0);
    
    // Configurar el oscilador
    waveform.begin(0.8, freq, WAVEFORM_SINE);  // Amplitud, Frecuencia, Forma de onda
    waveform.amplitude(velocity / 127.0);      // Ajustar volumen según velocity
    
    Serial.printf("Nota ON - Canal: %d, Nota: %d, Frec: %.2f Hz\n", channel, note, freq);
}

void OnNoteOff(byte channel, byte note, byte velocity) {
    waveform.amplitude(0);  // Silenciar al soltar la nota
    Serial.printf("Nota OFF - Canal: %d, Nota: %d\n", channel, note);
}

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 1500); // Esperar hasta 1.5 segundos por el puerto serial
    
    // Iniciar audio
    AudioMemory(10);
    audioShield.enable();
    audioShield.volume(0.7);  // Volumen general
    
    // Iniciar MIDI USB
    myusb.begin();
    midi1.setHandleNoteOn(OnNoteOn);
    midi1.setHandleNoteOff(OnNoteOff);
    
    Serial.println("Sintetizador MIDI listo");
}

void loop() {
    myusb.Task();  // Mantener USB activo
    midi1.read();   // Leer mensajes MIDI
}