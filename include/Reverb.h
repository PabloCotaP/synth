#pragma once
#include <Arduino.h>
#include <Audio.h>

class ReverbManager {
public:
    ReverbManager(float roomSize = 0.5f, float damping = 0.5f) : freeverb() {
        setRoomSize(roomSize);
        setDamping(damping);
    }

    AudioEffectFreeverb* getReverb() { return &freeverb; }

    void setRoomSize(float size) {
        if (size < 0.0f) size = 0.0f;
        if (size > 1.0f) size = 1.0f;
        roomSize = size;
        freeverb.roomsize(roomSize);
    }

    void setDamping(float amount) {
        if (amount < 0.0f) amount = 0.0f;
        if (amount > 1.0f) amount = 1.0f;
        damping = amount;
        freeverb.damping(damping);
    }

    void setParams(float roomSize, float damping) {
        setRoomSize(roomSize);
        setDamping(damping);
    }

    float getRoomSize() { return roomSize; }
    float getDamping() { return damping; }

private:
    AudioEffectFreeverb freeverb;
    float roomSize;
    float damping;
};
