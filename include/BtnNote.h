#pragma once
#include <Arduino.h>

template<typename VoiceManagerType>
class ButtonNoteController {
public:
    ButtonNoteController(VoiceManagerType* manager)
        : manager(manager)
    {
        pinMode(pinDo, INPUT_PULLUP);
        pinMode(pinMi, INPUT_PULLUP);
        pinMode(pinSol, INPUT_PULLUP);
        pinMode(pinLa, INPUT_PULLUP);
        lastStateDo = lastStateMi = lastStateSol = lastStateLa = HIGH;
    }

    void update() {
        int stateDo  = digitalRead(pinDo);
        int stateMi  = digitalRead(pinMi);
        int stateSol = digitalRead(pinSol);
        int stateLa  = digitalRead(pinLa);

        if (stateDo == LOW && lastStateDo == HIGH) manager->noteOn(60);
        if (stateDo == HIGH && lastStateDo == LOW) manager->noteOff(60);

        if (stateMi == LOW && lastStateMi == HIGH) manager->noteOn(64);
        if (stateMi == HIGH && lastStateMi == LOW) manager->noteOff(64);

        if (stateSol == LOW && lastStateSol == HIGH) manager->noteOn(67);
        if (stateSol == HIGH && lastStateSol == LOW) manager->noteOff(67);

        if (stateLa == LOW && lastStateLa == HIGH) manager->noteOn(69);
        if (stateLa == HIGH && lastStateLa == LOW) manager->noteOff(69);

        lastStateDo  = stateDo;
        lastStateMi  = stateMi;
        lastStateSol = stateSol;
        lastStateLa  = stateLa;
    }

private:
    VoiceManagerType* manager;
    const uint8_t pinDo  = 2;
    const uint8_t pinMi  = 3;
    const uint8_t pinSol = 4;
    const uint8_t pinLa  = 5;
    int lastStateDo, lastStateMi, lastStateSol, lastStateLa;
};