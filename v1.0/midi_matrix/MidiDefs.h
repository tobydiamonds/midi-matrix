#ifndef MidiDefs_h
#define MidiDefs_h

#include <Arduino.h>

#define MIDI_PPQ 24

#define TRACKS 4//8
#define CLIPS_PR_TRACK 8
#define CLIP_MAX_BARS 4 // how many bars to be able to store
#define CLIP_OVERDUBBING_PR_MESSAGE 8

#define ONE_BAR 96 // PPQ * 4 => 24 * 4
#define CLIP_MAX_MESSAGES 16 // 1bar 16th notes 


//https://users.cs.cf.ac.uk/Dave.Marshall/Multimedia/node158.html
enum MidiType
{
    InvalidType = 0x00,          ///< For notifying errors
    NoteOff = 0x80,              ///< Note Off
    NoteOn = 0x90,               ///< Note On
    AfterTouchPoly = 0xA0,       ///< Polyphonic AfterTouch
    ControlChange = 0xB0,        ///< Control Change / Channel Mode
    ProgramChange = 0xC0,        ///< Program Change
    AfterTouchChannel = 0xD0,    ///< Channel (monophonic) AfterTouch
    PitchBend = 0xE0,            ///< Pitch Bend
    SystemExclusive = 0xF0,      ///< System Exclusive
    TimeCodeQuarterFrame = 0xF1, ///< System Common - MIDI Time Code Quarter Frame
    SongPosition = 0xF2,         ///< System Common - Song Position Pointer
    SongSelect = 0xF3,           ///< System Common - Song Select
    TuneRequest = 0xF6,          ///< System Common - Tune Request
    Clock = 0xF8,                ///< System Real Time - Timing Clock
    Start = 0xFA,                ///< System Real Time - Start
    Continue = 0xFB,             ///< System Real Time - Continue
    Stop = 0xFC,                 ///< System Real Time - Stop
    ActiveSensing = 0xFE,        ///< System Real Time - Active Sensing
    SystemReset = 0xFF,          ///< System Real Time - System Reset
};

int ExpectedMessageLength(unsigned char type)
{
    switch (type)
    {
    // 1 byte messages
    case Start:
    case Continue:
    case Stop:
    case Clock:
    case ActiveSensing:
    case SystemReset:
    case TuneRequest:
        return 1;
        break;

        // 2 bytes messages
    case ProgramChange:
    case AfterTouchChannel:
    case TimeCodeQuarterFrame:
    case SongSelect:
        return 2;
        break;

        // 3 bytes messages
    case NoteOn:
    case NoteOff:
    case ControlChange:
    case PitchBend:
    case AfterTouchPoly:
    case SongPosition:
        return 3;
        break;

    case SystemExclusive:
        return -1;
        break;

    case InvalidType:
    default:
        return 0;
        break;
    }
}

unsigned char GetStatus(unsigned char type, unsigned char channel)
{
    //return ((byte)type | ((channel - 1) & 0x0F));
    return type | channel;
}

unsigned char GetChannel(unsigned char status)
{
    // status byte: CMD 4 bits, CH 4 bits
    return status & 0x0F;
}

#endif