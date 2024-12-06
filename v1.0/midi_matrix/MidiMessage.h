#ifndef MidiMessage_h
#define MidiMessage_h

#include <Arduino.h>
#include "MidiDefs.h"

struct MidiMessage
{
    uint8_t Type;
    uint8_t Data1;
    uint8_t Data2;

    MidiMessage()
    {
        Type = 0x00;
        Data1 = 0x00;
        Data2 = 0x00;
    }

    MidiMessage(uint8_t type, uint8_t data1, uint8_t data2)
    {
        Type = type;
        Data1 = data1;
        Data2 = data2;
    }
};

#endif