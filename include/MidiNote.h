#pragma once
#include <USBHost_t36.h>
#include <MIDI.h>

template <typename VoiceManagerType>
class MidiNoteController {
public:
    MidiNoteController(VoiceManagerType* vm, MIDIDevice& midi)
        : voiceManager(vm), midiInterface(midi) {}

    void update() {
        while (midiInterface.read()) {
            byte type = midiInterface.getType();
            byte note = midiInterface.getData1();
            byte velocity = midiInterface.getData2();

            if (type == midiInterface.NoteOn && velocity > 0) {
                voiceManager->noteOn(note);
            } else if (type == midiInterface.NoteOff || (type == midiInterface.NoteOn && velocity == 0)) {
                voiceManager->noteOff(note);
            }
        }
    }

private:
    VoiceManagerType* voiceManager;
    MIDIDevice& midiInterface;
};

