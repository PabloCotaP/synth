#pragma once
#include <USBHost_t36.h>

class MidiHandler {
public:
    void begin() {
        myusb.begin();
    }

    void update() {
        myusb.Task();
        midi.read();
    }

    void setCallbacks(void (*noteOn)(byte, byte, byte), void (*noteOff)(byte, byte, byte)) {
        midi.setHandleNoteOn(noteOn);
        midi.setHandleNoteOff(noteOff);
    }

private:
    USBHost myusb;
    USBHub hub1{myusb};
    MIDIDevice_BigBuffer midi{myusb};
};