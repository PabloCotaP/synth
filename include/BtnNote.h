#pragma once
#include <Arduino.h>
#include "Polyphony.h"

template<typename PolyphonyType>
class ButtonNoteController {
public:
    ButtonNoteController(PolyphonyType* poly)
        : poly(poly)
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

        if (stateDo == LOW && lastStateDo == HIGH) poly->noteOn(60);
        if (stateDo == HIGH && lastStateDo == LOW) poly->noteOff(60);

        if (stateMi == LOW && lastStateMi == HIGH) poly->noteOn(64);
        if (stateMi == HIGH && lastStateMi == LOW) poly->noteOff(64);

        if (stateSol == LOW && lastStateSol == HIGH) poly->noteOn(67);
        if (stateSol == HIGH && lastStateSol == LOW) poly->noteOff(67);

        if (stateLa == LOW && lastStateLa == HIGH) poly->noteOn(69);
        if (stateLa == HIGH && lastStateLa == LOW) poly->noteOff(69);

        lastStateDo  = stateDo;
        lastStateMi  = stateMi;
        lastStateSol = stateSol;
        lastStateLa  = stateLa;

    }

private:
    PolyphonyType* poly;
    const uint8_t pinDo  = 2;
    const uint8_t pinMi  = 3;
    const uint8_t pinSol = 4;
    const uint8_t pinLa = 5;
    int lastStateDo, lastStateMi, lastStateSol, lastStateLa;
};