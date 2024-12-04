#ifndef MidiMessage_h
#define MidiMessage_h

#include <Arduino.h>
#include "MidiDefs.h"

struct MidiMessage
{
    unsigned char Bar;
    unsigned char Position;
    unsigned char Type;
    unsigned char Data1;
    unsigned char Data2;

    MidiMessage()
    {
        Bar = 0;
        Position = 0;
        Type = 0x00;
        Data1 = 0x00;
        Data2 = 0x00;
    }

    MidiMessage(unsigned char bar, unsigned char position, unsigned char type, unsigned char data1, unsigned char data2)
    {
        Bar = bar;
        Position = position;
        Type = type;
        Data1 = data1;
        Data2 = data2;
    }
};

#endif